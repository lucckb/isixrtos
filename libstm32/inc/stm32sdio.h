/*
 * stm32sdio.h
 *
 *  Created on: 22-11-2012
 *      Author: lucck
 */

#ifndef STM32_SDIO_H_
#define STM32_SDIO_H_
/*----------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include <stm32rcc.h>
#include "stm32lib.h"
#include "stm32fxxx_sdio.h"
/*----------------------------------------------------------*/
#ifdef __cplusplus
namespace stm32 {
#endif
/*----------------------------------------------------------*/
#define SDIO_OFFSET                (SDIO_BASE - PERIPH_BASE)
/* --- CLKCR Register ---*/
/* Alias word address of CLKEN bit */
#define CLKCR_OFFSET              (SDIO_OFFSET + 0x04)
#define CLKEN_BitNumber           0x08
#define CLKCR_CLKEN_BB            (PERIPH_BB_BASE + (CLKCR_OFFSET * 32) + (CLKEN_BitNumber * 4))
/* --- CMD Register ---*/
/* Alias word address of SDIOSUSPEND bit */
#define CMD_OFFSET                (SDIO_OFFSET + 0x0C)
#define SDIOSUSPEND_BitNumber     0x0B
#define CMD_SDIOSUSPEND_BB        (PERIPH_BB_BASE + (CMD_OFFSET * 32) + (SDIOSUSPEND_BitNumber * 4))
/* Alias word address of ENCMDCOMPL bit */
#define ENCMDCOMPL_BitNumber      0x0C
#define CMD_ENCMDCOMPL_BB         (PERIPH_BB_BASE + (CMD_OFFSET * 32) + (ENCMDCOMPL_BitNumber * 4))
/* Alias word address of NIEN bit */
#define NIEN_BitNumber            0x0D
#define CMD_NIEN_BB               (PERIPH_BB_BASE + (CMD_OFFSET * 32) + (NIEN_BitNumber * 4))
/* Alias word address of ATACMD bit */
#define ATACMD_BitNumber          0x0E
#define CMD_ATACMD_BB             (PERIPH_BB_BASE + (CMD_OFFSET * 32) + (ATACMD_BitNumber * 4))
/* --- DCTRL Register ---*/
/* Alias word address of DMAEN bit */
#define DCTRL_OFFSET              (SDIO_OFFSET + 0x2C)
#define DMAEN_BitNumber           0x03
#define DCTRL_DMAEN_BB            (PERIPH_BB_BASE + (DCTRL_OFFSET * 32) + (DMAEN_BitNumber * 4))
/* Alias word address of RWSTART bit */
#define RWSTART_BitNumber         0x08
#define DCTRL_RWSTART_BB          (PERIPH_BB_BASE + (DCTRL_OFFSET * 32) + (RWSTART_BitNumber * 4))
/* Alias word address of RWSTOP bit */
#define RWSTOP_BitNumber          0x09
#define DCTRL_RWSTOP_BB           (PERIPH_BB_BASE + (DCTRL_OFFSET * 32) + (RWSTOP_BitNumber * 4))

/* Alias word address of RWMOD bit */
#define RWMOD_BitNumber           0x0A
#define DCTRL_RWMOD_BB            (PERIPH_BB_BASE + (DCTRL_OFFSET * 32) + (RWMOD_BitNumber * 4))

/* Alias word address of SDIOEN bit */
#define SDIOEN_BitNumber          0x0B
#define DCTRL_SDIOEN_BB           (PERIPH_BB_BASE + (DCTRL_OFFSET * 32) + (SDIOEN_BitNumber * 4))
/* CLKCR register clear mask */
#define CLKCR_CLEAR_MASK         ((uint32_t)0xFFFF8100)
/* SDIO PWRCTRL Mask */
#define PWR_PWRCTRL_MASK         ((uint32_t)0xFFFFFFFC)
/* SDIO DCTRL Clear Mask */
#define DCTRL_CLEAR_MASK         ((uint32_t)0xFFFFFF08)
/* CMD Register clear mask */
#define CMD_CLEAR_MASK           ((uint32_t)0xFFFFF800)
/* SDIO RESP Registers Address */
#define SDIO_RESP_ADDR           ((uint32_t)(SDIO_BASE + 0x14))
/*----------------------------------------------------------*/
/**
  * @brief  Deinitializes the SDIO peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
static inline void sdio_deinit(void)
{
  rcc_apb2_periph_reset_cmd(RCC_APB2Periph_SDIO, true);
  rcc_apb2_periph_reset_cmd(RCC_APB2Periph_SDIO, false);
}
/*----------------------------------------------------------*/
/**
  * @brief  Initializes the SDIO peripheral according to the specified
  *         parameters in the SDIO_InitStruct.
  * @param  SDIO_InitStruct : pointer to a SDIO_InitTypeDef structure
  *         that contains the configuration information for the SDIO peripheral.
  * @retval None
  */
static inline void sdio_init(uint32_t clock_edge, uint32_t clock_bypass, uint32_t clock_power_save,
		uint32_t bus_wide, uint32_t hardware_flow_ctrl, uint8_t clk_div )
{
  uint32_t tmpreg;


/*---------------------------- SDIO CLKCR Configuration ------------------------*/
  /* Get the SDIO CLKCR value */
  tmpreg = SDIO->CLKCR;

  /* Clear CLKDIV, PWRSAV, BYPASS, WIDBUS, NEGEDGE, HWFC_EN bits */
  tmpreg &= CLKCR_CLEAR_MASK;

  /* Set CLKDIV bits according to SDIO_ClockDiv value */
  /* Set PWRSAV bit according to SDIO_ClockPowerSave value */
  /* Set BYPASS bit according to SDIO_ClockBypass value */
  /* Set WIDBUS bits according to SDIO_BusWide value */
  /* Set NEGEDGE bits according to SDIO_ClockEdge value */
  /* Set HWFC_EN bits according to SDIO_HardwareFlowControl value */
  tmpreg |= (clk_div  | clock_power_save |
		  clock_bypass | bus_wide |
		  clock_edge | hardware_flow_ctrl );
  /* Write to SDIO CLKCR */
  SDIO->CLKCR = tmpreg;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the SDIO Clock.
  * @param  NewState: new state of the SDIO Clock.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_clock_cmd(bool enable)
{

  *(__IO uint32_t *) CLKCR_CLKEN_BB = (uint32_t)enable;
}
/*----------------------------------------------------------*/
/**
  * @brief  Sets the power status of the controller.
  * @param  SDIO_PowerState: new state of the Power state.
  *          This parameter can be one of the following values:
  *            @arg SDIO_PowerState_OFF: SDIO Power OFF
  *            @arg SDIO_PowerState_ON: SDIO Power ON
  * @retval None
  */
static inline void sdio_set_power_state(uint32_t SDIO_PowerState)
{

  SDIO->POWER = SDIO_PowerState;
}
/*----------------------------------------------------------*/
/**
  * @brief  Gets the power status of the controller.
  * @param  None
  * @retval Power status of the controller. The returned value can be one of the
  *         following values:
  *            - 0x00: Power OFF
  *            - 0x02: Power UP
  *            - 0x03: Power ON
  */
static inline uint32_t sdio_get_power_state(void)
{
  return (SDIO->POWER & (~PWR_PWRCTRL_MASK));
}
/*----------------------------------------------------------*/
/**
  * @brief  Initializes the SDIO Command according to the specified
  *         parameters in the SDIO_CmdInitStruct and send the command.
  * @param  SDIO_CmdInitStruct : pointer to a SDIO_CmdInitTypeDef
  *         structure that contains the configuration information for the SDIO
  *         command.
  * @retval None
  */
static inline void sdio_send_command(uint32_t argument, uint32_t cmd_index,
		uint32_t response, uint32_t wait, uint32_t cpsm )
{
  uint32_t tmpreg;

/*---------------------------- SDIO ARG Configuration ------------------------*/
  /* Set the SDIO Argument value */
  SDIO->ARG = argument;

/*---------------------------- SDIO CMD Configuration ------------------------*/
  /* Get the SDIO CMD value */
  tmpreg = SDIO->CMD;
  /* Clear CMDINDEX, WAITRESP, WAITINT, WAITPEND, CPSMEN bits */
  tmpreg &= CMD_CLEAR_MASK;
  /* Set CMDINDEX bits according to SDIO_CmdIndex value */
  /* Set WAITRESP bits according to SDIO_Response value */
  /* Set WAITINT and WAITPEND bits according to SDIO_Wait value */
  /* Set CPSMEN bits according to SDIO_CPSM value */
  tmpreg |= (uint32_t)cmd_index | response
           | wait | cpsm;

  /* Write to SDIO CMD */
  SDIO->CMD = tmpreg;
}
/*----------------------------------------------------------*/
/**
  * @brief  Returns response received from the card for the last command.
  * @param  SDIO_RESP: Specifies the SDIO response register.
  *          This parameter can be one of the following values:
  *            @arg SDIO_RESP1: Response Register 1
  *            @arg SDIO_RESP2: Response Register 2
  *            @arg SDIO_RESP3: Response Register 3
  *            @arg SDIO_RESP4: Response Register 4
  * @retval The Corresponding response register value.
  */
static inline uint32_t sdio_get_response(uint32_t SDIO_RESP)
{

  const uint32_t tmp = SDIO_RESP_ADDR + SDIO_RESP;

  return (*(__IO uint32_t *) tmp);
}
/*----------------------------------------------------------*/
/**
  * @brief  Initializes the SDIO data path according to the specified
  *         parameters in the SDIO_DataInitStruct.
  * @param  SDIO_DataInitStruct : pointer to a SDIO_DataInitTypeDef structure
  *         that contains the configuration information for the SDIO command.
  * @retval None
  */
static inline void sdio_data_config( uint32_t timeout, uint32_t length, uint32_t block_size,
		uint32_t transfer_dir, uint32_t transfer_mode, uint32_t dpsm )
{
  uint32_t tmpreg;

/*---------------------------- SDIO DTIMER Configuration ---------------------*/
  /* Set the SDIO Data TimeOut value */
  SDIO->DTIMER = timeout;

/*---------------------------- SDIO DLEN Configuration -----------------------*/
  /* Set the SDIO DataLength value */
  SDIO->DLEN = length;

/*---------------------------- SDIO DCTRL Configuration ----------------------*/
  /* Get the SDIO DCTRL value */
  tmpreg = SDIO->DCTRL;
  /* Clear DEN, DTMODE, DTDIR and DBCKSIZE bits */
  tmpreg &= DCTRL_CLEAR_MASK;
  /* Set DEN bit according to SDIO_DPSM value */
  /* Set DTMODE bit according to SDIO_TransferMode value */
  /* Set DTDIR bit according to SDIO_TransferDir value */
  /* Set DBCKSIZE bits according to SDIO_DataBlockSize value */
  tmpreg |= block_size | transfer_dir
           | transfer_mode | dpsm;

  /* Write to SDIO DCTRL */
  SDIO->DCTRL = tmpreg;
}

/*----------------------------------------------------------*/
/**
  * @brief  Returns number of remaining data bytes to be transferred.
  * @param  None
  * @retval Number of remaining data bytes to be transferred
  */
static inline uint32_t sdio_get_data_counter(void)
{
  return SDIO->DCOUNT;
}
/*----------------------------------------------------------*/
/**
  * @brief  Read one data word from Rx FIFO.
  * @param  None
  * @retval Data received
  */
static inline uint32_t sdio_read_data(void)
{
  return SDIO->FIFO;
}
/*----------------------------------------------------------*/
/**
  * @brief  Write one data word to Tx FIFO.
  * @param  Data: 32-bit data word to write.
  * @retval None
  */
static inline void sdio_write_data(uint32_t Data)
{
  SDIO->FIFO = Data;
}
/*----------------------------------------------------------*/
/**
  * @brief  Returns the number of words left to be written to or read from FIFO.
  * @param  None
  * @retval Remaining number of words.
  */
static inline uint32_t sdio_get_fifo_count(void)
{
  return SDIO->FIFOCNT;
}
/*----------------------------------------------------------*/
/**
  * @brief  Starts the SD I/O Read Wait operation.
  * @param  NewState: new state of the Start SDIO Read Wait operation.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_start_sdio_read_wait( bool enable)
{

  *(__IO uint32_t *) DCTRL_RWSTART_BB = (uint32_t) enable;
}
/*----------------------------------------------------------*/
/**
  * @brief  Stops the SD I/O Read Wait operation.
  * @param  NewState: new state of the Stop SDIO Read Wait operation.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_stop_sdio_read_wait(bool enable)
{

  *(__IO uint32_t *) DCTRL_RWSTOP_BB = (uint32_t)enable;
}
/*----------------------------------------------------------*/
/**
  * @brief  Sets one of the two options of inserting read wait interval.
  * @param  SDIO_ReadWaitMode: SD I/O Read Wait operation mode.
  *          This parameter can be:
  *            @arg SDIO_ReadWaitMode_CLK: Read Wait control by stopping SDIOCLK
  *            @arg SDIO_ReadWaitMode_DATA2: Read Wait control using SDIO_DATA2
  * @retval None
  */
static inline void sdio_set_sdio_read_wait_mode(uint32_t SDIO_ReadWaitMode)
{

  *(__IO uint32_t *) DCTRL_RWMOD_BB = SDIO_ReadWaitMode;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the SD I/O Mode Operation.
  * @param  NewState: new state of SDIO specific operation.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_set_sdio_operation(bool enable)
{

  *(__IO uint32_t *) DCTRL_SDIOEN_BB = enable;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the SD I/O Mode suspend command sending.
  * @param  NewState: new state of the SD I/O Mode suspend command.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_send_sdio_suspend_cmd(bool enable)
{

  *(__IO uint32_t *) CMD_SDIOSUSPEND_BB = (uint32_t)enable;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the command completion signal.
  * @param  NewState: new state of command completion signal.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_command_completion_cmd(bool enable)
{
  *(__IO uint32_t *) CMD_ENCMDCOMPL_BB = (uint32_t)enable;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the CE-ATA interrupt.
  * @param  NewState: new state of CE-ATA interrupt.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_ceata_it_cmd(bool enable)
{

  *(__IO uint32_t *) CMD_NIEN_BB = (uint32_t)((~((uint32_t)enable)) & ((uint32_t)0x1));
}
/*----------------------------------------------------------*/
/**
  * @brief  Sends CE-ATA command (CMD61).
  * @param  NewState: new state of CE-ATA command.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_send_ceata_cmd(bool enable)
{
  *(__IO uint32_t *) CMD_ATACMD_BB = (uint32_t)enable;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the SDIO DMA request.
  * @param  NewState: new state of the selected SDIO DMA request.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_dma_cmd(bool enable)
{

  *(__IO uint32_t *) DCTRL_DMAEN_BB = (uint32_t)enable;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the SDIO interrupts.
  * @param  SDIO_IT: specifies the SDIO interrupt sources to be enabled or disabled.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDIO_IT_CCRCFAIL: Command response received (CRC check failed) interrupt
  *            @arg SDIO_IT_DCRCFAIL: Data block sent/received (CRC check failed) interrupt
  *            @arg SDIO_IT_CTIMEOUT: Command response timeout interrupt
  *            @arg SDIO_IT_DTIMEOUT: Data timeout interrupt
  *            @arg SDIO_IT_TXUNDERR: Transmit FIFO underrun error interrupt
  *            @arg SDIO_IT_RXOVERR:  Received FIFO overrun error interrupt
  *            @arg SDIO_IT_CMDREND:  Command response received (CRC check passed) interrupt
  *            @arg SDIO_IT_CMDSENT:  Command sent (no response required) interrupt
  *            @arg SDIO_IT_DATAEND:  Data end (data counter, SDIDCOUNT, is zero) interrupt
  *            @arg SDIO_IT_STBITERR: Start bit not detected on all data signals in wide
  *                                   bus mode interrupt
  *            @arg SDIO_IT_DBCKEND:  Data block sent/received (CRC check passed) interrupt
  *            @arg SDIO_IT_CMDACT:   Command transfer in progress interrupt
  *            @arg SDIO_IT_TXACT:    Data transmit in progress interrupt
  *            @arg SDIO_IT_RXACT:    Data receive in progress interrupt
  *            @arg SDIO_IT_TXFIFOHE: Transmit FIFO Half Empty interrupt
  *            @arg SDIO_IT_RXFIFOHF: Receive FIFO Half Full interrupt
  *            @arg SDIO_IT_TXFIFOF:  Transmit FIFO full interrupt
  *            @arg SDIO_IT_RXFIFOF:  Receive FIFO full interrupt
  *            @arg SDIO_IT_TXFIFOE:  Transmit FIFO empty interrupt
  *            @arg SDIO_IT_RXFIFOE:  Receive FIFO empty interrupt
  *            @arg SDIO_IT_TXDAVL:   Data available in transmit FIFO interrupt
  *            @arg SDIO_IT_RXDAVL:   Data available in receive FIFO interrupt
  *            @arg SDIO_IT_SDIOIT:   SD I/O interrupt received interrupt
  *            @arg SDIO_IT_CEATAEND: CE-ATA command completion signal received for CMD61 interrupt
  * @param  NewState: new state of the specified SDIO interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void sdio_it_config(uint32_t SDIO_IT, bool enable)
{
  if ( enable )
  {
    /* Enable the SDIO interrupts */
    SDIO->MASK |= SDIO_IT;
  }
  else
  {
    /* Disable the SDIO interrupts */
    SDIO->MASK &= ~SDIO_IT;
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Checks whether the specified SDIO flag is set or not.
  * @param  SDIO_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg SDIO_FLAG_CCRCFAIL: Command response received (CRC check failed)
  *            @arg SDIO_FLAG_DCRCFAIL: Data block sent/received (CRC check failed)
  *            @arg SDIO_FLAG_CTIMEOUT: Command response timeout
  *            @arg SDIO_FLAG_DTIMEOUT: Data timeout
  *            @arg SDIO_FLAG_TXUNDERR: Transmit FIFO underrun error
  *            @arg SDIO_FLAG_RXOVERR:  Received FIFO overrun error
  *            @arg SDIO_FLAG_CMDREND:  Command response received (CRC check passed)
  *            @arg SDIO_FLAG_CMDSENT:  Command sent (no response required)
  *            @arg SDIO_FLAG_DATAEND:  Data end (data counter, SDIDCOUNT, is zero)
  *            @arg SDIO_FLAG_STBITERR: Start bit not detected on all data signals in wide bus mode.
  *            @arg SDIO_FLAG_DBCKEND:  Data block sent/received (CRC check passed)
  *            @arg SDIO_FLAG_CMDACT:   Command transfer in progress
  *            @arg SDIO_FLAG_TXACT:    Data transmit in progress
  *            @arg SDIO_FLAG_RXACT:    Data receive in progress
  *            @arg SDIO_FLAG_TXFIFOHE: Transmit FIFO Half Empty
  *            @arg SDIO_FLAG_RXFIFOHF: Receive FIFO Half Full
  *            @arg SDIO_FLAG_TXFIFOF:  Transmit FIFO full
  *            @arg SDIO_FLAG_RXFIFOF:  Receive FIFO full
  *            @arg SDIO_FLAG_TXFIFOE:  Transmit FIFO empty
  *            @arg SDIO_FLAG_RXFIFOE:  Receive FIFO empty
  *            @arg SDIO_FLAG_TXDAVL:   Data available in transmit FIFO
  *            @arg SDIO_FLAG_RXDAVL:   Data available in receive FIFO
  *            @arg SDIO_FLAG_SDIOIT:   SD I/O interrupt received
  *            @arg SDIO_FLAG_CEATAEND: CE-ATA command completion signal received for CMD61
  * @retval The new state of SDIO_FLAG (SET or RESET).
  */
static inline bool sdio_get_flag_status(uint32_t SDIO_FLAG)
{
  return (SDIO->STA & SDIO_FLAG)?(true):(false);
}
/*----------------------------------------------------------*/
/**
  * @brief  Clears the SDIO's pending flags.
  * @param  SDIO_FLAG: specifies the flag to clear.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDIO_FLAG_CCRCFAIL: Command response received (CRC check failed)
  *            @arg SDIO_FLAG_DCRCFAIL: Data block sent/received (CRC check failed)
  *            @arg SDIO_FLAG_CTIMEOUT: Command response timeout
  *            @arg SDIO_FLAG_DTIMEOUT: Data timeout
  *            @arg SDIO_FLAG_TXUNDERR: Transmit FIFO underrun error
  *            @arg SDIO_FLAG_RXOVERR:  Received FIFO overrun error
  *            @arg SDIO_FLAG_CMDREND:  Command response received (CRC check passed)
  *            @arg SDIO_FLAG_CMDSENT:  Command sent (no response required)
  *            @arg SDIO_FLAG_DATAEND:  Data end (data counter, SDIDCOUNT, is zero)
  *            @arg SDIO_FLAG_STBITERR: Start bit not detected on all data signals in wide bus mode
  *            @arg SDIO_FLAG_DBCKEND:  Data block sent/received (CRC check passed)
  *            @arg SDIO_FLAG_SDIOIT:   SD I/O interrupt received
  *            @arg SDIO_FLAG_CEATAEND: CE-ATA command completion signal received for CMD61
  * @retval None
  */
static inline void sdio_clear_flag(uint32_t SDIO_FLAG)
{

  SDIO->ICR = SDIO_FLAG;
}
/*----------------------------------------------------------*/
/**
  * @brief  Checks whether the specified SDIO interrupt has occurred or not.
  * @param  SDIO_IT: specifies the SDIO interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg SDIO_IT_CCRCFAIL: Command response received (CRC check failed) interrupt
  *            @arg SDIO_IT_DCRCFAIL: Data block sent/received (CRC check failed) interrupt
  *            @arg SDIO_IT_CTIMEOUT: Command response timeout interrupt
  *            @arg SDIO_IT_DTIMEOUT: Data timeout interrupt
  *            @arg SDIO_IT_TXUNDERR: Transmit FIFO underrun error interrupt
  *            @arg SDIO_IT_RXOVERR:  Received FIFO overrun error interrupt
  *            @arg SDIO_IT_CMDREND:  Command response received (CRC check passed) interrupt
  *            @arg SDIO_IT_CMDSENT:  Command sent (no response required) interrupt
  *            @arg SDIO_IT_DATAEND:  Data end (data counter, SDIDCOUNT, is zero) interrupt
  *            @arg SDIO_IT_STBITERR: Start bit not detected on all data signals in wide
  *                                   bus mode interrupt
  *            @arg SDIO_IT_DBCKEND:  Data block sent/received (CRC check passed) interrupt
  *            @arg SDIO_IT_CMDACT:   Command transfer in progress interrupt
  *            @arg SDIO_IT_TXACT:    Data transmit in progress interrupt
  *            @arg SDIO_IT_RXACT:    Data receive in progress interrupt
  *            @arg SDIO_IT_TXFIFOHE: Transmit FIFO Half Empty interrupt
  *            @arg SDIO_IT_RXFIFOHF: Receive FIFO Half Full interrupt
  *            @arg SDIO_IT_TXFIFOF:  Transmit FIFO full interrupt
  *            @arg SDIO_IT_RXFIFOF:  Receive FIFO full interrupt
  *            @arg SDIO_IT_TXFIFOE:  Transmit FIFO empty interrupt
  *            @arg SDIO_IT_RXFIFOE:  Receive FIFO empty interrupt
  *            @arg SDIO_IT_TXDAVL:   Data available in transmit FIFO interrupt
  *            @arg SDIO_IT_RXDAVL:   Data available in receive FIFO interrupt
  *            @arg SDIO_IT_SDIOIT:   SD I/O interrupt received interrupt
  *            @arg SDIO_IT_CEATAEND: CE-ATA command completion signal received for CMD61 interrupt
  * @retval The new state of SDIO_IT (SET or RESET).
  */
static inline bool sdio_get_it_status(uint32_t SDIO_IT)
{
  return (SDIO->STA & SDIO_IT)?(true):(false);
}
/*----------------------------------------------------------*/
/**
  * @brief  Clears the SDIO's interrupt pending bits.
  * @param  SDIO_IT: specifies the interrupt pending bit to clear.
  *          This parameter can be one or a combination of the following values:
  *            @arg SDIO_IT_CCRCFAIL: Command response received (CRC check failed) interrupt
  *            @arg SDIO_IT_DCRCFAIL: Data block sent/received (CRC check failed) interrupt
  *            @arg SDIO_IT_CTIMEOUT: Command response timeout interrupt
  *            @arg SDIO_IT_DTIMEOUT: Data timeout interrupt
  *            @arg SDIO_IT_TXUNDERR: Transmit FIFO underrun error interrupt
  *            @arg SDIO_IT_RXOVERR:  Received FIFO overrun error interrupt
  *            @arg SDIO_IT_CMDREND:  Command response received (CRC check passed) interrupt
  *            @arg SDIO_IT_CMDSENT:  Command sent (no response required) interrupt
  *            @arg SDIO_IT_DATAEND:  Data end (data counter, SDIO_DCOUNT, is zero) interrupt
  *            @arg SDIO_IT_STBITERR: Start bit not detected on all data signals in wide
  *                                   bus mode interrupt
  *            @arg SDIO_IT_SDIOIT:   SD I/O interrupt received interrupt
  *            @arg SDIO_IT_CEATAEND: CE-ATA command completion signal received for CMD61
  * @retval None
  */
static inline void sdio_clear_it_pending_bit(uint32_t SDIO_IT)
{

  SDIO->ICR = SDIO_IT;
}
/*----------------------------------------------------------*/
/**
  * @brief  Returns command index of last command for which response received.
  * @param  None
  * @retval Returns the command index of the last command response received.
  */
static inline uint8_t sdio_get_command_response(void)
{
  return (SDIO->RESPCMD);
}

/*----------------------------------------------------------*/
//Undef section
#undef SDIO_OFFSET
#undef CLKCR_OFFSET
#undef CLKEN_BitNumber
#undef CLKCR_CLKEN_BB
#undef CMD_OFFSET
#undef SDIOSUSPEND_BitNumber
#undef CMD_SDIOSUSPEND_BB
#undef ENCMDCOMPL_BitNumber
#undef CMD_ENCMDCOMPL_BB
#undef NIEN_BitNumber
#undef CMD_NIEN_BB
#undef ATACMD_BitNumber
#undef CMD_ATACMD_BB
#undef DCTRL_OFFSET
#undef DMAEN_BitNumber
#undef DCTRL_DMAEN_BB
#undef RWSTART_BitNumber
#undef DCTRL_RWSTART_BB
#undef RWSTOP_BitNumber
#undef DCTRL_RWSTOP_BB
#undef RWMOD_BitNumber
#undef DCTRL_RWMOD_BB
#undef SDIOEN_BitNumber
#undef DCTRL_SDIOEN_BB
#undef CLKCR_CLEAR_MASK
#undef PWR_PWRCTRL_MASK
#undef DCTRL_CLEAR_MASK
#undef CMD_CLEAR_MASK
#undef SDIO_RESP_ADDR

/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------------------*/
#endif /* STM32SDIO_H_ */
