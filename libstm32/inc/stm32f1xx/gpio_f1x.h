/*
 * gpio_f1x.h
 *
 *  Created on: 16-07-2012
 *      Author: lucck
 */

#ifndef GPIO_F1X_H_
#define GPIO_F1X_H_
/*----------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "stm32lib.h"
/*----------------------------------------------------------*/
#ifdef __cplusplus
namespace stm32 {
#endif
/*----------------------------------------------------------*/

/** GPIO bits macros */
enum e_gpio_mode
{
	//! GPIO mode input
	GPIO_MODE_INPUT = 0,
	//! GPIO mode out 10MHZ
	GPIO_MODE_10MHZ = 1,
	//! GPIO mode out 2MHZ
	GPIO_MODE_2MHZ = 2,
	//! GPIO mode out 50MHZ
	GPIO_MODE_50MHZ = 3
};

enum e_gpio_ocnf
{
	//! GPIO mode PUSH-PULL
	GPIO_CNF_GPIO_PP = 0,
	//! GPIO mode Open Drain
	GPIO_CNF_GPIO_OD = 1,
	//! Alternate function PUSH-PULL
	GPIO_CNF_ALT_PP = 2,
	//! Alternate function Open Drain
	GPIO_CNF_ALT_OD = 3
};

enum e_gpio_icnf
{
	//! Configuration IN analog mode
	GPIO_CNF_IN_ANALOG = 0,
	//! Configuration IN float mode
	GPIO_CNF_IN_FLOAT = 1,
	//! Configuration IN pullup mode
	GPIO_CNF_IN_PULLUP = 2
};
/*----------------------------------------------------------*/
//! Set GPIO bit macro
static inline void gpio_set(GPIO_TypeDef* port , unsigned bit)
{
	port->BSRR = 1<<bit;
}
/*----------------------------------------------------------*/
//! Clear GPIO bit macro
static inline void gpio_clr(GPIO_TypeDef* port , unsigned bit)
{
	port->BRR = 1<<bit;
}
/*----------------------------------------------------------*/
//! Set by the mask
static inline void gpio_set_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	port->BSRR = bitmask;
}
/*----------------------------------------------------------*/
//! Clear GPIO bit mask
static inline void gpio_clr_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	port->BRR = bitmask;
}
/*----------------------------------------------------------*/
//! set clr in one op
static inline void gpio_set_clr_mask(GPIO_TypeDef* port , uint16_t enflags, uint16_t mask)
{
	port->BSRR = (uint32_t)(enflags & mask) | ((uint32_t)( ~enflags & mask)<<16);
}
/*----------------------------------------------------------*/
//! Get GPIO bit macro
//#define io_get(PORT,BIT) (((PORT)->IDR & (1<<(BIT)))?1:0)
static inline bool gpio_get(GPIO_TypeDef* port , unsigned bit)
{
	return (port->IDR >> (bit))&1;
}
/*----------------------------------------------------------*/
//! Get GPIO bit mask
static inline uint16_t gpio_get_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	return port->IDR & bitmask;
}
/*----------------------------------------------------------*/
/**
 * 	Configure GPIO line into selected mode
 *	@param[in] port Port to configure
 *	@param[in] bit  Pin bit number
 *	@param[in] mode New port mode
 *	@param[in] config New port configuration
 */
static inline void gpio_config(GPIO_TypeDef* port,uint8_t bit,uint32_t mode,uint32_t config)
{
	if(bit>7)
	{
		port->CRH &= ~(0xf<<(4*(bit-8)));
		port->CRH |= (mode|(config<<2)) << (4*(bit-8));
	}
	else
	{
		port->CRL &= ~(0xf<<(4*bit));
		port->CRL |= (mode|(config<<2)) << (4*bit);
	}
}
/*----------------------------------------------------------*/
enum gpio_port_source
{
	GPIO_PortSourceGPIOA   =    0x00,
	GPIO_PortSourceGPIOB   =    0x01,
	GPIO_PortSourceGPIOC   =    0x02,
	GPIO_PortSourceGPIOD   =    0x03,
	GPIO_PortSourceGPIOE   =    0x04,
	GPIO_PortSourceGPIOF   =    0x05,
	GPIO_PortSourceGPIOG   =    0x06
};
enum gpio_pin_source
{
	GPIO_PinSource0     =       0x00,
	GPIO_PinSource1     =       0x01,
	GPIO_PinSource2     =       0x02,
	GPIO_PinSource3     =       0x03,
	GPIO_PinSource4     =       0x04,
	GPIO_PinSource5     =       0x05,
	GPIO_PinSource6     =       0x06,
	GPIO_PinSource7     =       0x07,
	GPIO_PinSource8     =       0x08,
	GPIO_PinSource9     =       0x09,
	GPIO_PinSource10    =       0x0A,
	GPIO_PinSource11    =       0x0B,
	GPIO_PinSource12    =       0x0C,
	GPIO_PinSource13    =       0x0D,
	GPIO_PinSource14    =       0x0E,
	GPIO_PinSource15    =       0x0F
};
/*----------------------------------------------------------*/
/**
  * @brief  Selects the GPIO pin used as EXTI Line.
  * @param  GPIO_PortSource: selects the GPIO port to be used as source for EXTI lines.
  *   This parameter can be GPIO_PortSourceGPIOx where x can be (A..G).
  * @param  GPIO_PinSource: specifies the EXTI line to be configured.
  *   This parameter can be GPIO_PinSourcex where x can be (0..15).
  * @retval None
  */
static inline void gpio_exti_line_config(uint8_t GPIO_PortSource, uint8_t GPIO_PinSource)
{
  uint32_t tmp = 0x00;
  tmp = ((uint32_t)0x0F) << (0x04 * (GPIO_PinSource & (uint8_t)0x03));
  AFIO->EXTICR[GPIO_PinSource >> 0x02] &= ~tmp;
  AFIO->EXTICR[GPIO_PinSource >> 0x02] |= (((uint32_t)GPIO_PortSource) << (0x04 * (GPIO_PinSource & (uint8_t)0x03)));
}
/*----------------------------------------------------------*/
/**
 * 	Configure GPIO lines into selected mode
 *	@param[in] port Port to configure
 *	@param[in] bitmask Pin bitmap to configure
 *	@param[in] mode New port mode
 *	@param[in] config New port configuration
 */
static inline void gpio_config_ext(GPIO_TypeDef* port, uint16_t bit, uint32_t mode, uint32_t config)
{
	for(unsigned i=0; i<16; i++)
	{
		if(bit & (1<<i))
		{
			gpio_config(port,i,mode,config);
		}
	}
}
/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------------------*/
#endif /* GPIO_F1X_H_ */
/*----------------------------------------------------------*/
