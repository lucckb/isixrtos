/*
 * sdio_sdcard_driver.cpp
 *
 *  Created on: 22-11-2012
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <stm32sdio.h>
#include <stm32system.h>
#include <stddef.h>
#include <stm32dma.h>
#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif
#include "sdcard_defs.h"
#include "sdio_sdcard_driver.h"
#include <string.h>
#include <isix.h>
#include <dbglog.h>
/* ------------------------------------------------------------------ */
/* Configure driver mode */
#define SD_DMA_MODE


//Config section
#ifndef SDIO_INIT_CLK_DIV
#define SDIO_INIT_CLK_DIV                ((uint8_t)0x76)
#endif

#ifndef SDIO_TRANSFER_CLK_DIV
#define SDIO_TRANSFER_CLK_DIV            ((uint8_t)0x2)
#endif

#ifndef SD_DETECT_GPIO_PORT
#define SD_DETECT_GPIO_PORT GPIOC
#endif

#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN 13
#endif

/* ------------------------------------------------------------------ */
#define SDIO_STATIC_FLAGS               ((uint32_t)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((uint32_t)0x00010000)

/**
  * @brief  Mask for errors Card Status R1 (OCR Register)
  */
#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

/**
  * @brief  Masks for R6 Response
  */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define SD_STD_CAPACITY                 ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)

#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)

#define SD_DATATIMEOUT                  ((uint32_t)0xFFFFFFFF)
#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)
/**
  * @brief  Command Class Supported
  */
#define SD_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)
/**
  * @brief  Following commands are SD Card Specific commands.
  *         SDIO_APP_CMD should be sent before sending these commands.
  */
#define SDIO_SEND_IF_COND               ((uint32_t)0x00000008)


/* ------------------------------------------------------------------ */
static uint32_t CardType =  SDIO_STD_CAPACITY_SD_CARD_V1_1;
static uint32_t CSD_Tab[4], CID_Tab[4], RCA = 0;
static uint8_t SDSTATUS_Tab[16];
static volatile  uint32_t StopCondition = 0;
static volatile sdcard_err TransferError = SD_OK;
static volatile  uint32_t TransferEnd = 0, DMAEndOfTransfer = 0;

/* ------------------------------------------------------------------ */
static sdcard_err CmdError(void);
static sdcard_err CmdResp1Error(uint8_t cmd);
static sdcard_err CmdResp7Error(void);
static sdcard_err CmdResp3Error(void);
static sdcard_err CmdResp2Error(void);
static sdcard_err CmdResp6Error(uint8_t cmd, uint16_t *prca);
static sdcard_err SDEnWideBus(FunctionalState NewState);
static sdcard_err IsCardProgramming(uint8_t *pstatus);
static sdcard_err FindSCR(uint16_t rca, uint32_t *pscr);
static uint8_t convert_from_bytes_to_power_of_two(uint16_t NumberOfBytes);
static SDCardState SD_GetState(void);
static uint8_t SD_Detect(void);
static sdcard_err SD_PowerON(void);
static sdcard_err SD_GetCardInfo(SD_CardInfo *cardinfo);
static sdcard_err SD_InitializeCards(void);
static sdcard_err SD_EnableWideBusOperation(uint32_t WideMode);
static sdcard_err SD_SelectDeselect(uint32_t addr);
static sdcard_err SD_StopTransfer(void);
static sdcard_err SD_SendStatus(uint32_t *pcardstatus);
static sdcard_err SD_SendSDStatus(uint32_t *psdstatus);
static void SD_ProcessDMAIRQ(void);
static sdcard_err SD_ProcessIRQSrc(void);
/* ------------------------------------------------------------------ */
/**
  * @brief  DeInitializes the SDIO interface.
  * @param  None
  * @retval None
  */
static void SD_DeInit(void)
{
	/*!< Disable SDIO Clock */
	sdio_clock_cmd(DISABLE);
	/*!< Set Power State to OFF */
	sdio_set_power_state(SDIO_PowerState_OFF);
	/*!< DeInitializes the SDIO peripheral */
	sdio_deinit();
	/* Disable the SDIO APB2 Clock */
	rcc_apb2_periph_clock_cmd(RCC_APB2Periph_SDIO, DISABLE);
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	/* Deactivate the GPIO ports */
	gpio_pin_AF_config( GPIOC, GPIO_PinSource8,  GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource9,  GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource10, GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource11, GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOC, GPIO_PinSource12, GPIO_AF_MCO );
	gpio_pin_AF_config( GPIOD, GPIO_PinSource2,  GPIO_AF_MCO );
	gpio_config_ext( GPIOC, gpioPIN(8)|gpioPIN(9)|gpioPIN(10)|gpioPIN(11)|gpioPIN(12),
			GPIO_MODE_INPUT, GPIO_PUPD_NONE, 0, 0 );
	gpio_config( GPIOC, 2, GPIO_MODE_INPUT, GPIO_PUPD_NONE, 0, 0 );
#else
#error F1 not implemented yet
#endif
}

/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for data
  *         transfer).
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_Init(SD_CardInfo *cardinfo)
{
  sdcard_err errorstatus = SD_OK;
  /* SDIO Peripheral Low Level Init */
  rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
  gpio_clock_enable( SD_DETECT_GPIO_PORT, true );
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
  gpio_pin_AF_config(GPIOC, GPIO_PinSource8, GPIO_AF_SDIO);
  gpio_pin_AF_config(GPIOC, GPIO_PinSource9, GPIO_AF_SDIO);
  gpio_pin_AF_config(GPIOC, GPIO_PinSource10, GPIO_AF_SDIO);
  gpio_pin_AF_config(GPIOC, GPIO_PinSource11, GPIO_AF_SDIO);
  gpio_pin_AF_config(GPIOC, GPIO_PinSource12, GPIO_AF_SDIO);
  gpio_pin_AF_config(GPIOD, GPIO_PinSource2, GPIO_AF_SDIO);
  gpio_config_ext( GPIOC, gpioPIN(8)|gpioPIN(9)|gpioPIN(10)|gpioPIN(11),
		  GPIO_MODE_ALTERNATE, GPIO_PUPD_PULLUP, GPIO_SPEED_25MHZ, GPIO_OTYPE_PP );
  gpio_config( GPIOD, 2, GPIO_MODE_ALTERNATE, GPIO_PUPD_PULLUP, GPIO_SPEED_25MHZ, GPIO_OTYPE_PP );
  gpio_config( GPIOC, 12, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, GPIO_SPEED_25MHZ, GPIO_OTYPE_PP );
  gpio_config( SD_DETECT_GPIO_PORT, SD_DETECT_PIN, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 0, 0 );
#ifdef  SD_DMA_MODE
  rcc_ahb1_periph_clock_cmd( RCC_AHB1Periph_DMA2, true );
#endif
#else
#error F1 not implemented yet
#endif
  rcc_apb2_periph_clock_cmd( RCC_APB2Periph_SDIO, true );
  /* SDIO Peripheral Low Level Init END*/
  sdio_deinit();
  errorstatus = SD_PowerON();
  if (errorstatus != SD_OK)
  {
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return(errorstatus);
  }
  errorstatus = SD_InitializeCards();
  if (errorstatus != SD_OK)
  {
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return(errorstatus);
  }
  /*!< Configure the SDIO peripheral */
  /*!< SDIO_CK = SDIOCLK / (SDIO_TRANSFER_CLK_DIV + 2) */
  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
  sdio_init(SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
		  SDIO_BusWide_1b, SDIO_HardwareFlowControl_Disable, SDIO_TRANSFER_CLK_DIV );
  errorstatus = SD_GetCardInfo(cardinfo);
  if (errorstatus == SD_OK)
  {
    errorstatus = SD_SelectDeselect((uint32_t) (cardinfo->RCA << 16));
  }
  if (errorstatus == SD_OK)
  {
    errorstatus = SD_EnableWideBusOperation(SDIO_BusWide_4b);
  }
  return(errorstatus);
}

#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2) && defined(SD_DMA_MODE)

#define SD_SDIO_DMA_FLAG_TCIF         DMA_FLAG_TCIF3
#define SD_SDIO_DMA_FLAG_FEIF         DMA_FLAG_FEIF3
#define SD_SDIO_DMA_STREAM            DMA2_Stream3
#define SD_SDIO_DMA_FLAG_HTIF         DMA_FLAG_HTIF3
#define SD_SDIO_DMA_FLAG_DMEIF        DMA_FLAG_DMEIF3
#define SD_SDIO_DMA_FLAG_TEIF         DMA_FLAG_TEIF3
#define SD_SDIO_DMA_CHANNEL           DMA_Channel_4
#define SDIO_FIFO_ADDRESS             ((void*)0x40012C80)
/**
  * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
  * @param  BufferSRC: pointer to the source buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
static void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize)
{
  dma_clear_flag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);
  /* DMA2 Stream3  or Stream6 disable */
  dma_cmd(SD_SDIO_DMA_STREAM, DISABLE);
  /* DMA2 Stream3  or Stream6 Config */
  dma_deinit( SD_SDIO_DMA_STREAM );
  dma_init( SD_SDIO_DMA_STREAM , DMA_DIR_MemoryToPeripheral | DMA_PeripheralInc_Disable |
		    SD_SDIO_DMA_CHANNEL |DMA_MemoryInc_Enable | DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
		  	DMA_Mode_Normal | DMA_Priority_VeryHigh | DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4,
		  	DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full, 0, SDIO_FIFO_ADDRESS, BufferSRC );
  dma_it_config(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
  dma_flow_controller_config(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
  /* DMA2 Stream3  or Stream6 enable */
  dma_cmd(SD_SDIO_DMA_STREAM, ENABLE);
}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
  * @param  BufferDST: pointer to the destination buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
static void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize)
{
  dma_clear_flag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);
  /* DMA2 Stream3  or Stream6 disable */
  dma_cmd(SD_SDIO_DMA_STREAM, DISABLE);
  /* DMA2 Stream3 or Stream6 Config */
  dma_deinit(SD_SDIO_DMA_STREAM);
  dma_init(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_CHANNEL | DMA_DIR_PeripheralToMemory | DMA_PeripheralInc_Disable |
		  DMA_MemoryInc_Enable | DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
		  DMA_Mode_Normal | DMA_Priority_VeryHigh | DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4 ,
		  DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full, 0, SDIO_FIFO_ADDRESS,  BufferDST );

  dma_it_config(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
  dma_flow_controller_config(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
  /* DMA2 Stream3 or Stream6 enable */
  dma_cmd(SD_SDIO_DMA_STREAM, ENABLE);
}


void __attribute__((__interrupt__)) dma2_stream3_isr_vector( void )
{
	SD_ProcessDMAIRQ();
}

void __attribute__((__interrupt__)) sdio_isr_vector( void )
{
	SD_ProcessIRQSrc();
}

#else
#error F1 not implemented yet
#endif


/**
  * @brief  Gets the cuurent sd card data transfer status.
  * @param  None
  * @retval SDTransferState: Data Transfer state.
  *   This value can be:
  *        - SD_TRANSFER_OK: No data transfer is acting
  *        - SD_TRANSFER_BUSY: Data transfer is acting
  */
static SDTransferState SD_GetStatus(void)
{
  SDCardState cardstate =  SD_CARD_TRANSFER;

  cardstate = SD_GetState();

  if (cardstate == SD_CARD_TRANSFER)
  {
    return(SD_TRANSFER_OK);
  }
  else if(cardstate == SD_CARD_ERROR)
  {
    return (SD_TRANSFER_ERROR);
  }
  else
  {
    return(SD_TRANSFER_BUSY);
  }
}

/**
  * @brief  Returns the current card's state.
  * @param  None
  * @retval SDCardState: SD Card Error or SD Card Current State.
  */
static SDCardState SD_GetState(void)
{
  uint32_t resp1 = 0;

  if(SD_Detect()== SD_PRESENT)
  {
	if (SD_SendStatus(&resp1) != SD_OK)
    {
      return SD_CARD_ERROR;
    }
    else
    {
      return (SDCardState)((resp1 >> 9) & 0x0F);
    }
  }
  else
  {
	  return SD_CARD_ERROR;
  }
}

/**
 * @brief  Detect if SD card is correctly plugged in the memory slot.
 * @param  None
 * @retval Return if SD is detected or not
 */
static uint8_t SD_Detect(void)
{
  __IO uint8_t status = SD_PRESENT;

  /*!< Check GPIO to detect SD */
  if ( gpio_get(SD_DETECT_GPIO_PORT, SD_DETECT_PIN) )
  {
    status = SD_NOT_PRESENT;
  }
  return status;
}

/**
  * @brief  Enquires cards about their operating voltage and configures
  *   clock controls.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_PowerON(void)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t response = 0, count = 0, validvoltage = 0;
  uint32_t SDType = SD_STD_CAPACITY;

  /*!< Power ON Sequence -----------------------------------------------------*/
  /*!< Configure the SDIO peripheral */
  /*!< SDIO_CK = SDIOCLK / (SDIO_INIT_CLK_DIV + 2) */
  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
  /*!< SDIO_CK for initialization should not exceed 400 KHz */
  sdio_init( SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
		  SDIO_BusWide_1b, SDIO_HardwareFlowControl_Disable, SDIO_INIT_CLK_DIV );


  /*!< Set Power State to ON */
  sdio_set_power_state(SDIO_PowerState_ON);

  for(int w=0; w<16; w++ ) nop();

  /*!< Enable SDIO Clock */
  sdio_clock_cmd(ENABLE);


  /*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
  /*!< No CMD response required */
  sdio_send_command( 0x00, SD_CMD_GO_IDLE_STATE, SDIO_Response_No, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdError();

  if (errorstatus != SD_OK)
  {
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return(errorstatus);
  }
  /*!< CMD8: SEND_IF_COND ----------------------------------------------------*/
  /*!< Send CMD8 to verify SD card interface operating condition */
  /*!< Argument: - [31:12]: Reserved (shall be set to '0')
               - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
               - [7:0]: Check Pattern (recommended 0xAA) */
  /*!< CMD Response: R7 */
  sdio_send_command( SD_CHECK_PATTERN, SDIO_SEND_IF_COND, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp7Error();
  if (errorstatus == SD_OK)
  {
    CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; /*!< SD Card 2.0 */
    SDType = SD_HIGH_CAPACITY;
  }
  else
  {
    /*!< CMD55 */
    sdio_send_command( 0x00, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
  }
  /*!< CMD55 */
  sdio_send_command( 0x00, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
  /*!< If errorstatus is Command TimeOut, it is a MMC card */
  /*!< If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch)
     or SD card 1.x */
  if (errorstatus == SD_OK)
  {
    /*!< SD CARD */
    /*!< Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
    while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
    {
      /*!< SEND CMD55 APP_CMD with RCA as 0 */
      sdio_send_command( 0x00, SD_CMD_APP_CMD, SDIO_Response_Short,SDIO_Wait_No, SDIO_CPSM_Enable );
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      sdio_send_command( SD_VOLTAGE_WINDOW_SD | SDType, SD_CMD_SD_APP_OP_COND,
    		  SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );

      errorstatus = CmdResp3Error();
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      response = sdio_get_response(SDIO_RESP1);
      validvoltage = (((response >> 31) == 1) ? 1 : 0);
      count++;
    }
    if (count >= SD_MAX_VOLT_TRIAL)
    {
      errorstatus = SD_INVALID_VOLTRANGE;
      return(errorstatus);
    }

    if (response &= SD_HIGH_CAPACITY)
    {
      CardType = SDIO_HIGH_CAPACITY_SD_CARD;
    }

  }/*!< else MMC Card */

  return(errorstatus);
}

/**
  * @brief  Turns the SDIO output signals off.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_PowerOFF(void)
{
  /*!< Set Power State to OFF */
  sdio_set_power_state(SDIO_PowerState_OFF);
  return SD_OK;
}

/**
  * @brief  Intialises all cards or single card as the case may be Card(s) come
  *         into standby state.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_InitializeCards(void)
{
  sdcard_err errorstatus = SD_OK;
  uint16_t rca = 0x01;
  if (sdio_get_power_state() == SDIO_PowerState_OFF)
  {
    errorstatus = SD_REQUEST_NOT_APPLICABLE;
    return(errorstatus);
  }
  if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
  {
    /*!< Send CMD2 ALL_SEND_CID */
    sdio_send_command( 0x00, SD_CMD_ALL_SEND_CID, SDIO_Response_Long, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = CmdResp2Error();
    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
    CID_Tab[0] = sdio_get_response(SDIO_RESP1);
    CID_Tab[1] = sdio_get_response(SDIO_RESP2);
    CID_Tab[2] = sdio_get_response(SDIO_RESP3);
    CID_Tab[3] = sdio_get_response(SDIO_RESP4);
  }
  if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) ||  (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||  (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType)
      ||  (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
    /*!< Send CMD3 SET_REL_ADDR with argument 0 */
    /*!< SD Card publishes its RCA. */
    sdio_send_command( 0x00, SD_CMD_SET_REL_ADDR, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
  }
  if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
  {
    RCA = rca;
    /*!< Send CMD9 SEND_CSD with argument as card's RCA */
    sdio_send_command(rca << 16, SD_CMD_SEND_CSD, SDIO_Response_Long, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = CmdResp2Error();
    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
    CSD_Tab[0] = sdio_get_response(SDIO_RESP1);
    CSD_Tab[1] = sdio_get_response(SDIO_RESP2);
    CSD_Tab[2] = sdio_get_response(SDIO_RESP3);
    CSD_Tab[3] = sdio_get_response(SDIO_RESP4);
  }
  errorstatus = SD_OK; /*!< All cards get intialized */
  return(errorstatus);
}

/**
  * @brief  Returns information about specific card.
  * @param  cardinfo: pointer to a SD_CardInfo structure that contains all SD card
  *         information.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_GetCardInfo(SD_CardInfo *cardinfo)
{
  sdcard_err errorstatus = SD_OK;
  uint8_t tmp = 0;

  cardinfo->CardType = (uint8_t)CardType;
  cardinfo->RCA = (uint16_t)RCA;

  /*!< Byte 0 */
  tmp = (uint8_t)((CSD_Tab[0] & 0xFF000000) >> 24);
  cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
  cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
  cardinfo->SD_csd.Reserved1 = tmp & 0x03;

  /*!< Byte 1 */
  tmp = (uint8_t)((CSD_Tab[0] & 0x00FF0000) >> 16);
  cardinfo->SD_csd.TAAC = tmp;

  /*!< Byte 2 */
  tmp = (uint8_t)((CSD_Tab[0] & 0x0000FF00) >> 8);
  cardinfo->SD_csd.NSAC = tmp;

  /*!< Byte 3 */
  tmp = (uint8_t)(CSD_Tab[0] & 0x000000FF);
  cardinfo->SD_csd.MaxBusClkFrec = tmp;

  /*!< Byte 4 */
  tmp = (uint8_t)((CSD_Tab[1] & 0xFF000000) >> 24);
  cardinfo->SD_csd.CardComdClasses = tmp << 4;

  /*!< Byte 5 */
  tmp = (uint8_t)((CSD_Tab[1] & 0x00FF0000) >> 16);
  cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
  cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;

  /*!< Byte 6 */
  tmp = (uint8_t)((CSD_Tab[1] & 0x0000FF00) >> 8);
  cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
  cardinfo->SD_csd.Reserved2 = 0; /*!< Reserved */

  if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0))
  {
    cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;

    /*!< Byte 7 */
    tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
    cardinfo->SD_csd.DeviceSize |= (tmp) << 2;

    /*!< Byte 8 */
    tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);
    cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;

    cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
    cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);

    /*!< Byte 9 */
    tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);
    cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
    cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
    cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
    /*!< Byte 10 */
    tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);
    cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;

    cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ;
    cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
    cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
    cardinfo->CardCapacity *= cardinfo->CardBlockSize;
  }
  else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    /*!< Byte 7 */
    tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
    cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;

    /*!< Byte 8 */
    tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);

    cardinfo->SD_csd.DeviceSize |= (tmp << 8);

    /*!< Byte 9 */
    tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);

    cardinfo->SD_csd.DeviceSize |= (tmp);

    /*!< Byte 10 */
    tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);

    cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024;
    cardinfo->CardBlockSize = 512;
  }


  cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;

  /*!< Byte 11 */
  tmp = (uint8_t)(CSD_Tab[2] & 0x000000FF);
  cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
  cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);

  /*!< Byte 12 */
  tmp = (uint8_t)((CSD_Tab[3] & 0xFF000000) >> 24);
  cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
  cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
  cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;

  /*!< Byte 13 */
  tmp = (uint8_t)((CSD_Tab[3] & 0x00FF0000) >> 16);
  cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
  cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.Reserved3 = 0;
  cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);

  /*!< Byte 14 */
  tmp = (uint8_t)((CSD_Tab[3] & 0x0000FF00) >> 8);
  cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
  cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
  cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
  cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
  cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
  cardinfo->SD_csd.ECC = (tmp & 0x03);

  /*!< Byte 15 */
  tmp = (uint8_t)(CSD_Tab[3] & 0x000000FF);
  cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
  cardinfo->SD_csd.Reserved4 = 1;


  /*!< Byte 0 */
  tmp = (uint8_t)((CID_Tab[0] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ManufacturerID = tmp;

  /*!< Byte 1 */
  tmp = (uint8_t)((CID_Tab[0] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.OEM_AppliID = tmp << 8;

  /*!< Byte 2 */
  tmp = (uint8_t)((CID_Tab[0] & 0x000000FF00) >> 8);
  cardinfo->SD_cid.OEM_AppliID |= tmp;

  /*!< Byte 3 */
  tmp = (uint8_t)(CID_Tab[0] & 0x000000FF);
  cardinfo->SD_cid.ProdName1 = tmp << 24;

  /*!< Byte 4 */
  tmp = (uint8_t)((CID_Tab[1] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ProdName1 |= tmp << 16;

  /*!< Byte 5 */
  tmp = (uint8_t)((CID_Tab[1] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.ProdName1 |= tmp << 8;

  /*!< Byte 6 */
  tmp = (uint8_t)((CID_Tab[1] & 0x0000FF00) >> 8);
  cardinfo->SD_cid.ProdName1 |= tmp;

  /*!< Byte 7 */
  tmp = (uint8_t)(CID_Tab[1] & 0x000000FF);
  cardinfo->SD_cid.ProdName2 = tmp;

  /*!< Byte 8 */
  tmp = (uint8_t)((CID_Tab[2] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ProdRev = tmp;

  /*!< Byte 9 */
  tmp = (uint8_t)((CID_Tab[2] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.ProdSN = tmp << 24;

  /*!< Byte 10 */
  tmp = (uint8_t)((CID_Tab[2] & 0x0000FF00) >> 8);
  cardinfo->SD_cid.ProdSN |= tmp << 16;

  /*!< Byte 11 */
  tmp = (uint8_t)(CID_Tab[2] & 0x000000FF);
  cardinfo->SD_cid.ProdSN |= tmp << 8;

  /*!< Byte 12 */
  tmp = (uint8_t)((CID_Tab[3] & 0xFF000000) >> 24);
  cardinfo->SD_cid.ProdSN |= tmp;

  /*!< Byte 13 */
  tmp = (uint8_t)((CID_Tab[3] & 0x00FF0000) >> 16);
  cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
  cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;

  /*!< Byte 14 */
  tmp = (uint8_t)((CID_Tab[3] & 0x0000FF00) >> 8);
  cardinfo->SD_cid.ManufactDate |= tmp;

  /*!< Byte 15 */
  tmp = (uint8_t)(CID_Tab[3] & 0x000000FF);
  cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
  cardinfo->SD_cid.Reserved2 = 1;

  return(errorstatus);
}

/**
  * @brief  Enables wide bus opeartion for the requeseted card if supported by
  *         card.
  * @param  WideMode: Specifies the SD card wide bus mode.
  *   This parameter can be one of the following values:
  *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
  *     @arg SDIO_BusWide_4b: 4-bit data transfer
  *     @arg SDIO_BusWide_1b: 1-bit data transfer
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_GetCardStatus(SD_CardStatus *cardstatus)
{
  sdcard_err errorstatus = SD_OK;
  uint8_t tmp = 0;

  errorstatus = SD_SendSDStatus((uint32_t *)SDSTATUS_Tab);

  if (errorstatus  != SD_OK)
  {
    return(errorstatus);
  }

  /*!< Byte 0 */
  tmp = (uint8_t)((SDSTATUS_Tab[0] & 0xC0) >> 6);
  cardstatus->DAT_BUS_WIDTH = tmp;

  /*!< Byte 0 */
  tmp = (uint8_t)((SDSTATUS_Tab[0] & 0x20) >> 5);
  cardstatus->SECURED_MODE = tmp;

  /*!< Byte 2 */
  tmp = (uint8_t)((SDSTATUS_Tab[2] & 0xFF));
  cardstatus->SD_CARD_TYPE = tmp << 8;

  /*!< Byte 3 */
  tmp = (uint8_t)((SDSTATUS_Tab[3] & 0xFF));
  cardstatus->SD_CARD_TYPE |= tmp;

  /*!< Byte 4 */
  tmp = (uint8_t)(SDSTATUS_Tab[4] & 0xFF);
  cardstatus->SIZE_OF_PROTECTED_AREA = tmp << 24;

  /*!< Byte 5 */
  tmp = (uint8_t)(SDSTATUS_Tab[5] & 0xFF);
  cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 16;

  /*!< Byte 6 */
  tmp = (uint8_t)(SDSTATUS_Tab[6] & 0xFF);
  cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 8;

  /*!< Byte 7 */
  tmp = (uint8_t)(SDSTATUS_Tab[7] & 0xFF);
  cardstatus->SIZE_OF_PROTECTED_AREA |= tmp;

  /*!< Byte 8 */
  tmp = (uint8_t)((SDSTATUS_Tab[8] & 0xFF));
  cardstatus->SPEED_CLASS = tmp;

  /*!< Byte 9 */
  tmp = (uint8_t)((SDSTATUS_Tab[9] & 0xFF));
  cardstatus->PERFORMANCE_MOVE = tmp;

  /*!< Byte 10 */
  tmp = (uint8_t)((SDSTATUS_Tab[10] & 0xF0) >> 4);
  cardstatus->AU_SIZE = tmp;

  /*!< Byte 11 */
  tmp = (uint8_t)(SDSTATUS_Tab[11] & 0xFF);
  cardstatus->ERASE_SIZE = tmp << 8;

  /*!< Byte 12 */
  tmp = (uint8_t)(SDSTATUS_Tab[12] & 0xFF);
  cardstatus->ERASE_SIZE |= tmp;

  /*!< Byte 13 */
  tmp = (uint8_t)((SDSTATUS_Tab[13] & 0xFC) >> 2);
  cardstatus->ERASE_TIMEOUT = tmp;

  /*!< Byte 13 */
  tmp = (uint8_t)((SDSTATUS_Tab[13] & 0x3));
  cardstatus->ERASE_OFFSET = tmp;

  return(errorstatus);
}

/**
  * @brief  Enables wide bus opeartion for the requeseted card if supported by
  *         card.
  * @param  WideMode: Specifies the SD card wide bus mode.
  *   This parameter can be one of the following values:
  *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
  *     @arg SDIO_BusWide_4b: 4-bit data transfer
  *     @arg SDIO_BusWide_1b: 1-bit data transfer
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_EnableWideBusOperation(uint32_t WideMode)
{
  sdcard_err errorstatus = SD_OK;

  /*!< MMC Card doesn't support this feature */
  if (SDIO_MULTIMEDIA_CARD == CardType)
  {
    errorstatus = SD_UNSUPPORTED_FEATURE;
    return(errorstatus);
  }
  else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
    if (SDIO_BusWide_8b == WideMode)
    {
      errorstatus = SD_UNSUPPORTED_FEATURE;
      return(errorstatus);
    }
    else if (SDIO_BusWide_4b == WideMode)
    {
      errorstatus = SDEnWideBus(ENABLE);

      if (SD_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
        sdio_init( SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
        		SDIO_BusWide_4b, SDIO_HardwareFlowControl_Disable, SDIO_TRANSFER_CLK_DIV );
      }
    }
    else
    {
      errorstatus = SDEnWideBus(DISABLE);

      if (SD_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
        sdio_init( SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
        		SDIO_BusWide_1b, SDIO_HardwareFlowControl_Disable, SDIO_TRANSFER_CLK_DIV );
      }
    }
  }
  return(errorstatus);
}

/**
  * @brief  Selects od Deselects the corresponding card.
  * @param  addr: Address of the Card to be selected.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_SelectDeselect(uint32_t addr)
{
  /*!< Send CMD7 SDIO_SEL_DESEL_CARD */
  sdio_send_command( addr, SD_CMD_SEL_DESEL_CARD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  sdcard_err errorstatus = CmdResp1Error(SD_CMD_SEL_DESEL_CARD);
  return(errorstatus);
}

/**
  * @brief  Allows to read one block from a specified address in a card. The Data
  *         transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  readbuff: pointer to the buffer that will contain the received data
  * @param  ReadAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_ReadBlock(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize)
{
  sdcard_err errorstatus = SD_OK;
#if defined (SD_POLLING_MODE)
  uint32_t count = 0, *tempbuff = (uint32_t *)readbuff;
#endif

  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 0;

  SDIO->DCTRL = 0x0;


  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    ReadAddr /= 512;
  }
  /* Set Block Size for Card */
  sdio_send_command( BlockSize, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }

  sdio_data_config( SD_DATATIMEOUT, BlockSize, 9 << 4, SDIO_TransferDir_ToSDIO,
		  SDIO_TransferMode_Block, SDIO_DPSM_Enable );

  /*!< Send CMD17 READ_SINGLE_BLOCK */
  sdio_send_command( ReadAddr, SD_CMD_READ_SINGLE_BLOCK, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable);
  errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
#if defined (SD_POLLING_MODE)
  /*!< In case of single block transfer, no need of stop transfer at all.*/
  /*!< Polling mode */
  while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (sdio_get_flag_status(SDIO_FLAG_RXFIFOHF) != RESET)
    {
      for (count = 0; count < 8; count++)
      {
        *(tempbuff + count) = sdio_read_data();
      }
      tempbuff += 8;
    }
  }

  if (sdio_get_flag_status(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_DCRCFAIL) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_RXOVERR) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_RXOVERR);
    errorstatus = SD_RX_OVERRUN;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_STBITERR) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }
  count = SD_DATATIMEOUT;
  while ((sdio_get_flag_status(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
  {
    *tempbuff = sdio_read_data();
    tempbuff++;
    count--;
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

#elif defined (SD_DMA_MODE)
    sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    sdio_dma_cmd(ENABLE);
    SD_LowLevel_DMA_RxConfig((uint32_t *)readbuff, BlockSize);
#endif

  return(errorstatus);
}

/**
  * @brief  Allows to read blocks from a specified address  in a card.  The Data
  *         transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  readbuff: pointer to the buffer that will contain the received data.
  * @param  ReadAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  sdcard_err errorstatus = SD_OK;
  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 1;
  SDIO->DCTRL = 0x0;
  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    ReadAddr /= 512;
  }
  /*!< Set Block Size for Card */
  sdio_send_command(BlockSize, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
  sdio_data_config( SD_DATATIMEOUT, NumberOfBlocks * BlockSize, 9 << 4,
	 SDIO_TransferDir_ToSDIO,  SDIO_TransferMode_Block, SDIO_DPSM_Enable );

  /*!< Send CMD18 READ_MULT_BLOCK with argument data address */
  sdio_send_command( ReadAddr, SD_CMD_READ_MULT_BLOCK, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  sdio_dma_cmd(ENABLE);
  SD_LowLevel_DMA_RxConfig((uint32_t *)readbuff, (NumberOfBlocks * BlockSize));
  return(errorstatus);
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished.
  *         This function should be called after SDIO_ReadMultiBlocks() function
  *         to insure that all data sent by the card are already transferred by
  *         the DMA controller.
  * @param  None.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_WaitReadOperation(void)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t timeout;

  timeout = SD_DATATIMEOUT;

  while ((DMAEndOfTransfer == 0x00) && (TransferEnd == 0) && (TransferError == SD_OK) && (timeout > 0))
  {
    timeout--;
  }

  DMAEndOfTransfer = 0x00;

  timeout = SD_DATATIMEOUT;

  while(((SDIO->STA & SDIO_FLAG_RXACT)) && (timeout > 0))
  {
    timeout--;
  }

  if (StopCondition == 1)
  {
    errorstatus = SD_StopTransfer();
    StopCondition = 0;
  }

  if ((timeout == 0) && (errorstatus == SD_OK))
  {
    errorstatus = SD_DATA_TIMEOUT;
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  if (TransferError != SD_OK)
  {
    return(TransferError);
  }
  else
  {
    return(errorstatus);
  }
}

/**
  * @brief  Allows to write one block starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  WriteAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_WriteBlock(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize)
{
  sdcard_err errorstatus = SD_OK;

#if defined (SD_POLLING_MODE)
  uint32_t bytestransferred = 0, count = 0, restwords = 0;
  uint32_t *tempbuff = (uint32_t *)writebuff;
#endif
  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 0;
  SDIO->DCTRL = 0x0;
  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    WriteAddr /= 512;
  }
  /* Set Block Size for Card */
  sdio_send_command( BlockSize, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
  /*!< Send CMD24 WRITE_SINGLE_BLOCK */
  sdio_send_command( WriteAddr, SD_CMD_WRITE_SINGLE_BLOCK, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  sdio_data_config( SD_DATATIMEOUT, BlockSize, 9 << 4, SDIO_TransferDir_ToCard,
		  SDIO_TransferMode_Block, SDIO_DPSM_Enable );
  /*!< In case of single data block transfer no need of stop command at all */
#if defined (SD_POLLING_MODE)
  while (!(SDIO->STA & (SDIO_FLAG_DBCKEND | SDIO_FLAG_TXUNDERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR)))
  {
    if (sdio_get_flag_status(SDIO_FLAG_TXFIFOHE) != RESET)
    {
      if ((512 - bytestransferred) < 32)
      {
        restwords = ((512 - bytestransferred) % 4 == 0) ? ((512 - bytestransferred) / 4) : (( 512 -  bytestransferred) / 4 + 1);
        for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
        {
          sdio_write_data(*tempbuff);
        }
      }
      else
      {
        for (count = 0; count < 8; count++)
        {
        	sdio_write_data(*(tempbuff + count));
        }
        tempbuff += 8;
        bytestransferred += 32;
      }
    }
  }
  if (sdio_get_flag_status(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_DCRCFAIL) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_TXUNDERR) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_TXUNDERR);
    errorstatus = SD_TX_UNDERRUN;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_STBITERR) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }
#elif defined (SD_DMA_MODE)
  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  SD_LowLevel_DMA_TxConfig((uint32_t *)writebuff, BlockSize);
  SDIO_DMACmd(ENABLE);
#endif

  return(errorstatus);
}

/**
  * @brief  Allows to write blocks starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode only.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  WriteAddr: Address from where data are to be read.
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_WriteMultiBlocks(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  sdcard_err errorstatus = SD_OK;

  TransferError = SD_OK;
  TransferEnd = 0;
  StopCondition = 1;
  SDIO->DCTRL = 0x0;
  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    BlockSize = 512;
    WriteAddr /= 512;
  }
  /* Set Block Size for Card */
  sdio_send_command( BlockSize, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
  /*!< To improve performance */
  sdio_send_command( RCA << 16, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  /*!< To improve performance */
  sdio_send_command( NumberOfBlocks, SD_CMD_SET_BLOCK_COUNT, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  /*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
  sdio_send_command( WriteAddr, SD_CMD_WRITE_MULT_BLOCK, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
  sdio_data_config( SD_DATATIMEOUT, NumberOfBlocks * BlockSize, 9 << 4,
		  SDIO_TransferDir_ToCard, SDIO_TransferMode_Block, SDIO_DPSM_Enable );
  sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  sdio_dma_cmd(ENABLE);
  SD_LowLevel_DMA_TxConfig((uint32_t *)writebuff, (NumberOfBlocks * BlockSize));
  return(errorstatus);
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished.
  *         This function should be called after SDIO_WriteBlock() and
  *         SDIO_WriteMultiBlocks() function to insure that all data sent by the
  *         card are already transferred by the DMA controller.
  * @param  None.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_WaitWriteOperation(void)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t timeout;
  timeout = SD_DATATIMEOUT;
  while ((DMAEndOfTransfer == 0x00) && (TransferEnd == 0) && (TransferError == SD_OK) && (timeout > 0))
  {
    timeout--;
  }
  DMAEndOfTransfer = 0x00;
  timeout = SD_DATATIMEOUT;
  while(((SDIO->STA & SDIO_FLAG_TXACT)) && (timeout > 0))
  {
    timeout--;
  }
  if (StopCondition == 1)
  {
    errorstatus = SD_StopTransfer();
    StopCondition = 0;
  }
  if ((timeout == 0) && (errorstatus == SD_OK))
  {
    errorstatus = SD_DATA_TIMEOUT;
  }
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  if (TransferError != SD_OK)
  {
    return(TransferError);
  }
  else
  {
    return(errorstatus);
  }
}

/**
  * @brief  Gets the cuurent data transfer state.
  * @param  None
  * @retval SDTransferState: Data Transfer state.
  *   This value can be:
  *        - SD_TRANSFER_OK: No data transfer is acting
  *        - SD_TRANSFER_BUSY: Data transfer is acting
  */
static SDTransferState SD_GetTransferState(void)
{
  if (SDIO->STA & (SDIO_FLAG_TXACT | SDIO_FLAG_RXACT))
  {
    return(SD_TRANSFER_BUSY);
  }
  else
  {
    return(SD_TRANSFER_OK);
  }
}

/**
  * @brief  Aborts an ongoing data transfer.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_StopTransfer(void)
{
  /*!< Send CMD12 STOP_TRANSMISSION  */
  sdio_send_command( 0x0, SD_CMD_STOP_TRANSMISSION, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  sdcard_err errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
  return(errorstatus);
}

/**
  * @brief  Allows to erase memory area specified for the given card.
  * @param  startaddr: the start address.
  * @param  endaddr: the end address.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_Erase(uint32_t startaddr, uint32_t endaddr)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t delay = 0;
  __IO uint32_t maxdelay = 0;
  uint8_t cardstate = 0;
  /*!< Check if the card coomnd class supports erase command */
  if (((CSD_Tab[1] >> 20) & SD_CCCC_ERASE) == 0)
  {
    errorstatus = SD_REQUEST_NOT_APPLICABLE;
    return(errorstatus);
  }
  maxdelay = 120000 / ((SDIO->CLKCR & 0xFF) + 2);
  if (sdio_get_response(SDIO_RESP1) & SD_CARD_LOCKED)
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }
  if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    startaddr /= 512;
    endaddr /= 512;
  }
  /*!< According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
  if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
  {
    /*!< Send CMD32 SD_ERASE_GRP_START with argument as addr  */
    sdio_send_command( startaddr, SD_CMD_SD_ERASE_GRP_START, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_START);
    if (errorstatus != SD_OK)
    {
      return(errorstatus);
    }
    /*!< Send CMD33 SD_ERASE_GRP_END with argument as addr  */
    sdio_send_command( endaddr, SD_CMD_SD_ERASE_GRP_END, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_END);
    if (errorstatus != SD_OK)
    {
      return(errorstatus);
    }
  }
  /*!< Send CMD38 ERASE */
  sdio_send_command( 0, SD_CMD_ERASE, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_ERASE);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  for (delay = 0; delay < maxdelay; delay++) nop();
  /*!< Wait till the card is in programming state */
  errorstatus = IsCardProgramming(&cardstate);
  delay = SD_DATATIMEOUT;
  while ((delay > 0) && (errorstatus == SD_OK) && ((SD_CARD_PROGRAMMING == cardstate) || (SD_CARD_RECEIVING == cardstate)))
  {
    errorstatus = IsCardProgramming(&cardstate);
    delay--;
  }

  return(errorstatus);
}

/**
  * @brief  Returns the current card's status.
  * @param  pcardstatus: pointer to the buffer that will contain the SD card
  *         status (Card Status register).
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_SendStatus(uint32_t *pcardstatus)
{
  sdcard_err errorstatus = SD_OK;
  if (pcardstatus == NULL)
  {
    errorstatus = SD_INVALID_PARAMETER;
    return(errorstatus);
  }
  sdio_send_command( RCA << 16, SD_CMD_SEND_STATUS, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  *pcardstatus = sdio_get_response(SDIO_RESP1);
  return(errorstatus);
}

/**
  * @brief  Returns the current SD card's status.
  * @param  psdstatus: pointer to the buffer that will contain the SD card status
  *         (SD Status register).
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_SendSDStatus(uint32_t *psdstatus)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t count = 0;
  if (sdio_get_response(SDIO_RESP1) & SD_CARD_LOCKED)
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }
  /*!< Set block size for card if it is not equal to current block size for card. */
  sdio_send_command( 64, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  /*!< CMD55 */
  sdio_send_command( RCA << 16, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  sdio_data_config( SD_DATATIMEOUT, 64, SDIO_DataBlockSize_64b,
		  SDIO_TransferDir_ToSDIO, SDIO_TransferMode_Block, SDIO_DPSM_Enable );

  /*!< Send ACMD13 SD_APP_STAUS  with argument as card's RCA.*/
  sdio_send_command( 0, SD_CMD_SD_APP_STAUS, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SD_APP_STAUS);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (sdio_get_flag_status(SDIO_FLAG_RXFIFOHF) != RESET)
    {
      for (count = 0; count < 8; count++)
      {
        *(psdstatus + count) = sdio_read_data();
      }
      psdstatus += 8;
    }
  }

  if (sdio_get_flag_status(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_DCRCFAIL) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_RXOVERR) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_RXOVERR);
    errorstatus = SD_RX_OVERRUN;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_STBITERR) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }
  count = SD_DATATIMEOUT;
  while ((sdio_get_flag_status(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
  {
    *psdstatus = sdio_read_data();
    psdstatus++;
    count--;
  }
  /*!< Clear all the static status flags*/
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  return(errorstatus);
}

/**
  * @brief  Allows to process all the interrupts that are high.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SD_ProcessIRQSrc(void)
{
  if (sdio_get_it_status(SDIO_IT_DATAEND) != RESET)
  {
    TransferError = SD_OK;
    sdio_clear_it_pending_bit(SDIO_IT_DATAEND);
    TransferEnd = 1;
  }
  else if (sdio_get_it_status(SDIO_IT_DCRCFAIL) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_DCRCFAIL);
    TransferError = SD_DATA_CRC_FAIL;
  }
  else if (sdio_get_it_status(SDIO_IT_DTIMEOUT) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_DTIMEOUT);
    TransferError = SD_DATA_TIMEOUT;
  }
  else if (sdio_get_it_status(SDIO_IT_RXOVERR) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_RXOVERR);
    TransferError = SD_RX_OVERRUN;
  }
  else if (sdio_get_it_status(SDIO_IT_TXUNDERR) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_TXUNDERR);
    TransferError = SD_TX_UNDERRUN;
  }
  else if (sdio_get_it_status(SDIO_IT_STBITERR) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_STBITERR);
    TransferError = SD_START_BIT_ERR;
  }
  sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
                SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
                SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
  return(TransferError);
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished.
  * @param  None.
  * @retval None.
  */
static void SD_ProcessDMAIRQ(void)
{
  if(DMA2->LISR & SD_SDIO_DMA_FLAG_TCIF)
  {
    DMAEndOfTransfer = 0x01;
    dma_clear_flag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF|SD_SDIO_DMA_FLAG_FEIF);
  }
}

/**
  * @brief  Checks for error conditions for CMD0.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err CmdError(void)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t timeout;
  timeout = SDIO_CMD0TIMEOUT; /*!< 10000 */
   while ((timeout > 0) && (sdio_get_flag_status(SDIO_FLAG_CMDSENT) == RESET))
  {
    timeout--;
  }
  if (timeout == 0)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R7 response.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err CmdResp7Error(void)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t status;
  uint32_t timeout = SDIO_CMD0TIMEOUT;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) && (timeout > 0))
  {
    timeout--;
    status = SDIO->STA;
  }

  if ((timeout == 0) || (status & SDIO_FLAG_CTIMEOUT))
  {
    /*!< Card is not V2.0 complient or card does not support the set voltage range */
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }

  if (status & SDIO_FLAG_CMDREND)
  {
    /*!< Card is SD V2.0 compliant */
    errorstatus = SD_OK;
    sdio_clear_flag(SDIO_FLAG_CMDREND);
    return(errorstatus);
  }
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R1 response.
  * @param  cmd: The sent command index.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err CmdResp1Error(uint8_t cmd)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t status;
  uint32_t response_r1;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    sdio_clear_flag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }
  /*!< Check response received is of desired command */
  if (sdio_get_command_response() != cmd)
  {
    errorstatus = SD_ILLEGAL_CMD;
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

  /*!< We have received response, retrieve it for analysis  */
  response_r1 = sdio_get_response(SDIO_RESP1);

  if ((response_r1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
  {
    return(errorstatus);
  }

  if (response_r1 & SD_OCR_ADDR_OUT_OF_RANGE)
  {
    return(SD_ADDR_OUT_OF_RANGE);
  }

  if (response_r1 & SD_OCR_ADDR_MISALIGNED)
  {
    return(SD_ADDR_MISALIGNED);
  }

  if (response_r1 & SD_OCR_BLOCK_LEN_ERR)
  {
    return(SD_BLOCK_LEN_ERR);
  }

  if (response_r1 & SD_OCR_ERASE_SEQ_ERR)
  {
    return(SD_ERASE_SEQ_ERR);
  }

  if (response_r1 & SD_OCR_BAD_ERASE_PARAM)
  {
    return(SD_BAD_ERASE_PARAM);
  }

  if (response_r1 & SD_OCR_WRITE_PROT_VIOLATION)
  {
    return(SD_WRITE_PROT_VIOLATION);
  }

  if (response_r1 & SD_OCR_LOCK_UNLOCK_FAILED)
  {
    return(SD_LOCK_UNLOCK_FAILED);
  }

  if (response_r1 & SD_OCR_COM_CRC_FAILED)
  {
    return(SD_COM_CRC_FAILED);
  }

  if (response_r1 & SD_OCR_ILLEGAL_CMD)
  {
    return(SD_ILLEGAL_CMD);
  }

  if (response_r1 & SD_OCR_CARD_ECC_FAILED)
  {
    return(SD_CARD_ECC_FAILED);
  }

  if (response_r1 & SD_OCR_CC_ERROR)
  {
    return(SD_CC_ERROR);
  }

  if (response_r1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
  {
    return(SD_GENERAL_UNKNOWN_ERROR);
  }

  if (response_r1 & SD_OCR_STREAM_READ_UNDERRUN)
  {
    return(SD_STREAM_READ_UNDERRUN);
  }

  if (response_r1 & SD_OCR_STREAM_WRITE_OVERRUN)
  {
    return(SD_STREAM_WRITE_OVERRUN);
  }

  if (response_r1 & SD_OCR_CID_CSD_OVERWRIETE)
  {
    return(SD_CID_CSD_OVERWRITE);
  }

  if (response_r1 & SD_OCR_WP_ERASE_SKIP)
  {
    return(SD_WP_ERASE_SKIP);
  }

  if (response_r1 & SD_OCR_CARD_ECC_DISABLED)
  {
    return(SD_CARD_ECC_DISABLED);
  }

  if (response_r1 & SD_OCR_ERASE_RESET)
  {
    return(SD_ERASE_RESET);
  }

  if (response_r1 & SD_OCR_AKE_SEQ_ERROR)
  {
    return(SD_AKE_SEQ_ERROR);
  }
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R3 (OCR) response.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err CmdResp3Error(void)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R2 (CID or CSD) response.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err CmdResp2Error(void)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    sdio_clear_flag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R6 (RCA) response.
  * @param  cmd: The sent command index.
  * @param  prca: pointer to the variable that will contain the SD card relative
  *         address RCA.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err CmdResp6Error(uint8_t cmd, uint16_t *prca)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t status;
  uint32_t response_r1;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    sdio_clear_flag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  /*!< Check response received is of desired command */
  if (sdio_get_command_response() != cmd)
  {
    errorstatus = SD_ILLEGAL_CMD;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

  /*!< We have received response, retrieve it.  */
  response_r1 = sdio_get_response(SDIO_RESP1);

  if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
  {
    *prca = (uint16_t) (response_r1 >> 16);
    return(errorstatus);
  }

  if (response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR)
  {
    return(SD_GENERAL_UNKNOWN_ERROR);
  }

  if (response_r1 & SD_R6_ILLEGAL_CMD)
  {
    return(SD_ILLEGAL_CMD);
  }

  if (response_r1 & SD_R6_COM_CRC_FAILED)
  {
    return(SD_COM_CRC_FAILED);
  }

  return(errorstatus);
}

/**
  * @brief  Enables or disables the SDIO wide bus mode.
  * @param  NewState: new state of the SDIO wide bus mode.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err SDEnWideBus(FunctionalState NewState)
{
  sdcard_err errorstatus = SD_OK;

  uint32_t scr[2] = {0, 0};

  if (sdio_get_response(SDIO_RESP1) & SD_CARD_LOCKED)
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }

  /*!< Get SCR Register */
  errorstatus = FindSCR(RCA, scr);

  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }

  /*!< If wide bus operation to be enabled */
  if (NewState == ENABLE)
  {
    /*!< If requested card supports wide bus operation */
    if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)
    {
      /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
      sdio_send_command( RCA << 16, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
      sdio_send_command( 0x2, SD_CMD_APP_SD_SET_BUSWIDTH, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
      errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      return(errorstatus);
    }
    else
    {
      errorstatus = SD_REQUEST_NOT_APPLICABLE;
      return(errorstatus);
    }
  }   /*!< If wide bus operation to be disabled */
  else
  {
    /*!< If requested card supports 1 bit mode operation */
    if ((scr[1] & SD_SINGLE_BUS_SUPPORT) != SD_ALLZERO)
    {
      /*!< Send CMD55 APP_CMD with argument as card's RCA.*/
      sdio_send_command( RCA << 16, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
      errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }

      sdio_send_command( 0x00, SD_CMD_APP_SD_SET_BUSWIDTH, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
      errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      return(errorstatus);
    }
    else
    {
      errorstatus = SD_REQUEST_NOT_APPLICABLE;
      return(errorstatus);
    }
  }
}

/**
  * @brief  Checks if the SD card is in programming state.
  * @param  pstatus: pointer to the variable that will contain the SD card state.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err IsCardProgramming(uint8_t *pstatus)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t respR1 = 0, status = 0;
  sdio_send_command( RCA << 16, SD_CMD_SEND_STATUS, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  status = SDIO->STA;
  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }
  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = SD_CMD_RSP_TIMEOUT;
    sdio_clear_flag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = SD_CMD_CRC_FAIL;
    sdio_clear_flag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }
  status = (uint32_t)sdio_get_command_response();
  /*!< Check response received is of desired command */
  if (status != SD_CMD_SEND_STATUS)
  {
    errorstatus = SD_ILLEGAL_CMD;
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  /*!< We have received response, retrieve it for analysis  */
  respR1 = sdio_get_response(SDIO_RESP1);
  /*!< Find out card status */
  *pstatus = (uint8_t) ((respR1 >> 9) & 0x0000000F);
  if ((respR1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
  {
    return(errorstatus);
  }
  if (respR1 & SD_OCR_ADDR_OUT_OF_RANGE)
  {
    return(SD_ADDR_OUT_OF_RANGE);
  }
  if (respR1 & SD_OCR_ADDR_MISALIGNED)
  {
    return(SD_ADDR_MISALIGNED);
  }
  if (respR1 & SD_OCR_BLOCK_LEN_ERR)
  {
    return(SD_BLOCK_LEN_ERR);
  }
  if (respR1 & SD_OCR_ERASE_SEQ_ERR)
  {
    return(SD_ERASE_SEQ_ERR);
  }
  if (respR1 & SD_OCR_BAD_ERASE_PARAM)
  {
    return(SD_BAD_ERASE_PARAM);
  }
  if (respR1 & SD_OCR_WRITE_PROT_VIOLATION)
  {
    return(SD_WRITE_PROT_VIOLATION);
  }
  if (respR1 & SD_OCR_LOCK_UNLOCK_FAILED)
  {
    return(SD_LOCK_UNLOCK_FAILED);
  }
  if (respR1 & SD_OCR_COM_CRC_FAILED)
  {
    return(SD_COM_CRC_FAILED);
  }

  if (respR1 & SD_OCR_ILLEGAL_CMD)
  {
    return(SD_ILLEGAL_CMD);
  }
  if (respR1 & SD_OCR_CARD_ECC_FAILED)
  {
    return(SD_CARD_ECC_FAILED);
  }
  if (respR1 & SD_OCR_CC_ERROR)
  {
    return(SD_CC_ERROR);
  }
  if (respR1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
  {
    return(SD_GENERAL_UNKNOWN_ERROR);
  }

  if (respR1 & SD_OCR_STREAM_READ_UNDERRUN)
  {
    return(SD_STREAM_READ_UNDERRUN);
  }
  if (respR1 & SD_OCR_STREAM_WRITE_OVERRUN)
  {
    return(SD_STREAM_WRITE_OVERRUN);
  }
  if (respR1 & SD_OCR_CID_CSD_OVERWRIETE)
  {
    return(SD_CID_CSD_OVERWRITE);
  }

  if (respR1 & SD_OCR_WP_ERASE_SKIP)
  {
    return(SD_WP_ERASE_SKIP);
  }
  if (respR1 & SD_OCR_CARD_ECC_DISABLED)
  {
    return(SD_CARD_ECC_DISABLED);
  }
  if(respR1 & SD_OCR_ERASE_RESET)
  {
    return(SD_ERASE_RESET);
  }
  if (respR1 & SD_OCR_AKE_SEQ_ERROR)
  {
    return(SD_AKE_SEQ_ERROR);
  }
  return(errorstatus);
}

/**
  * @brief  Find the SD card SCR register value.
  * @param  rca: selected card address.
  * @param  pscr: pointer to the buffer that will contain the SCR value.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err FindSCR(uint16_t rca, uint32_t *pscr)
{
  uint32_t index = 0;
  sdcard_err errorstatus = SD_OK;
  uint32_t tempscr[2] = {0, 0};
  /*!< Set Block Size To 8 Bytes */
  /*!< Send CMD55 APP_CMD with argument as card's RCA */
  sdio_send_command( 8, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  /*!< Send CMD55 APP_CMD with argument as card's RCA */
  sdio_send_command( RCA << 16, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  sdio_data_config( SD_DATATIMEOUT, 8, SDIO_DataBlockSize_8b, SDIO_TransferDir_ToSDIO,
		  SDIO_TransferMode_Block, SDIO_DPSM_Enable );

  /*!< Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
  sdio_send_command( 0x0, SD_CMD_SD_APP_SEND_SCR, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  while (!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (sdio_get_flag_status(SDIO_FLAG_RXDAVL) != RESET)
    {
      *(tempscr + index) = sdio_read_data();
      index++;
    }
  }
  if (sdio_get_flag_status(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    sdio_clear_flag(SDIO_FLAG_DTIMEOUT);
    errorstatus = SD_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_DCRCFAIL) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_DCRCFAIL);
    errorstatus = SD_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_RXOVERR) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_RXOVERR);
    errorstatus = SD_RX_OVERRUN;
    return(errorstatus);
  }
  else if (sdio_get_flag_status(SDIO_FLAG_STBITERR) != RESET)
  {
	sdio_clear_flag(SDIO_FLAG_STBITERR);
    errorstatus = SD_START_BIT_ERR;
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  *(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);
  *(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);
  return(errorstatus);
}

/**
  * @brief  Converts the number of bytes in power of two and returns the power.
  * @param  NumberOfBytes: number of bytes.
  * @retval None
  */
static uint8_t convert_from_bytes_to_power_of_two(uint16_t NumberOfBytes)
{
  uint8_t count = 0;

  while (NumberOfBytes != 1)
  {
    NumberOfBytes >>= 1;
    count++;
  }
  return(count);
}

/*--------------------------------------------------------------------*/
//ISIX driver public fat funcs
/*--------------------------------------------------------------------*/
//Initialize the SDIO card driver
int isix_sdio_card_driver_init(void)
{
	SD_CardInfo ci;
	memset(&ci, 0, sizeof(ci));
	nvic_set_priority( SDIO_IRQn, 1, 7 );
	nvic_irq_enable( SDIO_IRQn, true );
	nvic_set_priority( DMA2_Stream3_IRQn, 1, 7 );
	nvic_irq_enable( DMA2_Stream3_IRQn, true );
	return SD_Init( &ci );
}
/*--------------------------------------------------------------------*/
//SDIO card driver read
int isix_sdio_card_driver_read( void *buf, unsigned long sector, int count )
{
	sdcard_err e =  SD_ReadMultiBlocks( buf, sector, 512, count );
	if( e ) return e;
	e = SD_WaitReadOperation();
	e = SD_GetStatus();
	dbprintf("GSSSS %i", e );
	return e;
}
/*--------------------------------------------------------------------*/
