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
/* Configure driver model */
#ifndef SDDRV_TRANSFER_MODE
#define SDDRV_TRANSFER_MODE SDDRV_DMA_MODE
#endif

//Config section
#ifndef SDDRV_INIT_CLK_DIV
#define SDDRV_INIT_CLK_DIV                0x76
#endif

//Normal transfer CLK divider
#ifndef SDDRV_TRANSFER_CLK_DIV
#define SDDRV_TRANSFER_CLK_DIV            0x1
#endif

//Detect GPIO port
#ifndef SDDRV_DETECT_GPIO_PORT
#define SDDRV_DETECT_GPIO_PORT GPIOC
#endif

//Detect GPIO PIN
#ifndef SDDRV_DETECT_PIN
#define SDDRV_DETECT_PIN 13
#endif

//Interrupt priorities
#ifndef SDDRV_IRQ_PRIO
#define SDDRV_IRQ_PRIO 1
#endif
#ifndef SDDRV_IRQ_SUBPRIO
#define SDDRV_IRQ_SUBPRIO 7
#endif

//Data timeout
#ifndef SDDRV_HW_DATATIMEOUT
#define SDDRV_HW_DATATIMEOUT    0x003fffff
#endif

//Data timeout
#ifndef SDDRV_SEM_DATATIMEOUT
#define SDDRV_SEM_DATATIMEOUT    2000
#endif


//USE dma stream number valid no are 3 or 6
#ifndef SDDRV_DMA_STREAM_NO
#define SDDRV_DMA_STREAM_NO 3
#endif

/* ------------------------------------------------------------------ */
//Extra advanced API can be anabled
//#define  SDDRV_ADVANCED_SD_API

#define SD_RDWR_BLOCK_SIZE 512

#if (SDDRV_DMA_STREAM_NO != 3 && SDDRV_DMA_STREAM_NO!=6) && (defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2) )
#error Only Stream3 or Stream6 is connected to SDIO irq
#endif
/* ------------------------------------------------------------------ */

static uint32_t card_type =  SDIO_STD_CAPACITY_SD_CARD_V1_1;
static uint32_t csd_tab[4], cid_tab[4], RCA = 0;
static volatile  uint32_t stop_condition = 0;
static volatile sdcard_err transfer_error = SD_OK;
static volatile  uint32_t transfer_end = 0, dma_end_of_transfer = 0;
//Complete transfer sem
static sem_t*  	tcomplete_sem;
//API lock semaphore
static sem_t* 	tlock_sem;
/* ------------------------------------------------------------------ */
static sdcard_err cmd_error(void);
static sdcard_err cmd_resp1_error(uint8_t cmd);
static sdcard_err cmd_resp7_error(void);
static sdcard_err cmd_resp3_error(void);
static sdcard_err cmd_resp2_error(void);
static sdcard_err cmd_resp6_error(uint8_t cmd, uint16_t *prca);
static sdcard_err sd_en_wide_bus(FunctionalState NewState);
static sdcard_err find_scr(uint16_t rca, uint32_t *pscr);
static  sdcard_state sd_get_state(void);
static uint8_t sd_detect(void);
static sdcard_err sd_power_on(void);
static sdcard_err sd_get_card_info(void *cardinfo, scard_info_field req );
static sdcard_err sd_initialize_cards(void);
static sdcard_err sd_enable_wide_bus_operation(uint32_t WideMode);
static sdcard_err sd_select_deselect(uint32_t addr);
static sdcard_err sd_send_status(uint32_t *pcardstatus);
#ifdef SDDRV_ADVANCED_SD_API
static sdcard_err is_card_programming(uint8_t *pstatus);
static SDTransferState sd_get_transfer_state(void);
static sdcard_err sd_send_sd_status(uint32_t *psdstatus);
#endif
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
static sdcard_err sd_stop_transfer(void);
static void sd_process_dma_irq(void);
static sdcard_err sd_process_irq_src(void);
#endif
/* ------------------------------------------------------------------ */
/**
  * @brief  DeInitializes the SDIO interface.
  * @param  None
  * @retval None
  */
static void sd_deinit(void)
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
	rcc_ahb1_periph_clock_cmd( RCC_AHB1Periph_DMA2, DISABLE );
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
/*--------------------------------------------------------------------*/
/* Initialize the card in slot */
static sdcard_err sd_card_init(void)
{
	 sdcard_err errorstatus = SD_OK;
	 sdio_deinit();
	 errorstatus = sd_power_on();
	 if (errorstatus != SD_OK)
	 {
	   /*!< CMD Response TimeOut (wait for CMDSENT flag) */
	   return(errorstatus);
	 }
	 errorstatus = sd_initialize_cards();
	 if (errorstatus != SD_OK)
	 {
	   /*!< CMD Response TimeOut (wait for CMDSENT flag) */
	   return(errorstatus);
	 }
	  /*!< Configure the SDIO peripheral */
	  /*!< SDIO_CK = SDIOCLK / (SDDRV_TRANSFER_CLK_DIV + 2) */
	  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
	 sdio_init(SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
		  SDIO_BusWide_1b, SDIO_HardwareFlowControl_Disable, SDDRV_TRANSFER_CLK_DIV );
	 errorstatus = sd_select_deselect((uint32_t) (RCA << 16));
	 if (errorstatus == SD_OK)
	 {
	   errorstatus = sd_enable_wide_bus_operation(SDIO_BusWide_4b);
	 }
	 return(errorstatus);
}
/*--------------------------------------------------------------------*/
/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for data
  *         transfer).
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static void sd_hw_init(void)
{
  /* SDIO Peripheral Low Level Init */
  rcc_ahb1_periph_clock_cmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
  gpio_clock_enable( SDDRV_DETECT_GPIO_PORT, true );
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
  gpio_config( SDDRV_DETECT_GPIO_PORT, SDDRV_DETECT_PIN, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 0, 0 );
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
  rcc_ahb1_periph_clock_cmd( RCC_AHB1Periph_DMA2, true );
#endif
#else
#error F1 not implemented yet
#endif
  rcc_apb2_periph_clock_cmd( RCC_APB2Periph_SDIO, true );
}

/*--------------------------------------------------------------------*/
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE

#if (defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2))

#define _SD_SDIO_cat(x, y) x##y
#define _SD_SDIO_cat3(x, y, z) x##y##z
#define _SD_SDIO_STREAM_prv(x, y)  _SD_SDIO_cat(x, y)
#define _SD_SDIO_STREAM_prv3(x, y, z)  _SD_SDIO_cat3(x, y, z)
#define _SD_SDIO_ISR_VECT_expand(x, y, z ) _SD_SDIO_cat3(x, y, z)
#define _SD_SDIO_isr_dma_vector _SD_SDIO_ISR_VECT_expand(dma2_stream,SDDRV_DMA_STREAM_NO,_isr_vector)

#define SD_SDIO_DMA_FLAG_TCIF         _SD_SDIO_STREAM_prv( DMA_FLAG_TCIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_FEIF         _SD_SDIO_STREAM_prv( DMA_FLAG_FEIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_STREAM            _SD_SDIO_STREAM_prv( DMA2_Stream, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_HTIF         _SD_SDIO_STREAM_prv( DMA_FLAG_HTIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_DMEIF        _SD_SDIO_STREAM_prv( DMA_FLAG_DMEIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_FLAG_TEIF         _SD_SDIO_STREAM_prv( DMA_FLAG_TEIF, SDDRV_DMA_STREAM_NO )
#define SD_SDIO_DMA_CHANNEL           DMA_Channel_4
#define SD_SDIO_DMA_IRQn              _SD_SDIO_STREAM_prv3(DMA2_Stream,SDDRV_DMA_STREAM_NO,_IRQn)
#define SDIO_FIFO_ADDRESS             ((void*)0x40012C80)
#if (SDDRV_DMA_STREAM_NO<=3)
#define SD_SDIO_DMA_SR DMA2->LISR
#else
#define SD_SDIO_DMA_SR DMA2->HISR
#endif
/**
  * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
  * @param  BufferSRC: pointer to the source buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
static void sd_lowlevel_dma_tx_config(const uint32_t *buffer_src, uint32_t buf_size)
{
   //DMA transfer len managed by HW
   (void)buf_size;
  dma_clear_flag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF |
		  SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);
  /* DMA2 Stream3  or Stream6 disable */
  dma_cmd(SD_SDIO_DMA_STREAM, DISABLE);
  /* DMA2 Stream3  or Stream6 Config */
  dma_deinit( SD_SDIO_DMA_STREAM );
  dma_init( SD_SDIO_DMA_STREAM , DMA_DIR_MemoryToPeripheral | DMA_PeripheralInc_Disable |
		    SD_SDIO_DMA_CHANNEL |DMA_MemoryInc_Enable | DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
		  	DMA_Mode_Normal | DMA_Priority_Medium | DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4,
		  	DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full, 0, SDIO_FIFO_ADDRESS, (uint32_t*)buffer_src );
  dma_it_config(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
  dma_flow_controller_config(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
  /* DMA2 Stream3  or Stream6 enable */
  dma_cmd(SD_SDIO_DMA_STREAM, ENABLE);
}
/*--------------------------------------------------------------------*/
/**
  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
  * @param  BufferDST: pointer to the destination buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
static void sd_lowlevel_dma_rx_config(uint32_t *buf_dst, uint32_t buf_size)
{
  //DMA transfer len managed by HW
  (void)buf_size;
  dma_clear_flag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);
  /* DMA2 Stream3  or Stream6 disable */
  dma_cmd(SD_SDIO_DMA_STREAM, DISABLE);
  /* DMA2 Stream3 or Stream6 Config */
  dma_deinit(SD_SDIO_DMA_STREAM);
  dma_init(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_CHANNEL | DMA_DIR_PeripheralToMemory | DMA_PeripheralInc_Disable |
		  DMA_MemoryInc_Enable | DMA_PeripheralDataSize_Word | DMA_MemoryDataSize_Word |
		  DMA_Mode_Normal | DMA_Priority_Medium | DMA_MemoryBurst_INC4 | DMA_PeripheralBurst_INC4 ,
		  DMA_FIFOMode_Enable | DMA_FIFOThreshold_Full, 0, SDIO_FIFO_ADDRESS,  buf_dst );

  dma_it_config(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
  dma_flow_controller_config(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);
  /* DMA2 Stream3 or Stream6 enable */
  dma_cmd(SD_SDIO_DMA_STREAM, ENABLE);
}

/*--------------------------------------------------------------------*/
void __attribute__((__interrupt__)) _SD_SDIO_isr_dma_vector( void )
{
	sd_process_dma_irq();
}
/*--------------------------------------------------------------------*/
void __attribute__((__interrupt__)) sdio_isr_vector( void )
{
	sd_process_irq_src();
}

#else
#error F1 not implemented yet
#endif /* F2 and F4 */

#endif /* SDDRV_DMA_MODE */
/*--------------------------------------------------------------------*/
/**
  * @brief  Gets the cuurent sd card data transfer status.
  * @param  None
  * @retval SDTransferState: Data Transfer state.
  *   This value can be:
  *        - SD_TRANSFER_OK: No data transfer is acting
  *        - SD_TRANSFER_BUSY: Data transfer is acting
  */
static SDTransferState sd_get_status(void)
{
  sdcard_state cardstate = sd_get_state();
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
/*--------------------------------------------------------------------*/
/**
  * @brief  Returns the current card's state.
  * @param  None
  * @retval  sdcard_state: SD Card Error or SD Card Current State.
  */
static sdcard_state sd_get_state(void)
{
  uint32_t resp1 = 0;

  if(sd_detect()== SD_PRESENT)
  {
	if (sd_send_status(&resp1) != SD_OK)
    {
      return SD_CARD_ERROR;
    }
    else
    {
      return ( sdcard_state)((resp1 >> 9) & 0x0F);
    }
  }
  else
  {
	  return SD_CARD_ERROR;
  }
}
/*--------------------------------------------------------------------*/
/**
 * @brief  Detect if SD card is correctly plugged in the memory slot.
 * @param  None
 * @retval Return if SD is detected or not
 */
static uint8_t sd_detect(void)
{
  uint8_t status = SD_PRESENT;

  /*!< Check GPIO to detect SD */
  if ( gpio_get(SDDRV_DETECT_GPIO_PORT, SDDRV_DETECT_PIN) )
  {
    status = SD_NOT_PRESENT;
  }
  return status;
}
/*--------------------------------------------------------------------*/
/**
  * @brief  Enquires cards about their operating voltage and configures
  *   clock controls.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_power_on(void)
{
  sdcard_err errorstatus = SD_OK;
  uint32_t response = 0, count = 0, validvoltage = 0;
  uint32_t SDType = SD_STD_CAPACITY;

  /*!< Power ON Sequence -----------------------------------------------------*/
  /*!< Configure the SDIO peripheral */
  /*!< SDIO_CK = SDIOCLK / (SDDRV_INIT_CLK_DIV + 2) */
  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
  /*!< SDIO_CK for initialization should not exceed 400 KHz */
  sdio_init( SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
		  SDIO_BusWide_1b, SDIO_HardwareFlowControl_Disable, SDDRV_INIT_CLK_DIV );


  /*!< Set Power State to ON */
  sdio_set_power_state(SDIO_PowerState_ON);

  for(int w=0; w<16; w++ ) nop();

  /*!< Enable SDIO Clock */
  sdio_clock_cmd(ENABLE);


  /*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
  /*!< No CMD response required */
  sdio_send_command( 0x00, SD_CMD_GO_IDLE_STATE, SDIO_Response_No, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_error();

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
  errorstatus = cmd_resp7_error();
  if (errorstatus == SD_OK)
  {
    card_type = SDIO_STD_CAPACITY_SD_CARD_V2_0; /*!< SD Card 2.0 */
    SDType = SD_HIGH_CAPACITY;
  }
  else
  {
    /*!< CMD55 */
    sdio_send_command( 0x00, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = cmd_resp1_error(SD_CMD_APP_CMD);
  }
  /*!< CMD55 */
  sdio_send_command( 0x00, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_APP_CMD);
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
      errorstatus = cmd_resp1_error(SD_CMD_APP_CMD);
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      sdio_send_command( SD_VOLTAGE_WINDOW_SD | SDType, SD_CMD_SD_APP_OP_COND,
    		  SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );

      errorstatus = cmd_resp3_error();
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
      card_type = SDIO_HIGH_CAPACITY_SD_CARD;
    }

  }/*!< else MMC Card */

  return(errorstatus);
}
/*--------------------------------------------------------------------*/
/**
  * @brief  Turns the SDIO output signals off.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_power_off(void)
{
  /*!< Set Power State to OFF */
  sdio_set_power_state(SDIO_PowerState_OFF);
  return SD_OK;
}
/*--------------------------------------------------------------------*/
/**
  * @brief  Intialises all cards or single card as the case may be Card(s) come
  *         into standby state.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_initialize_cards(void)
{
  sdcard_err errorstatus = SD_OK;
  uint16_t rca = 0x01;
  if (sdio_get_power_state() == SDIO_PowerState_OFF)
  {
    errorstatus = SD_REQUEST_NOT_APPLICABLE;
    return(errorstatus);
  }
  if (SDIO_SECURE_DIGITAL_IO_CARD != card_type)
  {
    /*!< Send CMD2 ALL_SEND_CID */
    sdio_send_command( 0x00, SD_CMD_ALL_SEND_CID, SDIO_Response_Long, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = cmd_resp2_error();
    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
    cid_tab[0] = sdio_get_response(SDIO_RESP1);
    cid_tab[1] = sdio_get_response(SDIO_RESP2);
    cid_tab[2] = sdio_get_response(SDIO_RESP3);
    cid_tab[3] = sdio_get_response(SDIO_RESP4);
  }
  if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == card_type) ||  (SDIO_STD_CAPACITY_SD_CARD_V2_0 == card_type) ||  (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == card_type)
      ||  (SDIO_HIGH_CAPACITY_SD_CARD == card_type))
  {
    /*!< Send CMD3 SET_REL_ADDR with argument 0 */
    /*!< SD Card publishes its RCA. */
    sdio_send_command( 0x00, SD_CMD_SET_REL_ADDR, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = cmd_resp6_error(SD_CMD_SET_REL_ADDR, &rca);

    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
  }
  if (SDIO_SECURE_DIGITAL_IO_CARD != card_type)
  {
    RCA = rca;
    /*!< Send CMD9 SEND_CSD with argument as card's RCA */
    sdio_send_command(rca << 16, SD_CMD_SEND_CSD, SDIO_Response_Long, SDIO_Wait_No, SDIO_CPSM_Enable );
    errorstatus = cmd_resp2_error();
    if (SD_OK != errorstatus)
    {
      return(errorstatus);
    }
    csd_tab[0] = sdio_get_response(SDIO_RESP1);
    csd_tab[1] = sdio_get_response(SDIO_RESP2);
    csd_tab[2] = sdio_get_response(SDIO_RESP3);
    csd_tab[3] = sdio_get_response(SDIO_RESP4);
  }
  errorstatus = SD_OK; /*!< All cards get intialized */
  return(errorstatus);
}
/*--------------------------------------------------------------------*/
/**
 * @brief  Returns information about specific card.
  * @param  cardinfo: pointer to a sdcard_info structure that contains all SD card
  *         information.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_get_card_info(void *cardinfo, scard_info_field req )
{

  sdcard_err errorstatus = SD_OK;
  uint8_t tmp = 0;
  if( req == sdcard_info_f_cid )
  {
	   sd_cid *cid = (sd_cid*)cardinfo;
	  /*!< Byte 0 */
	   tmp = (uint8_t)((cid_tab[0] & 0xFF000000) >> 24);
	   cid->ManufacturerID = tmp;

	   /*!< Byte 1 */
	   tmp = (uint8_t)((cid_tab[0] & 0x00FF0000) >> 16);
	   cid->OEM_AppliID = tmp << 8;

	   /*!< Byte 2 */
	   tmp = (uint8_t)((cid_tab[0] & 0x000000FF00) >> 8);
	   cid->OEM_AppliID |= tmp;

	   /*!< Byte 3 */
	   tmp = (uint8_t)(cid_tab[0] & 0x000000FF);
	   cid->ProdName1 = tmp << 24;

	   /*!< Byte 4 */
	   tmp = (uint8_t)((cid_tab[1] & 0xFF000000) >> 24);
	   cid->ProdName1 |= tmp << 16;

	   /*!< Byte 5 */
	   tmp = (uint8_t)((cid_tab[1] & 0x00FF0000) >> 16);
	   cid->ProdName1 |= tmp << 8;

	   /*!< Byte 6 */
	   tmp = (uint8_t)((cid_tab[1] & 0x0000FF00) >> 8);
	   cid->ProdName1 |= tmp;

	   /*!< Byte 7 */
	   tmp = (uint8_t)(cid_tab[1] & 0x000000FF);
	   cid->ProdName2 = tmp;

	   /*!< Byte 8 */
	   tmp = (uint8_t)((cid_tab[2] & 0xFF000000) >> 24);
	   cid->ProdRev = tmp;

	   /*!< Byte 9 */
	   tmp = (uint8_t)((cid_tab[2] & 0x00FF0000) >> 16);
	   cid->ProdSN = tmp << 24;

	   /*!< Byte 10 */
	   tmp = (uint8_t)((cid_tab[2] & 0x0000FF00) >> 8);
	   cid->ProdSN |= tmp << 16;

	   /*!< Byte 11 */
	   tmp = (uint8_t)(cid_tab[2] & 0x000000FF);
	   cid->ProdSN |= tmp << 8;

	   /*!< Byte 12 */
	   tmp = (uint8_t)((cid_tab[3] & 0xFF000000) >> 24);
	   cid->ProdSN |= tmp;

	   /*!< Byte 13 */
	   tmp = (uint8_t)((cid_tab[3] & 0x00FF0000) >> 16);
	   cid->Reserved1 |= (tmp & 0xF0) >> 4;
	   cid->ManufactDate = (tmp & 0x0F) << 8;

	   /*!< Byte 14 */
	   tmp = (uint8_t)((cid_tab[3] & 0x0000FF00) >> 8);
	   cid->ManufactDate |= tmp;

	   /*!< Byte 15 */
	   tmp = (uint8_t)(cid_tab[3] & 0x000000FF);
	   cid->CID_CRC = (tmp & 0xFE) >> 1;
	   cid->Reserved2 = 1;
  }
  else if( req == sdcard_info_f_csd )
  {
	  	sd_csd *csd = (sd_csd*)cardinfo;
	    /*!< Byte 0 */
	    tmp = (uint8_t)((csd_tab[0] & 0xFF000000) >> 24);
	    csd->CSDStruct = (tmp & 0xC0) >> 6;
	    csd->SysSpecVersion = (tmp & 0x3C) >> 2;
	    csd->Reserved1 = tmp & 0x03;

	    /*!< Byte 1 */
	    tmp = (uint8_t)((csd_tab[0] & 0x00FF0000) >> 16);
	    csd->TAAC = tmp;

	    /*!< Byte 2 */
	    tmp = (uint8_t)((csd_tab[0] & 0x0000FF00) >> 8);
	    csd->NSAC = tmp;

	    /*!< Byte 3 */
	    tmp = (uint8_t)(csd_tab[0] & 0x000000FF);
	    csd->MaxBusClkFrec = tmp;

	    /*!< Byte 4 */
	    tmp = (uint8_t)((csd_tab[1] & 0xFF000000) >> 24);
	    csd->CardComdClasses = tmp << 4;

	    /*!< Byte 5 */
	    tmp = (uint8_t)((csd_tab[1] & 0x00FF0000) >> 16);
	    csd->CardComdClasses |= (tmp & 0xF0) >> 4;
	    csd->RdBlockLen = tmp & 0x0F;

	    /*!< Byte 6 */
	    tmp = (uint8_t)((csd_tab[1] & 0x0000FF00) >> 8);
	    csd->PartBlockRead = (tmp & 0x80) >> 7;
	    csd->WrBlockMisalign = (tmp & 0x40) >> 6;
	    csd->RdBlockMisalign = (tmp & 0x20) >> 5;
	    csd->DSRImpl = (tmp & 0x10) >> 4;
	    csd->Reserved2 = 0; /*!< Reserved */

	    if ((card_type == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (card_type == SDIO_STD_CAPACITY_SD_CARD_V2_0))
	    {
	      csd->DeviceSize = (tmp & 0x03) << 10;

	      /*!< Byte 7 */
	      tmp = (uint8_t)(csd_tab[1] & 0x000000FF);
	      csd->DeviceSize |= (tmp) << 2;

	      /*!< Byte 8 */
	      tmp = (uint8_t)((csd_tab[2] & 0xFF000000) >> 24);
	      csd->DeviceSize |= (tmp & 0xC0) >> 6;

	      csd->MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
	      csd->MaxRdCurrentVDDMax = (tmp & 0x07);

	      /*!< Byte 9 */
	      tmp = (uint8_t)((csd_tab[2] & 0x00FF0000) >> 16);
	      csd->MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
	      csd->MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
	      csd->DeviceSizeMul = (tmp & 0x03) << 1;
	      /*!< Byte 10 */
	      tmp = (uint8_t)((csd_tab[2] & 0x0000FF00) >> 8);
	      csd->DeviceSizeMul |= (tmp & 0x80) >> 7;

	    }
	    else if (card_type == SDIO_HIGH_CAPACITY_SD_CARD)
	    {
	      /*!< Byte 7 */
	      tmp = (uint8_t)(csd_tab[1] & 0x000000FF);
	      csd->DeviceSize = (tmp & 0x3F) << 16;

	      /*!< Byte 8 */
	      tmp = (uint8_t)((csd_tab[2] & 0xFF000000) >> 24);

	      csd->DeviceSize |= (tmp << 8);

	      /*!< Byte 9 */
	      tmp = (uint8_t)((csd_tab[2] & 0x00FF0000) >> 16);

	      csd->DeviceSize |= (tmp);

	      /*!< Byte 10 */
	      tmp = (uint8_t)((csd_tab[2] & 0x0000FF00) >> 8);

	    }
	    csd->EraseGrSize = (tmp & 0x40) >> 6;
	    csd->EraseGrMul = (tmp & 0x3F) << 1;

	    /*!< Byte 11 */
	    tmp = (uint8_t)(csd_tab[2] & 0x000000FF);
	    csd->EraseGrMul |= (tmp & 0x80) >> 7;
	    csd->WrProtectGrSize = (tmp & 0x7F);

	    /*!< Byte 12 */
	    tmp = (uint8_t)((csd_tab[3] & 0xFF000000) >> 24);
	    csd->WrProtectGrEnable = (tmp & 0x80) >> 7;
	    csd->ManDeflECC = (tmp & 0x60) >> 5;
	    csd->WrSpeedFact = (tmp & 0x1C) >> 2;
	    csd->MaxWrBlockLen = (tmp & 0x03) << 2;

	    /*!< Byte 13 */
	    tmp = (uint8_t)((csd_tab[3] & 0x00FF0000) >> 16);
	    csd->MaxWrBlockLen |= (tmp & 0xC0) >> 6;
	    csd->WriteBlockPaPartial = (tmp & 0x20) >> 5;
	    csd->Reserved3 = 0;
	    csd->ContentProtectAppli = (tmp & 0x01);

	    /*!< Byte 14 */
	    tmp = (uint8_t)((csd_tab[3] & 0x0000FF00) >> 8);
	    csd->FileFormatGrouop = (tmp & 0x80) >> 7;
	    csd->CopyFlag = (tmp & 0x40) >> 6;
	    csd->PermWrProtect = (tmp & 0x20) >> 5;
	    csd->TempWrProtect = (tmp & 0x10) >> 4;
	    csd->FileFormat = (tmp & 0x0C) >> 2;
	    csd->ECC = (tmp & 0x03);

	    /*!< Byte 15 */
	    tmp = (uint8_t)(csd_tab[3] & 0x000000FF);
	    csd->CSD_CRC = (tmp & 0xFE) >> 1;
	    csd->Reserved4 = 1;
  }
  else if( req == sdcard_info_f_info )
  {
	  sdcard_info *info = (sdcard_info*)cardinfo;
	  info->CardType = (uint8_t)card_type;
	  info->RCA = (uint16_t)RCA;
	  if ((card_type == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (card_type == SDIO_STD_CAPACITY_SD_CARD_V2_0))
	  {
		  /*!< Byte 6 */
		  tmp = (uint8_t)((csd_tab[1] & 0x0000FF00) >> 8);
		  uint32_t csd_ds = (tmp & 0x03) << 10;

		  /*!< Byte 7 */
		  tmp = (uint8_t)(csd_tab[1] & 0x000000FF);
		  csd_ds |= (tmp) << 2;

		  /*!< Byte 8 */
		  tmp = (uint8_t)((csd_tab[2] & 0xFF000000) >> 24);
		  csd_ds  |= (tmp & 0xC0) >> 6;

		  /*!< Byte 9 */
		  tmp = (uint8_t)((csd_tab[2] & 0x00FF0000) >> 16);
		  uint8_t csd_DeviceSizeMul = (tmp & 0x03) << 1;
		  /*!< Byte 10 */
		  tmp = (uint8_t)((csd_tab[2] & 0x0000FF00) >> 8);
		  csd_DeviceSizeMul |= (tmp & 0x80) >> 7;
		   /*!< Byte 5 */
		  tmp = (uint8_t)((csd_tab[1] & 0x00FF0000) >> 16);
		  uint8_t csd_RdBlockLen = tmp & 0x0F;
		  info->CardCapacity = (csd_ds + 1) ;
		  info->CardCapacity *= (1 << (csd_DeviceSizeMul + 2));
		  info->CardBlockSize = 1 << (csd_RdBlockLen);
		  info->CardCapacity *= info->CardBlockSize;
	  }
	  else if (card_type == SDIO_HIGH_CAPACITY_SD_CARD)
	  {
		  /*!< Byte 7 */
		  tmp = (uint8_t)(csd_tab[1] & 0x000000FF);
		  uint32_t csd_ds = (tmp & 0x3F) << 16;
		  /*!< Byte 8 */
		  tmp = (uint8_t)((csd_tab[2] & 0xFF000000) >> 24);
		  csd_ds |= (tmp << 8);
		  /*!< Byte 9 */
		  tmp = (uint8_t)((csd_tab[2] & 0x00FF0000) >> 16);
		  csd_ds |= (tmp);
		  info->CardCapacity = (csd_ds + 1) * 512 * 1024;
		  info->CardBlockSize = 512;
	  }
  }
  else
  {
	  errorstatus = SD_ERROR;
  }
  return errorstatus;
}
/*--------------------------------------------------------------------*/
#ifdef SDDRV_ADVANCED_SD_API
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
static sdcard_err sd_get_card_status(sdcard_status *cardstatus)
{
	  uint8_t g_sd_tab[16];
	  sdcard_err errorstatus = SD_OK;
	  uint8_t tmp = 0;

	  errorstatus = sd_send_sd_status((uint32_t *)g_sd_tab);

	  if (errorstatus  != SD_OK)
	  {
		return(errorstatus);
	  }

	  /*!< Byte 0 */
	  tmp = (uint8_t)((g_sd_tab[0] & 0xC0) >> 6);
	  cardstatus->DAT_BUS_WIDTH = tmp;

	  /*!< Byte 0 */
	  tmp = (uint8_t)((g_sd_tab[0] & 0x20) >> 5);
	  cardstatus->SECURED_MODE = tmp;

	  /*!< Byte 2 */
	  tmp = (uint8_t)((g_sd_tab[2] & 0xFF));
	  cardstatus->SD_CARD_TYPE = tmp << 8;

	  /*!< Byte 3 */
	  tmp = (uint8_t)((g_sd_tab[3] & 0xFF));
	  cardstatus->SD_CARD_TYPE |= tmp;

	  /*!< Byte 4 */
	  tmp = (uint8_t)(g_sd_tab[4] & 0xFF);
	  cardstatus->SIZE_OF_PROTECTED_AREA = tmp << 24;

	  /*!< Byte 5 */
	  tmp = (uint8_t)(g_sd_tab[5] & 0xFF);
	  cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 16;

	  /*!< Byte 6 */
	  tmp = (uint8_t)(g_sd_tab[6] & 0xFF);
	  cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 8;

	  /*!< Byte 7 */
	  tmp = (uint8_t)(g_sd_tab[7] & 0xFF);
	  cardstatus->SIZE_OF_PROTECTED_AREA |= tmp;

	  /*!< Byte 8 */
	  tmp = (uint8_t)((g_sd_tab[8] & 0xFF));
	  cardstatus->SPEED_CLASS = tmp;

	  /*!< Byte 9 */
	  tmp = (uint8_t)((g_sd_tab[9] & 0xFF));
	  cardstatus->PERFORMANCE_MOVE = tmp;

	  /*!< Byte 10 */
	  tmp = (uint8_t)((g_sd_tab[10] & 0xF0) >> 4);
	  cardstatus->AU_SIZE = tmp;

	  /*!< Byte 11 */
	  tmp = (uint8_t)(g_sd_tab[11] & 0xFF);
	  cardstatus->ERASE_SIZE = tmp << 8;

	  /*!< Byte 12 */
	  tmp = (uint8_t)(g_sd_tab[12] & 0xFF);
	  cardstatus->ERASE_SIZE |= tmp;

	  /*!< Byte 13 */
	  tmp = (uint8_t)((g_sd_tab[13] & 0xFC) >> 2);
	  cardstatus->ERASE_TIMEOUT = tmp;

	  /*!< Byte 13 */
	  tmp = (uint8_t)((g_sd_tab[13] & 0x3));
	  cardstatus->ERASE_OFFSET = tmp;

	  return(errorstatus);
}
#endif
/*--------------------------------------------------------------------*/
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
static sdcard_err sd_enable_wide_bus_operation(uint32_t WideMode)
{
  sdcard_err errorstatus = SD_OK;

  /*!< MMC Card doesn't support this feature */
  if (SDIO_MULTIMEDIA_CARD == card_type)
  {
    errorstatus = SD_UNSUPPORTED_FEATURE;
    return(errorstatus);
  }
  else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == card_type) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == card_type) || (SDIO_HIGH_CAPACITY_SD_CARD == card_type))
  {
    if (SDIO_BusWide_8b == WideMode)
    {
      errorstatus = SD_UNSUPPORTED_FEATURE;
      return(errorstatus);
    }
    else if (SDIO_BusWide_4b == WideMode)
    {
      errorstatus = sd_en_wide_bus(ENABLE);

      if (SD_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
        sdio_init( SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
        		SDIO_BusWide_4b, SDIO_HardwareFlowControl_Disable, SDDRV_TRANSFER_CLK_DIV );
      }
    }
    else
    {
      errorstatus = sd_en_wide_bus(DISABLE);

      if (SD_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
        sdio_init( SDIO_ClockEdge_Rising, SDIO_ClockBypass_Disable, SDIO_ClockPowerSave_Disable,
        		SDIO_BusWide_1b, SDIO_HardwareFlowControl_Disable, SDDRV_TRANSFER_CLK_DIV );
      }
    }
  }
  return(errorstatus);
}
/*--------------------------------------------------------------------*/
/**
  * @brief  Selects od Deselects the corresponding card.
  * @param  addr: Address of the Card to be selected.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_select_deselect(uint32_t addr)
{
  /*!< Send CMD7 SDIO_SEL_DESEL_CARD */
  sdio_send_command( addr, SD_CMD_SEL_DESEL_CARD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  sdcard_err errorstatus = cmd_resp1_error(SD_CMD_SEL_DESEL_CARD);
  return(errorstatus);
}
/*--------------------------------------------------------------------*/
/**
  * @brief  Allows to read one block from a specified address in a card. The Data
  *         transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - sd_get_status(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  readbuff: pointer to the buffer that will contain the received data
  * @param  ReadAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval sdcard_err: SD Card Error code.
  */
#if SDDRV_TRANSFER_MODE==SDDRV_POLLING_MODE
static sdcard_err sd_read_block(uint8_t *readbuff, uint32_t readaddr,  uint16_t blocksize)
{
  sdcard_err errorstatus = SD_OK;
#if (SDDRV_TRANSFER_MODE==SDDRV_POLLING_MODE)
  uint32_t count = 0, *tempbuff = (uint32_t *)readbuff;
#endif
  transfer_error = SD_OK;
  transfer_end = 0;
  stop_condition = 0;

  SDIO->DCTRL = 0x0;


  if (card_type == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    blocksize = 512;
    readaddr /= 512;
  }
  /* Set Block Size for Card */
  sdio_send_command( blocksize, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }

  sdio_data_config( SDDRV_HW_DATATIMEOUT, blocksize, 9 << 4, SDIO_TransferDir_ToSDIO,
		  SDIO_TransferMode_Block, SDIO_DPSM_Enable );

  /*!< Send CMD17 READ_SINGLE_BLOCK */
  sdio_send_command( readaddr, SD_CMD_READ_SINGLE_BLOCK, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable);
  errorstatus = cmd_resp1_error(SD_CMD_READ_SINGLE_BLOCK);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
#if (SDDRV_TRANSFER_MODE==SDDRV_POLLING_MODE)
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
  count = SDDRV_HW_DATATIMEOUT;
  while ((sdio_get_flag_status(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
  {
    *tempbuff = sdio_read_data();
    tempbuff++;
    count--;
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);

#elif (SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE)
    sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    sdio_dma_cmd(ENABLE);
    sd_lowlevel_dma_rx_config((uint32_t *)readbuff, blocksize);
#else
#error Unknown mode
#endif

  return(errorstatus);
}
#endif
/*--------------------------------------------------------------------*/
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
/**
  * @brief  Allows to read blocks from a specified address  in a card.  The Data
  *         transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - sd_get_status(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  readbuff: pointer to the buffer that will contain the received data.
  * @param  ReadAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_read_multi_blocks(uint8_t *readbuff, uint32_t readaddress, uint16_t block_size, uint32_t numb_blocks)
{
  sdcard_err errorstatus = SD_OK;
  transfer_error = SD_OK;
  transfer_end = 0;
  stop_condition = 1;
  SDIO->DCTRL = 0x0;
  if (card_type == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    block_size = 512;
    readaddress /= 512;
  }
  /*!< Set Block Size for Card */
  sdio_send_command(block_size, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
  sdio_data_config( SDDRV_HW_DATATIMEOUT, numb_blocks * block_size, 9 << 4,
	 SDIO_TransferDir_ToSDIO,  SDIO_TransferMode_Block, SDIO_DPSM_Enable );

  /*!< Send CMD18 READ_MULT_BLOCK with argument data address */
  sdio_send_command( readaddress, SD_CMD_READ_MULT_BLOCK, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_READ_MULT_BLOCK);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  sdio_dma_cmd(ENABLE);
  sd_lowlevel_dma_rx_config((uint32_t *)readbuff, (numb_blocks * block_size));
  return(errorstatus);
}
#endif
/*--------------------------------------------------------------------*/
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
/**
  * @brief  This function waits until the SDIO DMA data transfer is finished.
  *         This function should be called after SDIO_ReadMultiBlocks() function
  *         to insure that all data sent by the card are already transferred by
  *         the DMA controller.
  * @param  None.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_wait_read_operation(void)
{
  int errorstatus = SD_OK;
  uint32_t timeout;


  if( (errorstatus = isix_sem_wait( tcomplete_sem, SDDRV_SEM_DATATIMEOUT )) != ISIX_EOK )
	  return errorstatus;

  /*
  while ((dma_end_of_transfer == 0x00) && (transfer_end == 0) && (transfer_error == SD_OK) && (timeout > 0))
  {
    timeout--;
  }
*/
  dma_end_of_transfer = 0x00;

  timeout = SDDRV_HW_DATATIMEOUT;
  while(((SDIO->STA & SDIO_FLAG_RXACT)) && (timeout > 0))
  {
    timeout--;
    isix_yield();
  }

  if (stop_condition == 1)
  {
    errorstatus = sd_stop_transfer();
    stop_condition = 0;
  }

  if ((timeout == 0) && (errorstatus == SD_OK))
  {
    errorstatus = SD_DATA_TIMEOUT;
  }

  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  if (transfer_error != SD_OK)
  {
    return transfer_error;
  }
  else
  {
    return errorstatus ;
  }
}
#endif
/*--------------------------------------------------------------------*/
#if (SDDRV_TRANSFER_MODE==SDDRV_POLLING_MODE)
/**
  * @brief  Allows to write one block starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - sd_get_status(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  WriteAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_write_block(const uint8_t *writebuff, uint32_t writeaddr, uint16_t blocksize)
{
  sdcard_err errorstatus = SD_OK;

#if (SDDRV_TRANSFER_MODE==SDDRV_POLLING_MODE)
  uint32_t bytestransferred = 0, count = 0, restwords = 0;
  uint32_t *tempbuff = (uint32_t *)writebuff;
#endif
  transfer_error = SD_OK;
  transfer_end = 0;
  stop_condition = 0;
  SDIO->DCTRL = 0x0;
  if (card_type == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    blocksize = 512;
    writeaddr /= 512;
  }
  /* Set Block Size for Card */
  sdio_send_command( blocksize, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
  /*!< Send CMD24 WRITE_SINGLE_BLOCK */
  sdio_send_command( writeaddr, SD_CMD_WRITE_SINGLE_BLOCK, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_WRITE_SINGLE_BLOCK);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  sdio_data_config( SDDRV_HW_DATATIMEOUT, blocksize, 9 << 4, SDIO_TransferDir_ToCard,
		  SDIO_TransferMode_Block, SDIO_DPSM_Enable );
  /*!< In case of single data block transfer no need of stop command at all */
#if (SDDRV_TRANSFER_MODE==SDDRV_POLLING_MODE)
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
#elif SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
  sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  sd_lowlevel_dma_tx_config((uint32_t*)writebuff, blocksize);
  sdio_dma_cmd(ENABLE);
#endif

  return(errorstatus);
}
#endif
/*--------------------------------------------------------------------*/
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
/**
  * @brief  Allows to write blocks starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode only.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - sd_get_status(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  WriteAddr: Address from where data are to be read.
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_write_multi_blocks(const uint8_t *writebuff, uint32_t writeaddr, uint16_t blocksize, uint32_t numb_blocks)
{
  sdcard_err errorstatus = SD_OK;

  transfer_error = SD_OK;
  transfer_end = 0;
  stop_condition = 1;
  SDIO->DCTRL = 0x0;
  if (card_type == SDIO_HIGH_CAPACITY_SD_CARD)
  {
    blocksize = 512;
    writeaddr /= 512;
  }
  /* Set Block Size for Card */
  sdio_send_command( blocksize, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_SET_BLOCKLEN);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
  /*!< To improve performance */
  sdio_send_command( RCA << 16, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_APP_CMD);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  /*!< To improve performance */
  sdio_send_command( numb_blocks, SD_CMD_SET_BLOCK_COUNT, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_SET_BLOCK_COUNT);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  /*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
  sdio_send_command( writeaddr, SD_CMD_WRITE_MULT_BLOCK, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_WRITE_MULT_BLOCK);
  if (SD_OK != errorstatus)
  {
    return(errorstatus);
  }
  sdio_data_config( SDDRV_HW_DATATIMEOUT, numb_blocks * blocksize, 9 << 4,
		  SDIO_TransferDir_ToCard, SDIO_TransferMode_Block, SDIO_DPSM_Enable );
  sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  sdio_dma_cmd(ENABLE);
  sd_lowlevel_dma_tx_config((uint32_t *)writebuff, (numb_blocks * blocksize));
  return(errorstatus);
}
#endif

#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
/*--------------------------------------------------------------------*/
/**
  * @brief  This function waits until the SDIO DMA data transfer is finished.
  *         This function should be called after SDIO_WriteBlock() and
  *         SDIO_WriteMultiBlocks() function to insure that all data sent by the
  *         card are already transferred by the DMA controller.
  * @param  None.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_wait_write_operation(void)
{
  int errorstatus = SD_OK;
  uint32_t timeout;

  /*
  timeout = SDDRV_HW_DATATIMEOUT;
  while ((dma_end_of_transfer == 0x00) && (transfer_end == 0) && (transfer_error == SD_OK) && (timeout > 0))
  {
    timeout--;
  }
  */
  if ( (errorstatus = isix_sem_wait(tcomplete_sem,SDDRV_SEM_DATATIMEOUT)) != ISIX_EOK )
  dma_end_of_transfer = 0x00;
  timeout = SDDRV_HW_DATATIMEOUT;
  while(((SDIO->STA & SDIO_FLAG_TXACT)) && (timeout > 0))
  {
    timeout--;
    isix_yield();
  }
  if (stop_condition == 1)
  {
    errorstatus = sd_stop_transfer();
    stop_condition = 0;
  }
  if ((timeout == 0) && (errorstatus == SD_OK))
  {
    errorstatus = SD_DATA_TIMEOUT;
  }
  /*!< Clear all the static flags */
  sdio_clear_flag(SDIO_STATIC_FLAGS);
  if (transfer_error != SD_OK)
  {
    return(transfer_error);
  }
  else
  {
    return(errorstatus);
  }
}
#endif
/*--------------------------------------------------------------------*/
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
/**
  * @brief  Aborts an ongoing data transfer.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_stop_transfer(void)
{
  /*!< Send CMD12 STOP_TRANSMISSION  */
  sdio_send_command( 0x0, SD_CMD_STOP_TRANSMISSION, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  sdcard_err errorstatus = cmd_resp1_error(SD_CMD_STOP_TRANSMISSION);
  return(errorstatus);
}
#endif
/*--------------------------------------------------------------------*/
#ifdef SDDRV_ADVANCED_SD_API
/**
  * @brief  Allows to erase memory area specified for the given card.
  * @param  startaddr: the start address.
  * @param  endaddr: the end address.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_erase(uint32_t startaddr, uint32_t endaddr)
{
	  sdcard_err errorstatus = SD_OK;
	  uint32_t delay = 0;
	  __IO uint32_t maxdelay = 0;
	  uint8_t cardstate = 0;
	  /*!< Check if the card coomnd class supports erase command */
	  if (((csd_tab[1] >> 20) & SD_CCCC_ERASE) == 0)
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
	  if (card_type == SDIO_HIGH_CAPACITY_SD_CARD)
	  {
		startaddr /= 512;
		endaddr /= 512;
	  }
	  /*!< According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
	  if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == card_type) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == card_type) || (SDIO_HIGH_CAPACITY_SD_CARD == card_type))
	  {
		/*!< Send CMD32 SD_ERASE_GRP_START with argument as addr  */
		sdio_send_command( startaddr, SD_CMD_SD_ERASE_GRP_START, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
		errorstatus = cmd_resp1_error(SD_CMD_SD_ERASE_GRP_START);
		if (errorstatus != SD_OK)
		{
		  return(errorstatus);
		}
		/*!< Send CMD33 SD_ERASE_GRP_END with argument as addr  */
		sdio_send_command( endaddr, SD_CMD_SD_ERASE_GRP_END, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
		errorstatus = cmd_resp1_error(SD_CMD_SD_ERASE_GRP_END);
		if (errorstatus != SD_OK)
		{
		  return(errorstatus);
		}
	  }
	  /*!< Send CMD38 ERASE */
	  sdio_send_command( 0, SD_CMD_ERASE, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
	  errorstatus = cmd_resp1_error(SD_CMD_ERASE);
	  if (errorstatus != SD_OK)
	  {
		return(errorstatus);
	  }
	  for (delay = 0; delay < maxdelay; delay++) nop();
	  /*!< Wait till the card is in programming state */
	  errorstatus = is_card_programming(&cardstate);
	  delay = SDDRV_HW_DATATIMEOUT;
	  while ((delay > 0) && (errorstatus == SD_OK) && ((SD_CARD_PROGRAMMING == cardstate) || (SD_CARD_RECEIVING == cardstate)))
	  {
		errorstatus = is_card_programming(&cardstate);
		delay--;
	  }

	  return(errorstatus);
}
#endif
/*--------------------------------------------------------------------*/
/**
  * @brief  Returns the current card's status.
  * @param  pcardstatus: pointer to the buffer that will contain the SD card
  *         status (Card Status register).
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_send_status(uint32_t *pcardstatus)
{
  sdcard_err errorstatus = SD_OK;
  if (pcardstatus == NULL)
  {
    errorstatus = SD_INVALID_PARAMETER;
    return(errorstatus);
  }
  sdio_send_command( RCA << 16, SD_CMD_SEND_STATUS, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_SEND_STATUS);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  *pcardstatus = sdio_get_response(SDIO_RESP1);
  return(errorstatus);
}
/*--------------------------------------------------------------------*/
#ifdef SDDRV_ADVANCED_SD_API
/**
  * @brief  Returns the current SD card's status.
  * @param  psdstatus: pointer to the buffer that will contain the SD card status
  *         (SD Status register).
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_send_sd_status(uint32_t *psdstatus)
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
  errorstatus = cmd_resp1_error(SD_CMD_SET_BLOCKLEN);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  /*!< CMD55 */
  sdio_send_command( RCA << 16, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_APP_CMD);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  sdio_data_config( SDDRV_HW_DATATIMEOUT, 64, SDIO_DataBlockSize_64b,
		  SDIO_TransferDir_ToSDIO, SDIO_TransferMode_Block, SDIO_DPSM_Enable );

  /*!< Send ACMD13 SD_APP_STAUS  with argument as card's RCA.*/
  sdio_send_command( 0, SD_CMD_SD_APP_STAUS, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_SD_APP_STAUS);
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
  count = SDDRV_HW_DATATIMEOUT;
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
#endif
/*--------------------------------------------------------------------*/
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
/**
  * @brief  Allows to process all the interrupts that are high.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_process_irq_src(void)
{
  if (sdio_get_it_status(SDIO_IT_DATAEND) != RESET)
  {
    transfer_error = SD_OK;
    sdio_clear_it_pending_bit(SDIO_IT_DATAEND);
    transfer_end = 1;
  }
  else if (sdio_get_it_status(SDIO_IT_DCRCFAIL) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_DCRCFAIL);
    transfer_error = SD_DATA_CRC_FAIL;
  }
  else if (sdio_get_it_status(SDIO_IT_DTIMEOUT) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_DTIMEOUT);
    transfer_error = SD_DATA_TIMEOUT;
  }
  else if (sdio_get_it_status(SDIO_IT_RXOVERR) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_RXOVERR);
    transfer_error = SD_RX_OVERRUN;
  }
  else if (sdio_get_it_status(SDIO_IT_TXUNDERR) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_TXUNDERR);
    transfer_error = SD_TX_UNDERRUN;
  }
  else if (sdio_get_it_status(SDIO_IT_STBITERR) != RESET)
  {
	sdio_clear_it_pending_bit(SDIO_IT_STBITERR);
    transfer_error = SD_START_BIT_ERR;
  }
  sdio_it_config(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
                SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
                SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
  isix_sem_signal_isr( tcomplete_sem );
  return(transfer_error);
}
#endif
/*--------------------------------------------------------------------*/
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
/**
  * @brief  This function waits until the SDIO DMA data transfer is finished.
  * @param  None.
  * @retval None.
  */
static void sd_process_dma_irq(void)
{
  if(SD_SDIO_DMA_SR & SD_SDIO_DMA_FLAG_TCIF)
  {
    dma_end_of_transfer = 0x01;
    dma_clear_flag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_TCIF|SD_SDIO_DMA_FLAG_FEIF);
    isix_sem_signal_isr( tcomplete_sem );
  }
}

#endif
/**
  * @brief  Checks for error conditions for CMD0.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err cmd_error(void)
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
/*--------------------------------------------------------------------*/
/**
  * @brief  Checks for error conditions for R7 response.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err cmd_resp7_error(void)
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
/*--------------------------------------------------------------------*/
/**
  * @brief  Checks for error conditions for R1 response.
  * @param  cmd: The sent command index.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err cmd_resp1_error(uint8_t cmd)
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
/*--------------------------------------------------------------------*/
/**
  * @brief  Checks for error conditions for R3 (OCR) response.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err cmd_resp3_error(void)
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

/*--------------------------------------------------------------------*/
/**
  * @brief  Checks for error conditions for R2 (CID or CSD) response.
  * @param  None
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err cmd_resp2_error(void)
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

/*--------------------------------------------------------------------*/
/**
  * @brief  Checks for error conditions for R6 (RCA) response.
  * @param  cmd: The sent command index.
  * @param  prca: pointer to the variable that will contain the SD card relative
  *         address RCA.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err cmd_resp6_error(uint8_t cmd, uint16_t *prca)
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

/*--------------------------------------------------------------------*/
/**
  * @brief  Enables or disables the SDIO wide bus mode.
  * @param  NewState: new state of the SDIO wide bus mode.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err sd_en_wide_bus(FunctionalState NewState)
{
  sdcard_err errorstatus = SD_OK;

  uint32_t scr[2] = {0, 0};

  if (sdio_get_response(SDIO_RESP1) & SD_CARD_LOCKED)
  {
    errorstatus = SD_LOCK_UNLOCK_FAILED;
    return(errorstatus);
  }

  /*!< Get SCR Register */
  errorstatus = find_scr(RCA, scr);

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
      errorstatus = cmd_resp1_error(SD_CMD_APP_CMD);
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }
      /*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
      sdio_send_command( 0x2, SD_CMD_APP_SD_SET_BUSWIDTH, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
      errorstatus = cmd_resp1_error(SD_CMD_APP_SD_SET_BUSWIDTH);
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
      errorstatus = cmd_resp1_error(SD_CMD_APP_CMD);
      if (errorstatus != SD_OK)
      {
        return(errorstatus);
      }

      sdio_send_command( 0x00, SD_CMD_APP_SD_SET_BUSWIDTH, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
      errorstatus = cmd_resp1_error(SD_CMD_APP_SD_SET_BUSWIDTH);
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

/*--------------------------------------------------------------------*/
#ifdef SDDRV_ADVANCED_SD_API
/**
  * @brief  Checks if the SD card is in programming state.
  * @param  pstatus: pointer to the variable that will contain the SD card state.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err is_card_programming(uint8_t *pstatus)
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
#endif
/*--------------------------------------------------------------------*/
/**
  * @brief  Find the SD card SCR register value.
  * @param  rca: selected card address.
  * @param  pscr: pointer to the buffer that will contain the SCR value.
  * @retval sdcard_err: SD Card Error code.
  */
static sdcard_err find_scr(uint16_t rca, uint32_t *pscr)
{
  uint32_t index = 0;
  sdcard_err errorstatus = SD_OK;
  uint32_t tempscr[2] = {0, 0};
  /*!< Set Block Size To 8 Bytes */
  /*!< Send CMD55 APP_CMD with argument as card's RCA */
  sdio_send_command( 8, SD_CMD_SET_BLOCKLEN, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_SET_BLOCKLEN);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  /*!< Send CMD55 APP_CMD with argument as card's RCA */
  sdio_send_command( rca << 16, SD_CMD_APP_CMD, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_APP_CMD);
  if (errorstatus != SD_OK)
  {
    return(errorstatus);
  }
  sdio_data_config( SDDRV_HW_DATATIMEOUT, 8, SDIO_DataBlockSize_8b, SDIO_TransferDir_ToSDIO,
		  SDIO_TransferMode_Block, SDIO_DPSM_Enable );

  /*!< Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
  sdio_send_command( 0x0, SD_CMD_SD_APP_SEND_SCR, SDIO_Response_Short, SDIO_Wait_No, SDIO_CPSM_Enable );
  errorstatus = cmd_resp1_error(SD_CMD_SD_APP_SEND_SCR);
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

/*--------------------------------------------------------------------*/
//ISIX driver public fat funcs
/*--------------------------------------------------------------------*/
//Initialize the SDIO card driver
int isix_sdio_card_driver_init(void)
{
	if( tlock_sem )
		 return SD_LIB_ALREADY_INITIALIZED;
	tlock_sem = isix_sem_create_limited( NULL, 1, 1 );
	if( !tlock_sem )
	{
		return ISIX_ENOMEM;
	}
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
	nvic_set_priority( SDIO_IRQn, SDDRV_IRQ_PRIO, SDDRV_IRQ_SUBPRIO );
	nvic_irq_enable( SDIO_IRQn, true );
	nvic_set_priority( SD_SDIO_DMA_IRQn , SDDRV_IRQ_PRIO, SDDRV_IRQ_SUBPRIO );
	nvic_irq_enable( SD_SDIO_DMA_IRQn , true );
	//Initialize the complete SEM
	tcomplete_sem = isix_sem_create_limited( NULL, 0, 1);
	if( !tcomplete_sem )
	{
		isix_sem_destroy( tlock_sem );
		return ISIX_ENOMEM;
	}
	//Initialize the lock sem
#endif
	sd_hw_init();
	return sd_card_init( );
}
/*--------------------------------------------------------------------*/
/* Destroy card driver and disable interrupt */
void isix_sdio_card_driver_destroy(void)
{
	nvic_irq_enable( SDIO_IRQn, false );
#if (SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE)
	nvic_irq_enable( SD_SDIO_DMA_IRQn, false );
#endif
	sd_power_off();
	sd_deinit();
	if( tlock_sem ) isix_sem_destroy( tlock_sem );
	if( tcomplete_sem ) isix_sem_destroy( tcomplete_sem );
	tlock_sem = tcomplete_sem = NULL;
}
/*--------------------------------------------------------------------*/
//SDIO card driver read
int isix_sdio_card_driver_read( void *buf, unsigned long LBA, size_t count )
{
	if( !tlock_sem )
		 return SD_LIB_NOT_INITIALIZED;
	if( card_type != SDIO_HIGH_CAPACITY_SD_CARD )
		LBA *= SD_RDWR_BLOCK_SIZE;
#if (SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE)
	int err;
	if ((err=isix_sem_wait( tlock_sem, ISIX_TIME_INFINITE )) != ISIX_EOK )
		return err;
	while ( (err = sd_get_status()) == SD_TRANSFER_BUSY )
	{
		isix_yield();
	}
	if( err != SD_TRANSFER_OK )
	{
		isix_sem_signal( tlock_sem );
		return SD_ERROR;
	}
	err =  sd_read_multi_blocks( buf, LBA, SD_RDWR_BLOCK_SIZE ,count );
	if( err )
	{
		isix_sem_signal( tlock_sem );
		return err;
	}
	err = sd_wait_read_operation();
	isix_sem_signal( tlock_sem );
	return err;
#elif (SDDRV_TRANSFER_MODE==SDDRV_POLLING_MODE)
	int err;
	if ((err=isix_sem_wait( tlock_sem, ISIX_TIME_INFINITE )) != ISIX_EOK )
		return err;
	uint8_t *pb = buf;
	while ( (err = sd_get_status()) == SD_TRANSFER_BUSY )
	{
		isix_yield();
	}
	if( err != SD_TRANSFER_OK )
	{
		isix_sem_signal( tlock_sem );
		return SD_ERROR;
	}
	for( unsigned long s = 0; s < count; s++ )
	{
		err = sd_read_block( pb+(s*512), LBA+s, SD_RDWR_BLOCK_SIZE );
		if( err )
		{
			isix_sem_signal( tlock_sem );
			return err;
		}
	}
	isix_sem_signal( tlock_sem );
	return err;
#else
#error "unknown SDDRV_TRANSFER_MODE"
#endif
}
/*--------------------------------------------------------------------*/
//SDIO card driver write
int isix_sdio_card_driver_write( const void *buf, unsigned long LBA, size_t count )
{
	if( !tlock_sem )
		return SD_LIB_NOT_INITIALIZED;
	if( card_type != SDIO_HIGH_CAPACITY_SD_CARD )
		LBA *= SD_RDWR_BLOCK_SIZE;
#if SDDRV_TRANSFER_MODE==SDDRV_DMA_MODE
	int err;
	if( (err=isix_sem_wait( tlock_sem, ISIX_TIME_INFINITE )) != ISIX_EOK)
		return err;
	while ( (err = sd_get_status()) == SD_TRANSFER_BUSY )
	{
		isix_yield();
	}
	if( err != SD_TRANSFER_OK )
	{
		isix_sem_signal( tlock_sem );
		return SD_ERROR;
	}
	err  = sd_write_multi_blocks( buf, LBA, SD_RDWR_BLOCK_SIZE, count );
	if( err )
	{
		isix_sem_signal( tlock_sem );
		return err;
	}
	err = sd_wait_write_operation();
	isix_sem_signal( tlock_sem );
	return err;
#elif (SDDRV_TRANSFER_MODE==SDDRV_POLLING_MODE)
	isix_sem_wait( tlock_sem, ISIX_TIME_INFINITE );
	int err;
	while ( (err = sd_get_status()) == SD_TRANSFER_BUSY )
	{
		isix_yield();
	}
	if( err != SD_TRANSFER_OK )
	{
		isix_sem_signal( tlock_sem );
		return SD_ERROR;
	}
	const uint8_t *pb = buf;
	for( unsigned long s = 0; s < count; s++ )
	{
		err = sd_write_block( pb+(s*512), LBA+s, SD_RDWR_BLOCK_SIZE );
		if( err )
		{
			isix_sem_signal( tlock_sem );
			return err;
		}
	}
	isix_sem_signal( tlock_sem );
	return err;
#endif
}
/* ------------------------------------------------------------------ */
//Card driver status
sdcard_drvstat isix_sdio_card_driver_status(void)
{
	if( tlock_sem )
	{
		if( isix_sem_wait( tlock_sem, ISIX_TIME_INFINITE ) != ISIX_EOK )
			return SDCARD_DRVSTAT_INTERNAL;
	}
	sdcard_drvstat cstat = SDCARD_DRVSTAT_OK;
	do {
		if( sd_detect() == SD_NOT_PRESENT )
		{
			cstat = SDCARD_DRVSTAT_NODISK;
			break;
		}
		if( !tlock_sem )
			cstat = SDCARD_DRVSTAT_NOINIT;
		else
		{
			switch( sd_get_status() )
			{
			case SD_TRANSFER_BUSY:
				cstat = SDCARD_DRVSTAT_BUSY;
				break;
			case SD_TRANSFER_OK:
				break;
			default:
				cstat = SDCARD_DRVSTAT_NODISK;
				break;
			}
		}
	} while(0);
	if( tlock_sem ) isix_sem_signal( tlock_sem );
	return cstat;
}
/*--------------------------------------------------------------------*/
//Get SD card info
int isix_sdio_card_driver_get_info( void *cardinfo, scard_info_field req )
{
	int err;
	if( !tlock_sem )
		return SD_LIB_NOT_INITIALIZED;
	if( (err=isix_sem_wait( tlock_sem, ISIX_TIME_INFINITE )) != ISIX_EOK )
		return err;
	while ( (err = sd_get_status()) == SD_TRANSFER_BUSY )
	{
		isix_yield();
	}
	if( err != SD_TRANSFER_OK )
	{
		isix_sem_signal( tlock_sem );
		return SD_ERROR;
	}
	err = sd_get_card_info( cardinfo, req );
	isix_sem_signal( tlock_sem );
	return err;
}
/*--------------------------------------------------------------------*/
//SD CARD initialize again
int isix_sdio_card_driver_reinitialize( void )
{
	int err;
	if((err=isix_sem_wait( tlock_sem, ISIX_TIME_INFINITE )) != ISIX_EOK )
	{
		return err;
	}
	err =  sd_card_init( );
	isix_sem_signal( tlock_sem );
	return err;
}
/*--------------------------------------------------------------------*/
//IS card present
bool isix_sdio_card_driver_is_card_in_slot( void )
{
	return (sd_detect() == SD_PRESENT);
}
/*--------------------------------------------------------------------*/
