/*
 * gpio_f1x.h
 *
 *  Created on: 16-07-2012
 *      Author: lucck
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "stm32lib.h"
//Legacy alternate functions
#if defined(STM32MCU_MAJOR_TYPE_F2) || defined(STM32MCU_MAJOR_TYPE_F4)
#include <stm32f4x/stm32f4x_gpio.h>
#endif

#ifdef __cplusplus
namespace stm32 {
#endif

/** GPIO bits macros */
enum e_gpio_mode {
	//! GPIO mode input
	GPIO_MODE_INPUT 	= 0,
	//! GPIO mode output
	GPIO_MODE_OUTPUT 	= 1,
	//! GPIO alternate mode
	GPIO_MODE_ALTERNATE = 2,
	//! GPIO mode out 50MHZ
	GPIO_MODE_ANALOG 	= 3
};


/*** GPIO type ***/
enum e_gpio_otype {
	//! Output type push pull
	GPIO_OTYPE_PP = 0,
	//! Output type open drain
	GPIO_OTYPE_OD = 1
};


/** New recommended speed enumeration */
enum e_lvl_gpio_ospeed {
	GPIO_SPEED_LEVEL_0,
	GPIO_SPEED_LEVEL_1,
	GPIO_SPEED_LEVEL_2,
	GPIO_SPEED_LEVEL_3,
};

/** Old GPIO speed enumeration */
#if defined(STM32MCU_MAJOR_TYPE_F2) || \
	defined(STM32MCU_MAJOR_TYPE_F4) || \
	defined(STM32MCU_MAJOR_TYPE_F7)

enum e_gpio_ospeed {
	//! GPIO port speed 2MHz
	GPIO_SPEED_LOW =	GPIO_SPEED_LEVEL_0,
	//! GPIO port speed 25MHz
	GPIO_SPEED_MED =	GPIO_SPEED_LEVEL_1,
	//! GPIO port speed 50MHz
	GPIO_SPEED_FAST =	GPIO_SPEED_LEVEL_2,
	//! GPIO port speed 100MHz
	GPIO_SPEED_HI =		GPIO_SPEED_LEVEL_3
};
#elif defined(STM32MCU_MAJOR_TYPE_F37) || defined(STM32MCU_MAJOR_TYPE_F3)
enum e_gpio_ospeed {
	//! GPIO port speed 2MHz
	GPIO_SPEED_LOW =	GPIO_SPEED_LEVEL_2,
	//! GPIO port speed 10M
	GPIO_SPEED_MED =	GPIO_SPEED_LEVEL_1,
	//! GPIO port speed 10M
	GPIO_SPEED_FAST =	GPIO_SPEED_LEVEL_1,
	//! GPIO port speed 50M
	GPIO_SPEED_HI =		GPIO_SPEED_LEVEL_3
};
#endif


enum e_gpio_pullup {
	//! No pullup
	GPIO_PUPD_NONE		= 0,
	//!Pull up config
	GPIO_PUPD_PULLUP	= 1,
	//! Pull down config
	GPIO_PUPD_PULLDOWN	= 2
};

enum gpio_port_source {
	GPIO_PortSourceGPIOA   =    0x00,
	GPIO_PortSourceGPIOB   =    0x01,
	GPIO_PortSourceGPIOC   =    0x02,
	GPIO_PortSourceGPIOD   =    0x03,
	GPIO_PortSourceGPIOE   =    0x04,
	GPIO_PortSourceGPIOF   =    0x05,
	GPIO_PortSourceGPIOG   =    0x06,
	GPIO_PortSourceGPIOH   =    0x07,
	GPIO_PortSourceGPIOI   =    0x08
};

enum gpio_pin_source {
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


/** GPIO alternate function */
enum gpio_af
{
	GPIO_AF_0,
	GPIO_AF_1,
	GPIO_AF_2,
	GPIO_AF_3,
	GPIO_AF_4,
	GPIO_AF_5,
	GPIO_AF_6,
	GPIO_AF_7,
	GPIO_AF_8,
	GPIO_AF_9,
	GPIO_AF_10,
	GPIO_AF_11,
	GPIO_AF_12,
	GPIO_AF_13,
	GPIO_AF_14,
	GPIO_AF_15
};

//! Set GPIO bit macro
static inline void gpio_set(GPIO_TypeDef* port , unsigned bit)
{
#if defined( _GPIO_TYPEDEF_BSRR_DEFINED )
	port->BSRR |= ((1U<<bit)&0xffffU);
#else
	port->BSRRL = 1U<<bit;
#endif
}
//! Clear GPIO bit macro
static inline void gpio_clr(GPIO_TypeDef* port , unsigned bit)
{
#if defined( _GPIO_TYPEDEF_BSRR_DEFINED )
	port->BSRR |= (1U<<bit)<<16;
#else
	port->BSRRH = 1U<<bit;
#endif
}
//! Set by the mask
static inline void gpio_set_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
#ifdef _GPIO_TYPEDEF_BSRR_DEFINED
	port->BSRR = bitmask;
#else
	port->BSRRL = bitmask;
#endif
}
//! Clear GPIO bit mask
static inline void gpio_clr_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
#ifdef _GPIO_TYPEDEF_BSRR_DEFINED
	port->BSRR = ((uint32_t)bitmask)<<16;
#else
	port->BSRRH = bitmask;
#endif
}
//! set clr in one op
static inline void gpio_set_clr_mask(GPIO_TypeDef* port , uint16_t enflags, uint16_t mask)
{
#ifdef _GPIO_TYPEDEF_BSRR_DEFINED
	port->BSRR = (uint32_t)(enflags & mask) | ((uint32_t)( ~enflags & mask)<<16);
#else
	__IO uint32_t * const BSRR = (__IO uint32_t*)&port->BSRRL;
	*BSRR = (uint32_t)(enflags & mask) | ((uint32_t)( ~enflags & mask)<<16);
#endif
}

//! Get GPIO bit macro
//#define io_get(PORT,BIT) (((PORT)->IDR & (1<<(BIT)))?1:0)
static inline bool gpio_get(GPIO_TypeDef* port , unsigned bit)
{
	return (port->IDR >> (bit))&1;
}
//! Get GPIO bit mask
static inline uint16_t gpio_get_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	return port->IDR & bitmask;
}
/**
 * 	Configure GPIO line into selected mode
 *	@param[in] port Port to configure
 *	@param[in] bit  Pin bit number
 *	@param[in] mode Port mode
 *	@param[in] pullup Port pullup configuration
 *	@param[in] ospeed Port speed
 *	@param[in] otype Output port type
 */
static inline void gpio_config(GPIO_TypeDef* port, uint8_t bit , uint16_t mode, uint16_t pullup,
#ifdef __cplusplus
		uint16_t ospeed=0, uint16_t otype=0)
#else
		uint16_t ospeed ,uint16_t otype )
#endif

{
	static const uint32_t C_mode_mask = 0x03;
	static const uint32_t C_ospeed_mask = 0x03;
	static const uint16_t C_otype_mask = 0x01;
	static const uint32_t C_pupd_mask = 0x03;
	//Setup moder
	port->MODER &= ~(C_mode_mask << (2*bit));
	port->MODER |= ((uint32_t)mode & C_mode_mask) << (2*bit);
	if( mode == GPIO_MODE_OUTPUT || mode == GPIO_MODE_ALTERNATE )
	{
		//Setup output speed
		port->OSPEEDR &= ~(C_ospeed_mask << (2*bit));
		port->OSPEEDR |= ((uint32_t)ospeed & C_ospeed_mask) << (2*bit);
		//Setup otype
		port->OTYPER &= ~(C_otype_mask << bit);
		port->OTYPER |= (otype & C_ospeed_mask) << bit;
	}
	//Pull up pull down resistor configuration
	port->PUPDR &= ~(C_pupd_mask << (2*bit));
	port->PUPDR |= ((uint32_t)pullup & C_mode_mask) << (2*bit);
}




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
  SYSCFG->EXTICR[GPIO_PinSource >> 0x02] &= ~tmp;
  SYSCFG->EXTICR[GPIO_PinSource >> 0x02] |= (((uint32_t)GPIO_PortSource) << (0x04 * (GPIO_PinSource & (uint8_t)0x03)));
}

/**
 * 	Configure GPIO line into selected mode
 *	@param[in] port Port to configure
 *	@param[in] bit  Pin bit number
 *	@param[in] mode Port mode
 *	@param[in] pullup Port pullup configuration
 *	@param[in] ospeed Port speed
 *	@param[in] otype Output port type
 */
static inline void gpio_config_ext(GPIO_TypeDef* port, uint16_t bit, uint16_t mode, uint16_t pullup,
#ifdef __cplusplus
		uint16_t ospeed=0, uint16_t otype=0)
#else
		uint16_t ospeed ,uint16_t otype )
#endif
{
	for(unsigned i=0; i<16; i++)
	{
		if(bit & (1<<i))
		{
			gpio_config( port, i, mode, pullup, ospeed, otype );
		}
	}
}
/**
  * @brief  Locks GPIO Pins configuration registers.
  * @note   The locked registers are GPIOx_MODER, GPIOx_OTYPER, GPIOx_OSPEEDR,
  *         GPIOx_PUPDR, GPIOx_AFRL and GPIOx_AFRH.
  * @note   The configuration of the locked GPIO pins can no longer be modified
  *         until the next reset.
  * @param  GPIOx: where x can be (A..I) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bit to be locked.
  *          This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
 */
static inline void gpio_pin_lock_config(GPIO_TypeDef* port, uint16_t bit )
{
	__IO uint32_t tmp = 0x00010000 | (1<<bit);
	 /* Set LCKK bit */
	 port->LCKR = tmp;
	 /* Reset LCKK bit */
	 port->LCKR =  1<<bit;
	 /* Set LCKK bit */
	 port->LCKR = tmp;
	 /* Read LCKK bit*/
	 tmp = port->LCKR;
	 /* Read LCKK bit*/
	 tmp = port->LCKR;

}
/**
  * @brief  Changes the mapping of the specified pin.
  * @param  GPIOx: where x can be (A..I) to select the GPIO peripheral.
  * @param  GPIO_PinSource: specifies the pin for the Alternate function.
  *         This parameter can be GPIO_PinSourcex where x can be (0..15).
  * @param  GPIO_AFSelection: selects the pin to used as Alternate function.
  *          This parameter can be one of the following values:
  *            @arg GPIO_AF_RTC_50Hz: Connect RTC_50Hz pin to AF0 (default after reset)
  *            @arg GPIO_AF_MCO: Connect MCO pin (MCO1 and MCO2) to AF0 (default after reset)
  *            @arg GPIO_AF_TAMPER: Connect TAMPER pins (TAMPER_1 and TAMPER_2) to AF0 (default after reset)
  */
/** Setup alternate function for the pin
 * @param[in] GPIOx Gpio port
 * @param[in] GPIO_PinSource Pin number
 * @param[in] GPIO_AF Alternate number
 *
 * ***/
static inline void gpio_pin_AF_config(GPIO_TypeDef* GPIOx, uint16_t GPIO_PinSource, uint8_t GPIO_AF)
{

  uint32_t temp = ((uint32_t)(GPIO_AF) << ((uint32_t)((uint32_t)GPIO_PinSource & (uint32_t)0x07) * 4)) ;
  GPIOx->AFR[GPIO_PinSource >> 0x03] &= ~((uint32_t)0xF << ((uint32_t)((uint32_t)GPIO_PinSource & (uint32_t)0x07) * 4)) ;
  uint32_t temp_2 = GPIOx->AFR[GPIO_PinSource >> 0x03] | temp;
  GPIOx->AFR[GPIO_PinSource >> 0x03] = temp_2;
}


#ifdef __cplusplus
namespace _internal {
namespace gpio_v2 {
#endif
//Internal port to number conversion
static inline int _gpio_clock_port_to_number( GPIO_TypeDef* port )
{
#if defined(RCC_AHB1ENR_GPIOAEN)
	if		( port == GPIOA ) return RCC_AHB1ENR_GPIOAEN;
	else if ( port == GPIOB ) return RCC_AHB1ENR_GPIOBEN;
	else if ( port == GPIOC ) return RCC_AHB1ENR_GPIOCEN;
	else if ( port == GPIOD ) return RCC_AHB1ENR_GPIODEN;
	else if ( port == GPIOE ) return RCC_AHB1ENR_GPIOEEN;
	else if ( port == GPIOF ) return RCC_AHB1ENR_GPIOFEN;
	else if ( port == GPIOG ) return RCC_AHB1ENR_GPIOGEN;
	else if ( port == GPIOH ) return RCC_AHB1ENR_GPIOHEN;
	else if ( port == GPIOI ) return RCC_AHB1ENR_GPIOIEN;
	else return -1;
#elif defined(RCC_AHBENR_GPIOAEN)
	if		( port == GPIOA ) return RCC_AHBENR_GPIOAEN;
	else if ( port == GPIOB ) return RCC_AHBENR_GPIOBEN;
	else if ( port == GPIOC ) return RCC_AHBENR_GPIOCEN;
	else if ( port == GPIOD ) return RCC_AHBENR_GPIODEN;
	else if ( port == GPIOE ) return RCC_AHBENR_GPIOEEN;
	else if ( port == GPIOF ) return RCC_AHBENR_GPIOFEN;
	else return -1;
#endif
}
#ifdef __cplusplus
}}
#endif

/*** Enable or disable CLK for selected port
 * @param[in] port GPIO porrt
 * @param[in] enable Enable disable flag
 */
static inline void gpio_clock_enable( GPIO_TypeDef* port, bool enable )
{
#ifdef __cplusplus
	using namespace stm32;
	using namespace _internal::gpio_v2;
#endif
#if defined(RCC_AHB1ENR_GPIOAEN)
#define _ENR_REG_ RCC->AHB1ENR 
#elif defined(RCC_AHBENR_GPIOAEN)
#define _ENR_REG_ RCC->AHBENR 
#endif
	if(_gpio_clock_port_to_number( port ) < 0 )
		return;
	if( enable )
		_ENR_REG_ |=  _gpio_clock_port_to_number( port );
	else
		_ENR_REG_ &=  ~_gpio_clock_port_to_number( port );
#undef ENR_REG
}

/**
  * @brief  Selects the ETHERNET media interface
  * @param  SYSCFG_ETH_MediaInterface: specifies the Media Interface mode.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_ETH_MediaInterface_MII: MII mode selected
  *            @arg SYSCFG_ETH_MediaInterface_RMII: RMII mode selected
  * @retval None
  */

#if defined(PERIPH_BB_BASE)
enum gpio_media_interface_enum {
	GPIO_ETH_MediaInterface_MII = 0,
	GPIO_ETH_MediaInterface_RMII = 1
};
static inline void gpio_eth_media_interface_config( uint32_t media_ifc )
{

#   define SYSCFG_OFFSET             (SYSCFG_BASE - PERIPH_BASE)
#	define PMC_OFFSET                (SYSCFG_OFFSET + 0x04)
#	define MII_RMII_SEL_BitNumber    ((uint8_t)0x17) 
#	define PMC_MII_RMII_SEL_BB       (PERIPH_BB_BASE + (PMC_OFFSET * 32) + (MII_RMII_SEL_BitNumber * 4))
	  *(__IO uint32_t *) PMC_MII_RMII_SEL_BB = media_ifc;
#undef PMC_OFFSET
#undef MII_RMII_SEL_BitNumber
#undef PMC_MII_RMII_SEL_BB
#undef SYSCFG_OFFSET
}
#elif defined(SYSCFG_PMC_MII_RMII_SEL)
enum gpio_media_interface_enum {
	GPIO_ETH_MediaInterface_MII = 0,
	GPIO_ETH_MediaInterface_RMII = SYSCFG_PMC_MII_RMII_SEL
};

static inline void gpio_eth_media_interface_config( uint32_t media_ifc )
{
	SYSCFG->PMC &= ~(SYSCFG_PMC_MII_RMII_SEL);
	SYSCFG->PMC |= (uint32_t)media_ifc;
}
#endif


#ifdef __cplusplus
}
#endif
