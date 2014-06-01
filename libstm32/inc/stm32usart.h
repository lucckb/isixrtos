/*
 * stm32usart.h
 *
 *  Created on: 23 lis 2013
 *      Author: lucck
 */

#ifndef STM32USART_H_
#define STM32USART_H_
/*----------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stm32rcc.h>
#include "stm32lib.h"
#include "stm32fxxx_usart.h"

/*----------------------------------------------------------*/
#define CR1_CLEAR_MASK            ((uint16_t)(USART_CR1_M | USART_CR1_PCE | \
                                              USART_CR1_PS | USART_CR1_TE | \
                                              USART_CR1_RE))
/*!< USART CR3 register clear Mask ((~(uint16_t)0xFCFF)) */
#define CR3_CLEAR_MASK            ((uint16_t)(USART_CR3_RTSE | USART_CR3_CTSE))

/*!< USART CR2 register clock bits clear Mask ((~(uint16_t)0xF0FF)) */
#define CR2_CLOCK_CLEAR_MASK      ((uint16_t)(USART_CR2_CLKEN | USART_CR2_CPOL | \
                                              USART_CR2_CPHA | USART_CR2_LBCL))

/*!< USART Interrupts mask */
#define IT_MASK                   ((uint16_t)0x001F)

/*----------------------------------------------------------*/
#ifdef __cplusplus
namespace stm32 {
#endif
/*----------------------------------------------------------*/
/**
  * @brief  Initializes the USARTx peripheral according to the specified
  *         parameters in the USART_InitStruct .
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that contains
  *         the configuration information for the specified USART peripheral.
  * @retval None
  */
static inline void usart_init(USART_TypeDef* USARTx, uint32_t baudrate, uint16_t word_length,
		uint16_t stop_bits, uint16_t parity, uint16_t mode, uint16_t hw_flow_ctl,
		unsigned pclk1, unsigned pclk2)
{
  uint32_t tmpreg = 0x00, apbclock = 0x00;
  uint32_t integerdivider = 0x00;
  uint32_t fractionaldivider = 0x00;


/*---------------------------- USART CR2 Configuration -----------------------*/
  tmpreg = USARTx->CR2;

  /* Clear STOP[13:12] bits */
  tmpreg &= (uint32_t)~((uint32_t)USART_CR2_STOP);

  /* Configure the USART Stop Bits, Clock, CPOL, CPHA and LastBit :
      Set STOP[13:12] bits according to USART_StopBits value */
  tmpreg |= (uint32_t)stop_bits;

  /* Write to USART CR2 */
  USARTx->CR2 = (uint16_t)tmpreg;

/*---------------------------- USART CR1 Configuration -----------------------*/
  tmpreg = USARTx->CR1;

  /* Clear M, PCE, PS, TE and RE bits */
  tmpreg &= (uint32_t)~((uint32_t)CR1_CLEAR_MASK);

  /* Configure the USART Word Length, Parity and mode:
     Set the M bits according to USART_WordLength value
     Set PCE and PS bits according to USART_Parity value
     Set TE and RE bits according to USART_Mode value */
  tmpreg |= (uint32_t)word_length | parity | mode;

  /* Write to USART CR1 */
  USARTx->CR1 = (uint16_t)tmpreg;

/*---------------------------- USART CR3 Configuration -----------------------*/
  tmpreg = USARTx->CR3;

  /* Clear CTSE and RTSE bits */
  tmpreg &= (uint32_t)~((uint32_t)CR3_CLEAR_MASK);

  /* Configure the USART HFC :
      Set CTSE and RTSE bits according to USART_HardwareFlowControl value */
  tmpreg |= hw_flow_ctl;

  /* Write to USART CR3 */
  USARTx->CR3 = (uint16_t)tmpreg;

/*---------------------------- USART BRR Configuration -----------------------*/
#ifndef STM32MCU_MAJOR_TYPE_F1
  if ((USARTx == USART1) || (USARTx == USART6))
#else
  if( USARTx == USART1 ) 
#endif
  {
    apbclock = pclk2;
  }
  else
  {
    apbclock = pclk1;
  }

  /* Determine the integer part */
  if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
  {
    /* Integer part computing in case Oversampling mode is 8 Samples */
    integerdivider = ((25 * apbclock) / (2 * (baudrate)));
  }
  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
  {
    /* Integer part computing in case Oversampling mode is 16 Samples */
    integerdivider = ((25 * apbclock) / (4 * (baudrate)));
  }
  tmpreg = (integerdivider / 100) << 4;

  /* Determine the fractional part */
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

  /* Implement the fractional part in the register */
  if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
  {
    tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
  }
  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
  {
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
  }

  /* Write to USART BRR register */
  USARTx->BRR = (uint16_t)tmpreg;
}
/*----------------------------------------------------------*/
/**
  * @brief  Initializes the USARTx peripheral Clock according to the
  *         specified parameters in the USART_ClockInitStruct .
  * @param  USARTx: where x can be 1, 2, 3 or 6 to select the USART peripheral.
  * @param  USART_ClockInitStruct: pointer to a USART_ClockInitTypeDef structure that
  *         contains the configuration information for the specified  USART peripheral.
  * @note   The Smart Card and Synchronous modes are not available for UART4 and UART5.
  * @retval None
  */
static inline void usart_clock_init(USART_TypeDef* USARTx, uint16_t clock, uint16_t cpol,
		uint16_t cpha, uint16_t last_bit )
{
  uint32_t tmpreg = 0x00;

/*---------------------------- USART CR2 Configuration -----------------------*/
  tmpreg = USARTx->CR2;
  /* Clear CLKEN, CPOL, CPHA and LBCL bits */
  tmpreg &= (uint32_t)~((uint32_t)CR2_CLOCK_CLEAR_MASK);
  /* Configure the USART Clock, CPOL, CPHA and LastBit ------------*/
  /* Set CLKEN bit according to USART_Clock value */
  /* Set CPOL bit according to USART_CPOL value */
  /* Set CPHA bit according to USART_CPHA value */
  /* Set LBCL bit according to USART_LastBit value */
  tmpreg |= (uint32_t)clock | cpol | cpha | last_bit;
  /* Write to USART CR2 */
  USARTx->CR2 = (uint16_t)tmpreg;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the specified USART peripheral.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the USARTx peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_cmd(USART_TypeDef* USARTx, bool enabled )
{
  /* Check the parameters */

  if ( enabled )
  {
    /* Enable the selected USART by setting the UE bit in the CR1 register */
    USARTx->CR1 |= USART_CR1_UE;
  }
  else
  {
    /* Disable the selected USART by clearing the UE bit in the CR1 register */
    USARTx->CR1 &= (uint16_t)~((uint16_t)USART_CR1_UE);
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Sets the system clock prescaler.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_Prescaler: specifies the prescaler clock.
  * @note   The function is used for IrDA mode with UART4 and UART5.
  * @retval None
  */
static inline void usart_set_prescaler(USART_TypeDef* USARTx, uint8_t prescaler)
{

  /* Clear the USART prescaler */
  USARTx->GTPR &= USART_GTPR_GT;
  /* Set the USART prescaler */
  USARTx->GTPR |= prescaler;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the USART's 8x oversampling mode.
  * @note   This function has to be called before calling USART_Init() function
  *         in order to have correct baudrate Divider value.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the USART 8x oversampling mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_over_sampling8_cmd(USART_TypeDef* USARTx, bool enabled)
{
  if ( enabled )
  {
    /* Enable the 8x Oversampling mode by setting the OVER8 bit in the CR1 register */
    USARTx->CR1 |= USART_CR1_OVER8;
  }
  else
  {
    /* Disable the 8x Oversampling mode by clearing the OVER8 bit in the CR1 register */
    USARTx->CR1 &= (uint16_t)~((uint16_t)USART_CR1_OVER8);
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the USART's one bit sampling method.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the USART one bit sampling method.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_one_bit_method_cmd(USART_TypeDef* USARTx, bool enabled )
{
  if ( enabled )
  {
    /* Enable the one bit method by setting the ONEBITE bit in the CR3 register */
    USARTx->CR3 |= USART_CR3_ONEBIT;
  }
  else
  {
    /* Disable the one bit method by clearing the ONEBITE bit in the CR3 register */
    USARTx->CR3 &= (uint16_t)~((uint16_t)USART_CR3_ONEBIT);
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Transmits single data through the USARTx peripheral.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  Data: the data to transmit.
  * @retval None
  */
static inline void usart_send_data(USART_TypeDef* USARTx, uint16_t data)
{

  /* Transmit Data */
  USARTx->DR = (data & (uint16_t)0x01FF);
}
/*----------------------------------------------------------*/
/**
  * @brief  Returns the most recent received data by the USARTx peripheral.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @retval The received data.
  */
static inline uint16_t usart_receive_data(USART_TypeDef* USARTx)
{
  /* Receive Data */
  return (uint16_t)(USARTx->DR & (uint16_t)0x01FF);
}
/*----------------------------------------------------------*/
/**
  * @brief  Sets the address of the USART node.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_Address: Indicates the address of the USART node.
  * @retval None
  */
static inline void usart_set_address(USART_TypeDef* USARTx, uint8_t address )
{

  /* Clear the USART address */
  USARTx->CR2 &= (uint16_t)~((uint16_t)USART_CR2_ADD);
  /* Set the USART address node */
  USARTx->CR2 |= address;
}
/*----------------------------------------------------------*/
/**
  * @brief  Determines if the USART is in mute mode or not.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the USART mute mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_receiver_wake_up_cmd(USART_TypeDef* USARTx, bool enabled )
{
  if ( enabled )
  {
    /* Enable the USART mute mode  by setting the RWU bit in the CR1 register */
    USARTx->CR1 |= USART_CR1_RWU;
  }
  else
  {
    /* Disable the USART mute mode by clearing the RWU bit in the CR1 register */
    USARTx->CR1 &= (uint16_t)~((uint16_t)USART_CR1_RWU);
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Selects the USART WakeUp method.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_WakeUp: specifies the USART wakeup method.
  *          This parameter can be one of the following values:
  *            @arg USART_WakeUp_IdleLine: WakeUp by an idle line detection
  *            @arg USART_WakeUp_AddressMark: WakeUp by an address mark
  * @retval None
  */
static inline void usart_wakeup_config(USART_TypeDef* USARTx, uint16_t wakeup)
{

  USARTx->CR1 &= (uint16_t)~((uint16_t)USART_CR1_WAKE);
  USARTx->CR1 |= wakeup;
}
/*----------------------------------------------------------*/
/**
  * @brief  Sets the USART LIN Break detection length.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_LINBreakDetectLength: specifies the LIN break detection length.
  *          This parameter can be one of the following values:
  *            @arg USART_LINBreakDetectLength_10b: 10-bit break detection
  *            @arg USART_LINBreakDetectLength_11b: 11-bit break detection
  * @retval None
  */
static inline void usart_lin_break_detectlengthconfig(USART_TypeDef* USARTx, uint16_t length)
{

  USARTx->CR2 &= (uint16_t)~((uint16_t)USART_CR2_LBDL);
  USARTx->CR2 |= length;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the USART's LIN mode.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the USART LIN mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_lin_cmd(USART_TypeDef* USARTx, bool enabled)
{
  if ( enabled )
  {
    /* Enable the LIN mode by setting the LINEN bit in the CR2 register */
    USARTx->CR2 |= USART_CR2_LINEN;
  }
  else
  {
    /* Disable the LIN mode by clearing the LINEN bit in the CR2 register */
    USARTx->CR2 &= (uint16_t)~((uint16_t)USART_CR2_LINEN);
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Transmits break characters.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @retval None
  */
static inline void USART_SendBreak(USART_TypeDef* USARTx)
{
  /* Send break characters */
  USARTx->CR1 |= USART_CR1_SBK;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the USART's Half Duplex communication.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the USART Communication.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_half_duplex_cmd(USART_TypeDef* USARTx, bool enabled)
{

  if ( enabled )
  {
    /* Enable the Half-Duplex mode by setting the HDSEL bit in the CR3 register */
    USARTx->CR3 |= USART_CR3_HDSEL;
  }
  else
  {
    /* Disable the Half-Duplex mode by clearing the HDSEL bit in the CR3 register */
    USARTx->CR3 &= (uint16_t)~((uint16_t)USART_CR3_HDSEL);
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Sets the specified USART guard time.
  * @param  USARTx: where x can be 1, 2, 3 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_GuardTime: specifies the guard time.
  * @retval None
  */
static inline void usart_set_guard_time(USART_TypeDef* USARTx, uint8_t gtime)
{
  /* Clear the USART Guard time */
  USARTx->GTPR &= USART_GTPR_PSC;
  /* Set the USART guard time */
  USARTx->GTPR |= (uint16_t)((uint16_t)gtime << 0x08);
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the USART's Smart Card mode.
  * @param  USARTx: where x can be 1, 2, 3 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the Smart Card mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_smart_card_cmd(USART_TypeDef* USARTx, bool enabled )
{

  if ( enabled )
  {
    /* Enable the SC mode by setting the SCEN bit in the CR3 register */
    USARTx->CR3 |= USART_CR3_SCEN;
  }
  else
  {
    /* Disable the SC mode by clearing the SCEN bit in the CR3 register */
    USARTx->CR3 &= (uint16_t)~((uint16_t)USART_CR3_SCEN);
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables NACK transmission.
  * @param  USARTx: where x can be 1, 2, 3 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the NACK transmission.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_smart_card_nack_cmd(USART_TypeDef* USARTx, bool enabled )
{
  if ( enabled )
  {
    /* Enable the NACK transmission by setting the NACK bit in the CR3 register */
    USARTx->CR3 |= USART_CR3_NACK;
  }
  else
  {
    /* Disable the NACK transmission by clearing the NACK bit in the CR3 register */
    USARTx->CR3 &= (uint16_t)~((uint16_t)USART_CR3_NACK);
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Configures the USART's IrDA interface.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_IrDAMode: specifies the IrDA mode.
  *          This parameter can be one of the following values:
  *            @arg USART_IrDAMode_LowPower
  *            @arg USART_IrDAMode_Normal
  * @retval None
  */
static inline void usart_irda_config(USART_TypeDef* USARTx, uint16_t USART_IrDAMode)
{
  USARTx->CR3 &= (uint16_t)~((uint16_t)USART_CR3_IRLP);
  USARTx->CR3 |= USART_IrDAMode;
}
/*----------------------------------------------------------*/

/**
  * @brief  Enables or disables the USART's IrDA interface.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the IrDA mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_irda_cmd(USART_TypeDef* USARTx, bool enabled )
{
  if ( enabled )
  {
    /* Enable the IrDA mode by setting the IREN bit in the CR3 register */
    USARTx->CR3 |= USART_CR3_IREN;
  }
  else
  {
    /* Disable the IrDA mode by clearing the IREN bit in the CR3 register */
    USARTx->CR3 &= (uint16_t)~((uint16_t)USART_CR3_IREN);
  }
}
/*----------------------------------------------------------*/
static inline void usart_dma_cmd(USART_TypeDef* USARTx, uint16_t USART_DMAReq, bool enabled )
{
  if ( enabled )
  {
    /* Enable the DMA transfer for selected requests by setting the DMAT and/or
       DMAR bits in the USART CR3 register */
    USARTx->CR3 |= USART_DMAReq;
  }
  else
  {
    /* Disable the DMA transfer for selected requests by clearing the DMAT and/or
       DMAR bits in the USART CR3 register */
    USARTx->CR3 &= (uint16_t)~USART_DMAReq;
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the specified USART interrupts.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_IT: specifies the USART interrupt sources to be enabled or disabled.
  *          This parameter can be one of the following values:
  *            @arg USART_IT_CTS:  CTS change interrupt
  *            @arg USART_IT_LBD:  LIN Break detection interrupt
  *            @arg USART_IT_TXE:  Transmit Data Register empty interrupt
  *            @arg USART_IT_TC:   Transmission complete interrupt
  *            @arg USART_IT_RXNE: Receive Data register not empty interrupt
  *            @arg USART_IT_IDLE: Idle line detection interrupt
  *            @arg USART_IT_PE:   Parity Error interrupt
  *            @arg USART_IT_ERR:  Error interrupt(Frame error, noise error, overrun error)
  * @param  NewState: new state of the specified USARTx interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_it_config(USART_TypeDef* USARTx, uint16_t USART_IT, bool enabled )
{
  uint32_t usartreg = 0x00, itpos = 0x00, itmask = 0x00;
  uint32_t usartxbase = 0x00;

  usartxbase = (uint32_t)USARTx;

  /* Get the USART register index */
  usartreg = (((uint8_t)USART_IT) >> 0x05);

  /* Get the interrupt position */
  itpos = USART_IT & IT_MASK;
  itmask = (((uint32_t)0x01) << itpos);

  if (usartreg == 0x01) /* The IT is in CR1 register */
  {
    usartxbase += 0x0C;
  }
  else if (usartreg == 0x02) /* The IT is in CR2 register */
  {
    usartxbase += 0x10;
  }
  else /* The IT is in CR3 register */
  {
    usartxbase += 0x14;
  }
  if ( enabled )
  {
    *(__IO uint32_t*)usartxbase  |= itmask;
  }
  else
  {
    *(__IO uint32_t*)usartxbase &= ~itmask;
  }
}
/*----------------------------------------------------------*/
/**
  * @brief  Checks whether the specified USART flag is set or not.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg USART_FLAG_CTS:  CTS Change flag (not available for UART4 and UART5)
  *            @arg USART_FLAG_LBD:  LIN Break detection flag
  *            @arg USART_FLAG_TXE:  Transmit data register empty flag
  *            @arg USART_FLAG_TC:   Transmission Complete flag
  *            @arg USART_FLAG_RXNE: Receive data register not empty flag
  *            @arg USART_FLAG_IDLE: Idle Line detection flag
  *            @arg USART_FLAG_ORE:  OverRun Error flag
  *            @arg USART_FLAG_NE:   Noise Error flag
  *            @arg USART_FLAG_FE:   Framing Error flag
  *            @arg USART_FLAG_PE:   Parity Error flag
  * @retval The new state of USART_FLAG (SET or RESET).
  */
static inline bool usart_get_flag_status(USART_TypeDef* USARTx, uint16_t flag)
{
	return ((USARTx->SR & flag) != 0);
}
/*----------------------------------------------------------*/
/**
  * @brief  Clears the USARTx's pending flags.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_FLAG: specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg USART_FLAG_CTS:  CTS Change flag (not available for UART4 and UART5).
  *            @arg USART_FLAG_LBD:  LIN Break detection flag.
  *            @arg USART_FLAG_TC:   Transmission Complete flag.
  *            @arg USART_FLAG_RXNE: Receive data register not empty flag.
  *
  * @note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (OverRun
  *          error) and IDLE (Idle line detected) flags are cleared by software
  *          sequence: a read operation to USART_SR register (USART_GetFlagStatus())
  *          followed by a read operation to USART_DR register (USART_ReceiveData()).
  * @note   RXNE flag can be also cleared by a read to the USART_DR register
  *          (USART_ReceiveData()).
  * @note   TC flag can be also cleared by software sequence: a read operation to
  *          USART_SR register (USART_GetFlagStatus()) followed by a write operation
  *          to USART_DR register (USART_SendData()).
  * @note   TXE flag is cleared only by a write to the USART_DR register
  *          (USART_SendData()).
  *
  * @retval None
  */
static inline void usart_clear_flag(USART_TypeDef* USARTx, uint16_t flag)
{

  USARTx->SR = (uint16_t)~flag;
}
/*----------------------------------------------------------*/
/**
  * @brief  Checks whether the specified USART interrupt has occurred or not.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_IT: specifies the USART interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg USART_IT_CTS    : CTS change interrupt (not available for UART4 and UART5)
  *            @arg USART_IT_LBD    : LIN Break detection interrupt
  *            @arg USART_IT_TXE    : Transmit Data Register empty interrupt
  *            @arg USART_IT_TC     : Transmission complete interrupt
  *            @arg USART_IT_RXNE   : Receive Data register not empty interrupt
  *            @arg USART_IT_IDLE   : Idle line detection interrupt
  *            @arg USART_IT_ORE_RX : OverRun Error interrupt if the RXNEIE bit is set
  *            @arg USART_IT_ORE_ER : OverRun Error interrupt if the EIE bit is set
  *            @arg USART_IT_NE     : Noise Error interrupt
  *            @arg USART_IT_FE     : Framing Error interrupt
  *            @arg USART_IT_PE     : Parity Error interrupt
  * @retval The new state of USART_IT (SET or RESET).
  */
static inline bool usart_get_it_status(USART_TypeDef* USARTx, uint16_t usart_it)
{
  uint32_t bitpos = 0x00, itmask = 0x00, usartreg = 0x00;


  /* Get the USART register index */
  usartreg = (((uint8_t)usart_it) >> 0x05);
  /* Get the interrupt position */
  itmask = usart_it & IT_MASK;
  itmask = (uint32_t)0x01 << itmask;

  if (usartreg == 0x01) /* The IT  is in CR1 register */
  {
    itmask &= USARTx->CR1;
  }
  else if (usartreg == 0x02) /* The IT  is in CR2 register */
  {
    itmask &= USARTx->CR2;
  }
  else /* The IT  is in CR3 register */
  {
    itmask &= USARTx->CR3;
  }

  bitpos = usart_it >> 0x08;
  bitpos = (uint32_t)0x01 << bitpos;
  bitpos &= USARTx->SR;
  return ((itmask != (uint16_t)0)&&(bitpos != (uint16_t)0));
}
/*----------------------------------------------------------*/
/**
  * @brief  Clears the USARTx's interrupt pending bits.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  USART_IT: specifies the interrupt pending bit to clear.
  *          This parameter can be one of the following values:
  *            @arg USART_IT_CTS:  CTS change interrupt (not available for UART4 and UART5)
  *            @arg USART_IT_LBD:  LIN Break detection interrupt
  *            @arg USART_IT_TC:   Transmission complete interrupt.
  *            @arg USART_IT_RXNE: Receive Data register not empty interrupt.
  *
  * @note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (OverRun
  *          error) and IDLE (Idle line detected) pending bits are cleared by
  *          software sequence: a read operation to USART_SR register
  *          (USART_GetITStatus()) followed by a read operation to USART_DR register
  *          (USART_ReceiveData()).
  * @note   RXNE pending bit can be also cleared by a read to the USART_DR register
  *          (USART_ReceiveData()).
  * @note   TC pending bit can be also cleared by software sequence: a read
  *          operation to USART_SR register (USART_GetITStatus()) followed by a write
  *          operation to USART_DR register (USART_SendData()).
  * @note   TXE pending bit is cleared only by a write to the USART_DR register
  *          (USART_SendData()).
  *
  * @retval None
  */
static inline void usart_clear_it_pending_bit(USART_TypeDef* USARTx, uint16_t usart_it)
{
  uint16_t bitpos = 0x00, itmask = 0x00;

  bitpos = usart_it >> 0x08;
  itmask = ((uint16_t)0x01 << (uint16_t)bitpos);
  USARTx->SR = (uint16_t)~itmask;
}

/*----------------------------------------------------------*/
#undef CR1_CLEAR_MASK
#undef CR3_CLEAR_MASK
#undef CR2_CLOCK_CLEAR_MASK
#undef IT_MASK
/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------------------*/
#endif /* STM32USART_H_ */
/*----------------------------------------------------------*/
