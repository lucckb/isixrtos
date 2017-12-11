/*
 * stm32usart.h
 *
 *  Created on: 23 lis 2013
 *      Author: lucck
 */

#ifndef STM32USART_H_
#define STM32USART_H_

#include <stdint.h>
#include <stdbool.h>
#include <stm32rcc.h>
#include "stm32lib.h"


#define CR1_CLEAR_MASK            ((stm32_usart_reg_t)(USART_CR1_M | USART_CR1_PCE | \
                                              USART_CR1_PS | USART_CR1_TE | \
                                              USART_CR1_RE))


/*!< USART CR2 register clock bits clear Mask ((~(uint16_t)0xF0FF)) */
#define CR2_CLOCK_CLEAR_MASK      ((stm32_usart_reg_t)(USART_CR2_CLKEN | USART_CR2_CPOL | \
                                              USART_CR2_CPHA | USART_CR2_LBCL))

/*!< USART CR3 register clear Mask ((~(uint16_t)0xFCFF)) */
#define CR3_CLEAR_MASK            ((stm32_usart_reg_t)(USART_CR3_RTSE | USART_CR3_CTSE))


/*!< USART Interrupts mask */
#if USART_HARDWARE_VERSION == 2
#define IT_MASK                   ((stm32_usart_reg_t)0x000000FF)
#else
#define IT_MASK                   ((stm32_usart_reg_t)0x001F)
#endif


#ifdef __cplusplus
namespace stm32 {
#endif


static inline void usart_set_baudrate( USART_TypeDef* USARTx, 
		uint32_t baudrate, unsigned pclk1, unsigned pclk2 )
{  
  uint32_t tmpreg = 0x00, apbclock = 0x00;

#if USART_HARDWARE_VERSION == 2
  uint32_t divider = 0;
#else
  uint32_t integerdivider = 0x00;
  uint32_t fractionaldivider = 0x00;
#endif
#ifdef USART6
  if ((USARTx == USART1) || (USARTx == USART6))
#else
  if( USARTx == USART1 )
#endif
  {
	  /* USART1 Clock is PCLK1 instead of PCLK2 (limitation described in the 
       STM32F302/01/34 x4/x6/x8 respective erratasheets) */
#if defined(STM32F303x8) || defined(STM32F334x8) || defined(STM32F301x8) || defined(STM32F302x8)
	apbclock = pclk1;
	(void)pclk2;
#else
    apbclock = pclk2;
#endif
  }
  else
  {
    apbclock = pclk1;
  }
  //TODO: Fix with system clock engine list
#if defined( RCC_CFGR3_USART1SW )
 if( USARTx == USART1 ) {
	switch( RCC->CFGR3 & 3 ) {
		case 2: apbclock = 32768LU; break;
		case 3: apbclock = 8000000LU; break;
	}
 }
#endif
#if defined( RCC_CFGR3_USART2SW )
 if( USARTx == USART2 ) {
	switch( (RCC->CFGR3>>16) & 3 ) {
		case 2: apbclock = 32768LU; break;
		case 3: apbclock = 8000000LU; break;
	}
 }
#endif
#if defined( RCC_CFGR3_USART3SW )
 if( USARTx == USART3 ) {
	switch( (RCC->CFGR3>>18) & 3 ) {
		case 2: apbclock = 32768LU; break;
		case 3: apbclock = 8000000LU; break;
	}
 }
#endif

#if USART_HARDWARE_VERSION == 2
 /* Determine the integer part */
  if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
  {
    /* (divider * 10) computing in case Oversampling mode is 8 Samples */
    divider = (uint32_t)((2 * apbclock) / (baudrate));
    tmpreg  = (uint32_t)((2 * apbclock) % (baudrate));
  }
  else /* if ((USARTx->CR1 & CR1_OVER8_Set) == 0) */
  {
    /* (divider * 10) computing in case Oversampling mode is 16 Samples */
    divider = (uint32_t)((apbclock) / (baudrate));
    tmpreg  = (uint32_t)((apbclock) % (baudrate));
  }
  
  /* round the divider : if fractional part i greater than 0.5 increment divider */
  if (tmpreg >=  (baudrate) / 2)
  {
    divider++;
  } 
  
  /* Implement the divider in case Oversampling mode is 8 Samples */
  if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
  {
    /* get the LSB of divider and shift it to the right by 1 bit */
    tmpreg = (divider & (uint16_t)0x000F) >> 1;
    
    /* update the divider value */
    divider = (divider & (uint16_t)0xFFF0) | tmpreg;
  }
  /* Write to USART BRR */
  USARTx->BRR = (uint16_t)divider;
#else
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
#endif

}

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


/* Disable USART */
  USARTx->CR1 &= (uint32_t)~((uint32_t)USART_CR1_UE);

  uint32_t tmpreg = USARTx->CR2;

  /* Clear STOP[13:12] bits */
  tmpreg &= (uint32_t)~((uint32_t)USART_CR2_STOP);

  /* Configure the USART Stop Bits, Clock, CPOL, CPHA and LastBit :
      Set STOP[13:12] bits according to USART_StopBits value */
  tmpreg |= (uint32_t)stop_bits;

  /* Write to USART CR2 */
  USARTx->CR2 = tmpreg;


  tmpreg = USARTx->CR1;

  /* Clear M, PCE, PS, TE and RE bits */
  tmpreg &= (uint32_t)~((uint32_t)CR1_CLEAR_MASK);

  /* Configure the USART Word Length, Parity and mode:
     Set the M bits according to USART_WordLength value
     Set PCE and PS bits according to USART_Parity value
     Set TE and RE bits according to USART_Mode value */
  tmpreg |= (uint32_t)word_length | parity | mode;

  /* Write to USART CR1 */
  USARTx->CR1 = tmpreg;


  tmpreg = USARTx->CR3;

  /* Clear CTSE and RTSE bits */
  tmpreg &= (uint32_t)~((uint32_t)CR3_CLEAR_MASK);

  /* Configure the USART HFC :
      Set CTSE and RTSE bits according to USART_HardwareFlowControl value */
  tmpreg |= hw_flow_ctl;

  /* Write to USART CR3 */
  USARTx->CR3 = tmpreg;

  usart_set_baudrate( USARTx, baudrate, pclk1, pclk2 );
}


/**
  * @brief  Initializes the USARTx peripheral Clock according to the
  *         specified parameters in the USART_ClockInitStruct .
  * @param  USARTx: where x can be 1, 2, 3 or 6 to select the USART peripheral.
  * @param  USART_ClockInitStruct: pointer to a USART_ClockInitTypeDef structure that
  *         contains the configuration information for the specified  USART peripheral.
  * @note   The Smart Card and Synchronous modes are not available for UART4 and UART5.
  * @retval None
  */
static inline void usart_clock_init(USART_TypeDef* USARTx, uint32_t clock, uint32_t cpol,
		uint32_t cpha, uint32_t last_bit )
{
  uint32_t tmpreg = 0x00;


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
  USARTx->CR2 = (stm32_usart_reg_t)tmpreg;
}

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
    USARTx->CR1 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR1_UE);
  }
}

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

#if USART_HARDWARE_VERSION==2
/**
  * @brief  Enables or disables the USART's transmitter or receiver.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_Direction: specifies the USART direction.
  *          This parameter can be any combination of the following values:
  *            @arg USART_Mode_Tx: USART Transmitter
  *            @arg USART_Mode_Rx: USART Receiver
  * @param  NewState: new state of the USART transfer direction.
  *          This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
static inline void usart_direction_mode_cmd(USART_TypeDef* USARTx, 
		uint32_t USART_DirectionMode, bool enable )
{

  if ( enable ) {
    /* Enable the USART's transfer interface by setting the TE and/or RE bits 
       in the USART CR1 register */
    USARTx->CR1 |= USART_DirectionMode;
  } else {
    /* Disable the USART's transfer interface by clearing the TE and/or RE bits
       in the USART CR3 register */
    USARTx->CR1 &= (uint32_t)~USART_DirectionMode;
  }
}
#endif


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
    USARTx->CR1 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR1_OVER8);
  }
}

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
    USARTx->CR3 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR3_ONEBIT);
  }
}


#if USART_HARDWARE_VERSION==2
/**
  * @brief  Enables or disables the USART's most significant bit first 
  *         transmitted/received following the start bit.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  NewState: new state of the USART most significant bit first
  *         transmitted/received following the start bit.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   This function has to be called before calling USART_Cmd() function.
  * @retval None
  */
static inline void usart_msb_first_cmd(USART_TypeDef* USARTx, bool enable )
{
  
  if ( enable )
  {
    /* Enable the most significant bit first transmitted/received following the 
       start bit by setting the MSBFIRST bit in the CR2 register */
    USARTx->CR2 |= USART_CR2_MSBFIRST;
  }
  else
  {
    /* Disable the most significant bit first transmitted/received following the 
       start bit by clearing the MSBFIRST bit in the CR2 register */
    USARTx->CR2 &= (uint32_t)~((uint32_t)USART_CR2_MSBFIRST);
  }
}

/**
  * @brief  Enables or disables the binary data inversion.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  NewState: new defined levels for the USART data.
  *          This parameter can be:
  *            @arg ENABLE: Logical data from the data register are send/received in negative
  *                          logic (1=L, 0=H). The parity bit is also inverted.
  *            @arg DISABLE: Logical data from the data register are send/received in positive
  *                          logic (1=H, 0=L) 
  * @note   This function has to be called before calling USART_Cmd() function.
  * @retval None
  */
static inline void usart_data_inv_cmd(USART_TypeDef* USARTx, bool enabled )
{
  if ( enabled )
  {
    /* Enable the binary data inversion feature by setting the DATAINV bit in 
       the CR2 register */
    USARTx->CR2 |= USART_CR2_DATAINV;
  }
  else
  {
    /* Disable the binary data inversion feature by clearing the DATAINV bit in 
       the CR2 register */
    USARTx->CR2 &= (uint32_t)~((uint32_t)USART_CR2_DATAINV);
  }
}

/**
  * @brief  Enables or disables the Pin(s) active level inversion.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_InvPin: specifies the USART pin(s) to invert.
  *          This parameter can be any combination of the following values:
  *            @arg USART_InvPin_Tx: USART Tx pin active level inversion.
  *            @arg USART_InvPin_Rx: USART Rx pin active level inversion.
  * @param  NewState: new active level status for the USART pin(s).
  *          This parameter can be:
  *            @arg ENABLE: pin(s) signal values are inverted (Vdd =0, Gnd =1).
  *            @arg DISABLE: pin(s) signal works using the standard logic levels (Vdd =1, Gnd =0).
  * @note   This function has to be called before calling USART_Cmd() function.  
  * @retval None
  */
static inline void usart_inv_pin_cmd(USART_TypeDef* USARTx, 
		uint32_t USART_InvPin, bool enabled )
{
  if ( enabled )
  {
    /* Enable the active level inversion for selected pins by setting the TXINV 
       and/or RXINV bits in the USART CR2 register */
    USARTx->CR2 |= USART_InvPin;
  }
  else
  {
    /* Disable the active level inversion for selected requests by clearing the 
       TXINV and/or RXINV bits in the USART CR2 register */
    USARTx->CR2 &= (uint32_t)~USART_InvPin;
  }
}

/**
  * @brief  Enables or disables the swap Tx/Rx pins.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  NewState: new state of the USARTx TX/RX pins pinout.
  *          This parameter can be:
  *            @arg ENABLE: The TX and RX pins functions are swapped.
  *            @arg DISABLE: TX/RX pins are used as defined in standard pinout
  * @note   This function has to be called before calling USART_Cmd() function.
  * @retval None
  */
static inline void usart_swap_pin_cmd(USART_TypeDef* USARTx, bool enabled )
{
  if (enabled)
  {
    /* Enable the SWAP feature by setting the SWAP bit in the CR2 register */
    USARTx->CR2 |= USART_CR2_SWAP;
  }
  else
  {
    /* Disable the SWAP feature by clearing the SWAP bit in the CR2 register */
    USARTx->CR2 &= (uint32_t)~((uint32_t)USART_CR2_SWAP);
  }
}

/**
  * @brief  Enables or disables the receiver Time Out feature.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  NewState: new state of the USARTx receiver Time Out.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_receiver_timeout_cmd(USART_TypeDef* USARTx, bool enabled )
{
  if ( enabled )
  {
    /* Enable the receiver time out feature by setting the RTOEN bit in the CR2 
       register */
    USARTx->CR2 |= USART_CR2_RTOEN;
  }
  else
  {
    /* Disable the receiver time out feature by clearing the RTOEN bit in the CR2 
       register */
    USARTx->CR2 &= (uint32_t)~((uint32_t)USART_CR2_RTOEN);
  }
}

/**
  * @brief  Sets the receiver Time Out value.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_ReceiverTimeOut: specifies the Receiver Time Out value.
  * @retval None
  */
static inline void usart_set_receiver_timeout(USART_TypeDef* USARTx, uint32_t USART_ReceiverTimeOut)
{    
  /* Clear the receiver Time Out value by clearing the RTO[23:0] bits in the RTOR
     register  */
  USARTx->RTOR &= (uint32_t)~((uint32_t)USART_RTOR_RTO);
  /* Set the receiver Time Out value by setting the RTO[23:0] bits in the RTOR
     register  */
  USARTx->RTOR |= USART_ReceiverTimeOut;
}

/**
  * @brief  Enables or disables the specified USART peripheral in STOP Mode.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  NewState: new state of the USARTx peripheral state in stop mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   This function has to be called when USART clock is set to HSI or LSE.
  * @retval None
  */
#ifdef USART_CR1_UESM
static inline void usart_stop_mode_cmd(USART_TypeDef* USARTx, bool enabled )
{
  if ( enabled )
  {
    /* Enable the selected USART in STOP mode by setting the UESM bit in the CR1
       register */
    USARTx->CR1 |= USART_CR1_UESM;
  }
  else
  {
    /* Disable the selected USART in STOP mode by clearing the UE bit in the CR1
       register */
    USARTx->CR1 &= (uint32_t)~((uint32_t)USART_CR1_UESM);
  }
}
#endif

#ifdef USART_CR3_WUS
/**
  * @brief  Selects the USART WakeUp method form stop mode.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_WakeUp: specifies the selected USART wakeup method.
  *          This parameter can be one of the following values:
  *            @arg USART_WakeUpSource_AddressMatch: WUF active on address match.
  *            @arg USART_WakeUpSource_StartBit: WUF active on Start bit detection.
  *            @arg USART_WakeUpSource_RXNE: WUF active on RXNE.
  * @note   This function has to be called before calling USART_Cmd() function.  
  * @retval None
  */
static inline void usart_stop_mode_wakeup_source_config(USART_TypeDef* USARTx, uint32_t USART_WakeUpSource)
{
  USARTx->CR3 &= (uint32_t)~((uint32_t)USART_CR3_WUS);
  USARTx->CR3 |= USART_WakeUpSource;
}
#endif

#endif /* USART_HARDWARE_VERSION==2 */

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
#if USART_HARDWARE_VERSION==2
	USARTx->TDR = (data & (uint16_t)0x01FF);
#else
	USARTx->DR = (data & (uint16_t)0x01FF);
#endif
}

/**
  * @brief  Returns the most recent received data by the USARTx peripheral.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @retval The received data.
  */
static inline uint16_t usart_receive_data(USART_TypeDef* USARTx)
{
  /* Receive Data */
#if USART_HARDWARE_VERSION==2
	return (uint16_t)(USARTx->RDR & (uint16_t)0x01FF);
#else
	return (uint16_t)(USARTx->DR & (uint16_t)0x01FF);
#endif
}

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
  USARTx->CR2 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR2_ADD);
  /* Set the USART address node */
#if USART_HARDWARE_VERSION==2
  USARTx->CR2 |=((uint32_t)address << (uint32_t)0x18);
#else
  USARTx->CR2 |= address;
#endif
}


#if USART_HARDWARE_VERSION==2
/**
  * @brief  Enables or disables the USART's mute mode.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  NewState: new state of the USART mute mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void usart_mute_mode_cmd(USART_TypeDef* USARTx, bool enable )
{
  if (enable)
  {
    /* Enable the USART mute mode by setting the MME bit in the CR1 register */
    USARTx->CR1 |= USART_CR1_MME;
  }
  else
  {
    /* Disable the USART mute mode by clearing the MME bit in the CR1 register */
    USARTx->CR1 &= (uint32_t)~((uint32_t)USART_CR1_MME);
  }
}
static inline void usart_mute_mode_wakeup_config(USART_TypeDef* USARTx, uint32_t USART_WakeUp)
{
  USARTx->CR1 &= (uint32_t)~((uint32_t)USART_CR1_WAKE);
  USARTx->CR1 |= USART_WakeUp;
}

/**
  * @brief  Configure the the USART Address detection length.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_AddressLength: specifies the USART address length detection.
  *          This parameter can be one of the following values:
  *            @arg USART_AddressLength_4b: 4-bit address length detection 
  *            @arg USART_AddressLength_7b: 7-bit address length detection 
  * @retval None
  */
static inline void usart_address_detection_config(USART_TypeDef* USARTx, 
		uint32_t USART_AddressLength)
{
  USARTx->CR2 &= (uint32_t)~((uint32_t)USART_CR2_ADDM7);
  USARTx->CR2 |= USART_AddressLength;
}

#endif /* USART_HARDWARE_VERSION==2 */
/**
  * @brief  Determines if the USART is in mute mode or not.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @param  NewState: new state of the USART mute mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */

#if USART_HARDWARE_VERSION==1
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
#endif /* USART_HARDWARE_VERSION==1*/

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

  USARTx->CR2 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR2_LBDL);
  USARTx->CR2 |= length;
}

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
    USARTx->CR2 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR2_LINEN);
  }
}


/**
  * @brief  Transmits break characters.
  * @param  USARTx: where x can be 1, 2, 3, 4, 5 or 6 to select the USART or
  *         UART peripheral.
  * @retval None
  */

#if USART_HARDWARE_VERSION==1
static inline void usart_send_break(USART_TypeDef* USARTx)
{
  /* Send break characters */
  USARTx->CR1 |= USART_CR1_SBK;
}
#endif

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
    USARTx->CR3 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR3_HDSEL);
  }
}

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
    USARTx->CR3 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR3_SCEN);
  }
}

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
    USARTx->CR3 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR3_NACK);
  }
}

#if USART_HARDWARE_VERSION==2
/**
  * @brief  Sets the Smart Card number of retries in transmit and receive.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_AutoCount: specifies the Smart Card auto retry count.
  * @retval None
  */
static inline void usart_set_auto_retry_count(USART_TypeDef* USARTx, uint8_t USART_AutoCount)
{    
  /* Clear the USART auto retry count */
  USARTx->CR3 &= (uint32_t)~((uint32_t)USART_CR3_SCARCNT);
  /* Set the USART auto retry count*/
  USARTx->CR3 |= (uint32_t)((uint32_t)USART_AutoCount << 0x11);
}

/**
  * @brief  Sets the Smart Card Block length.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_BlockLength: specifies the Smart Card block length.
  * @retval None
  */
static inline void usart_set_block_length(USART_TypeDef* USARTx, uint8_t USART_BlockLength)
{    
  /* Clear the Smart card block length */
  USARTx->RTOR &= (uint32_t)~((uint32_t)USART_RTOR_BLEN);
  /* Set the Smart Card block length */
  USARTx->RTOR |= (uint32_t)((uint32_t)USART_BlockLength << 0x18);
}
#endif


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
  USARTx->CR3 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR3_IRLP);
  USARTx->CR3 |= USART_IrDAMode;
}


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
    USARTx->CR3 &= (stm32_usart_reg_t)~((stm32_usart_reg_t)USART_CR3_IREN);
  }
}


#if USART_HARDWARE_VERSION==2
//##### RS485 mode functions #####
/**
  * @brief  Enables or disables the USART's DE functionality.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  NewState: new state of the driver enable mode.
  *          This parameter can be: ENABLE or DISABLE.      
  * @retval None
  */
static inline void usart_de_cmd(USART_TypeDef* USARTx, bool enable )
{
  if ( enable )
  {
    /* Enable the DE functionality by setting the DEM bit in the CR3 register */
    USARTx->CR3 |= USART_CR3_DEM;
  }
  else
  {
    /* Disable the DE functionality by clearing the DEM bit in the CR3 register */
    USARTx->CR3 &= (uint32_t)~((uint32_t)USART_CR3_DEM);
  }
}
/**
  * @brief  Configures the USART's DE polarity
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_DEPolarity: specifies the DE polarity.
  *          This parameter can be one of the following values:
  *            @arg USART_DEPolarity_Low
  *            @arg USART_DEPolarity_High
  * @retval None
  */
static inline void usart_de_polarity_config(USART_TypeDef* USARTx, uint32_t USART_DEPolarity)
{
  USARTx->CR3 &= (uint32_t)~((uint32_t)USART_CR3_DEP);
  USARTx->CR3 |= USART_DEPolarity;
}

/**
  * @brief  Sets the specified RS485 DE assertion time
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_DEAssertionTime: specifies the time between the activation of
  *         the DE signal and the beginning of the start bit
  * @retval None
  */
static inline void usart_set_de_assertion_time(USART_TypeDef* USARTx, uint32_t USART_DEAssertionTime)
{
  /* Clear the DE assertion time */
  USARTx->CR1 &= (uint32_t)~((uint32_t)USART_CR1_DEAT);
  /* Set the new value for the DE assertion time */
  USARTx->CR1 |=((uint32_t)USART_DEAssertionTime << (uint32_t)0x15);
}

/**
  * @brief  Sets the specified RS485 DE deassertion time
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_DeassertionTime: specifies the time between the middle of the last 
  *         stop bit in a transmitted message and the de-activation of the DE signal
  * @retval None
  */
static inline void usart_set_ded_deassertion_time(USART_TypeDef* USARTx, uint32_t USART_DEDeassertionTime)
{
  /* Clear the DE deassertion time */
  USARTx->CR1 &= (uint32_t)~((uint32_t)USART_CR1_DEDT);
  /* Set the new value for the DE deassertion time */
  USARTx->CR1 |=((uint32_t)USART_DEDeassertionTime << (uint32_t)0x10);
}

#endif /* USART_HARDWARE_VERSION==2*/

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
    USARTx->CR3 &= (stm32_usart_reg_t)~USART_DMAReq;
  }
}


#if USART_HARDWARE_VERSION==2
/**
  * @brief  Enables or disables the USART's DMA interface when reception error occurs.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_DMAOnError: specifies the DMA status in case of reception error.
  *          This parameter can be any combination of the following values:
  *            @arg USART_DMAOnError_Enable: DMA receive request enabled when the USART DMA  
  *                                          reception error is asserted.
  *            @arg USART_DMAOnError_Disable: DMA receive request disabled when the USART DMA 
  *                                           reception error is asserted.
  * @retval None
  */
static inline void usart_dma_reception_error_config(USART_TypeDef* USARTx, uint32_t USART_DMAOnError)
{
  /* Clear the DMA Reception error detection bit */
  USARTx->CR3 &= (uint32_t)~((uint32_t)USART_CR3_DDRE);
  /* Set the new value for the DMA Reception error detection bit */
  USARTx->CR3 |= USART_DMAOnError;
}
#endif /* USART_HARDWARE_VERSION==2*/


#if USART_HARDWARE_VERSION==1
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
static inline void usart_it_config(USART_TypeDef* USARTx, uint32_t USART_IT, bool enabled )
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
#elif  USART_HARDWARE_VERSION==2
static inline void usart_it_config(USART_TypeDef* USARTx, uint32_t USART_IT, bool enable)
{
  uint32_t usartreg = 0, itpos = 0, itmask = 0;
  uint32_t usartxbase = 0;
  
  usartxbase = (uint32_t)USARTx;

  /* Get the USART register index */
  usartreg = (((uint16_t)USART_IT) >> 0x08);

  /* Get the interrupt position */
  itpos = USART_IT & IT_MASK;
  itmask = (((uint32_t)0x01) << itpos);

  if (usartreg == 0x02) /* The IT is in CR2 register */
  {
    usartxbase += 0x04;
  }
  else if (usartreg == 0x03) /* The IT is in CR3 register */
  {
    usartxbase += 0x08;
  }
  else /* The IT is in CR1 register */
  {
  }
  if (enable)
  {
    *(__IO uint32_t*)usartxbase  |= itmask;
  }
  else
  {
    *(__IO uint32_t*)usartxbase &= ~itmask;
  }
}
#else
#error Unknown uart revision
#endif



#if USART_HARDWARE_VERSION==2
/**
  * @brief  Enables the specified USART's Request.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_Request: specifies the USART request.
  *          This parameter can be any combination of the following values:
  *            @arg USART_Request_TXFRQ: Transmit data flush ReQuest
  *            @arg USART_Request_RXFRQ: Receive data flush ReQuest
  *            @arg USART_Request_MMRQ: Mute Mode ReQuest
  *            @arg USART_Request_SBKRQ: Send Break ReQuest
  *            @arg USART_Request_ABRRQ: Auto Baud Rate ReQuest
  * @param  NewState: new state of the DMA interface when reception error occurs.
  *          This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
static inline void usart_request_cmd(USART_TypeDef* USARTx, uint32_t USART_Request, bool enabled )
{
  if ( enabled)
  {
    /* Enable the USART ReQuest by setting the dedicated request bit in the RQR
       register.*/
    USARTx->RQR |= USART_Request;
  }
  else
  {
    /* Disable the USART ReQuest by clearing the dedicated request bit in the RQR
       register.*/
    USARTx->RQR &= (uint32_t)~USART_Request;
  }
}
/**
  * @brief  Enables or disables the USART's Overrun detection.
  * @param  USARTx: where x can be 1 or 2 or 3 to select the USART peripheral.
  * @param  USART_OVRDetection: specifies the OVR detection status in case of OVR error.
  *          This parameter can be any combination of the following values:
  *            @arg USART_OVRDetection_Enable: OVR error detection enabled when
  *                                            the USART OVR error is asserted.
  *            @arg USART_OVRDetection_Disable: OVR error detection disabled when
  *                                             the USART OVR error is asserted.
  * @retval None
  */
static inline void usart_overrun_detection_config(USART_TypeDef* USARTx, uint32_t USART_OVRDetection)
{
  /* Clear the OVR detection bit */
  USARTx->CR3 &= (uint32_t)~((uint32_t)USART_CR3_OVRDIS);
  /* Set the new value for the OVR detection bit */
  USARTx->CR3 |= USART_OVRDetection;
}

#endif /* USART_HARDWARE_VERSION==2*/
  

/**
  * @brief  Checks whether the specified USART flag is set or not.
  */
static inline bool usart_get_flag_status(USART_TypeDef* USARTx, uint32_t flag)
{

#if USART_HARDWARE_VERSION==2
	return (USARTx->ISR & flag) != 0;
#else
	return (USARTx->SR & flag) != 0;
#endif
}

/**
  * @brief  Clears the USARTx's pending flags.
  */
static inline void usart_clear_flag(USART_TypeDef* USARTx, uint32_t flag)
{
#if USART_HARDWARE_VERSION==2
  USARTx->ICR = flag;
#else
  USARTx->SR = (uint16_t)~flag;
#endif
}

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
static inline bool usart_get_it_status(USART_TypeDef* USARTx, uint32_t usart_it)
{
  uint32_t bitpos = 0x00, itmask = 0x00, usartreg = 0x00;


  /* Get the USART register index */

#if USART_HARDWARE_VERSION==2
  usartreg = (((uint16_t)usart_it) >> 0x08);
#else
  usartreg = (((uint8_t)usart_it) >> 0x05);
#endif
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

#if USART_HARDWARE_VERSION==2
  bitpos = usart_it >> 0x10;
#else
  bitpos = usart_it >> 0x08;
#endif
  bitpos = (uint32_t)0x01 << bitpos;

#if USART_HARDWARE_VERSION==2
  bitpos &= USARTx->ISR;
#else
  bitpos &= USARTx->SR;
#endif
  return ((itmask != (uint16_t)0)&&(bitpos != (uint16_t)0));
}

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
  stm32_usart_reg_t bitpos = 0x00, itmask = 0x00;
#if USART_HARDWARE_VERSION==2
  bitpos = usart_it >> 0x10;
#else
  bitpos = usart_it >> 0x08;
#endif
  itmask = ((stm32_usart_reg_t)0x01 << (stm32_usart_reg_t)bitpos);
#if USART_HARDWARE_VERSION==2
  USARTx->ICR = (uint32_t)itmask;
#else
  USARTx->SR = (uint16_t)~itmask;
#endif
}


#undef CR1_CLEAR_MASK
#undef CR3_CLEAR_MASK
#undef CR2_CLOCK_CLEAR_MASK
#undef IT_MASK

#ifdef __cplusplus
}
#endif

#endif /* STM32USART_H_ */

