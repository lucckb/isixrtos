/*
 * gpio_f1x.h
 *
 *  Created on: 16-07-2012
 *      Author: lucck
 */

#ifndef GPIO_F4X_H_
#define GPIO_F4X_H_
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
	GPIO_MODE_INPUT 	= 0,
	//! GPIO mode output
	GPIO_MODE_OUTPUT 	= 1,
	//! GPIO alternate mode
	GPIO_MODE_ALTERNATE = 2,
	//! GPIO mode out 50MHZ
	GPIO_MODE_ANALOG 	= 3
};


/*** GPIO type ***/
enum e_gpio_otype
{
	//! Output type push pull
	GPIO_OTYPE_PP = 0,
	//! Output type open drain
	GPIO_OTYPE_OD = 1
};

enum e_gpio_ospeed
{
	//! GPIO port speed 2MHz
	GPIO_SPEED_2MHZ = 0,
	//! GPIO port speed 25MHz
	GPIO_SPEED_25MHZ = 1,
	//! GPIO port speed 50MHz
	GPIO_SPEED_50MHZ = 2,
	//! GPIO port speed 100MHz
	GPIO_SPEED_100MHZ = 3
};

enum e_gpio_pullup
{
	//! No pullup
	GPIO_PUPD_NONE 		= 0,
	//!Pull up config
	GPIO_PUPD_PULLUP 	= 1,
	//! Pull down config
	GPIO_PUPD_PULLDOWN 	= 2
};

/*----------------------------------------------------------*/
//! Set GPIO bit macro
static inline void gpio_set(GPIO_TypeDef* port , unsigned bit)
{
	port->BSRRL = 1<<bit;
}
/*----------------------------------------------------------*/
//! Clear GPIO bit macro
static inline void gpio_clr(GPIO_TypeDef* port , unsigned bit)
{
	port->BSRRH = 1<<bit;
}
/*----------------------------------------------------------*/
//! Set by the mask
static inline void gpio_set_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	//Uggly hack for complette 32 bit reg
	port->BSRRL = bitmask;
}
/*----------------------------------------------------------*/
//! Clear GPIO bit mask
static inline void gpio_clr_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	port->BSRRH = bitmask;
}
/*----------------------------------------------------------*/
//! set clr in one op
static inline void gpio_set_clr_mask(GPIO_TypeDef* port , uint16_t enflags, uint16_t mask)
{

	__IO uint32_t * const BSRR = (__IO uint32_t*)&port->BSRRL;
	*BSRR = (uint32_t)(enflags & mask) | ((uint32_t)( ~enflags & mask)<<16);
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

/*----------------------------------------------------------*/
enum gpio_port_source
{
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
//GPIO alternate functions
enum { GPIO_AF_RTC_50Hz = 0   };  /* RTC_50Hz Alternate Function mapping */
enum { GPIO_AF_MCO      = 0   };  /* MCO (MCO1 and MCO2) Alternate Function mapping */
enum { GPIO_AF_TAMPER   = 0   };  /* TAMPER (TAMPER_1 and TAMPER_2) Alternate Function mapping */
enum { GPIO_AF_SWJ      = 0   };  /* SWJ (SWD and JTAG) Alternate Function mapping */
enum { GPIO_AF_TRACE    = 0   };  /* TRACE Alternate Function mapping */
/**
  * @brief   AF 1 selection
  */
enum { GPIO_AF_TIM1 =  0x01   };  /* TIM1 Alternate Function mapping */
enum { GPIO_AF_TIM2 =  0x01   };  /* TIM2 Alternate Function mapping */
/**
  * @brief   AF 2 selection
  */
enum { GPIO_AF_TIM3     = 0x02  };  /* TIM3 Alternate Function mapping */
enum { GPIO_AF_TIM4     = 0x02  };  /* TIM4 Alternate Function mapping */
enum { GPIO_AF_TIM5     = 0x02  };  /* TIM5 Alternate Function mapping */
/**
  * @brief   AF 3 selection
  */
enum { GPIO_AF_TIM8      =  0x03  };  /* TIM8 Alternate Function mapping */
enum { GPIO_AF_TIM9      =  0x03  };  /* TIM9 Alternate Function mapping */
enum { GPIO_AF_TIM10     =  0x03  };  /* TIM10 Alternate Function mapping */
enum { GPIO_AF_TIM11     =  0x03  };  /* TIM11 Alternate Function mapping */
/**
  * @brief   AF 4 selection
  */
enum {  GPIO_AF_I2C1     =  0x04  }; /* I2C1 Alternate Function mapping */
enum {  GPIO_AF_I2C2     =  0x04  }; /* I2C2 Alternate Function mapping */
enum {  GPIO_AF_I2C3     =  0x04  }; /* I2C3 Alternate Function mapping */
/**
  * @brief   AF 5 selection
  */
enum { GPIO_AF_SPI1    = 0x05  };  /* SPI1 Alternate Function mapping */
enum { GPIO_AF_SPI2    = 0x05  };  /* SPI2/I2S2 Alternate Function mapping */
/**
  * @brief   AF 6 selection
  */
enum { GPIO_AF_SPI3  = 6  };     /* SPI3/I2S3 Alternate Function mapping */
/**
  * @brief   AF 7 selection
  */
enum { GPIO_AF_USART1       = 0x07  };  /* USART1 Alternate Function mapping */
enum { GPIO_AF_USART2       = 0x07  };  /* USART2 Alternate Function mapping */
enum { GPIO_AF_USART3       = 0x07  };  /* USART3 Alternate Function mapping */
enum { GPIO_AF_I2S3ext      = 0x07  };  /* I2S3ext Alternate Function mapping */
/**
  * @brief   AF 8 selection
  */
enum { GPIO_AF_UART4     = 0x08 };  /* UART4 Alternate Function mapping */
enum { GPIO_AF_UART5     = 0x08 };  /* UART5 Alternate Function mapping */
enum { GPIO_AF_USART6    = 0x08 };  /* USART6 Alternate Function mapping */
/**
  * @brief   AF 9 selection
  */
enum { GPIO_AF_CAN1    =   0x09  };  /* CAN1 Alternate Function mapping */
enum { GPIO_AF_CAN2    =   0x09  };  /* CAN2 Alternate Function mapping */
enum { GPIO_AF_TIM12   =   0x09  };  /* TIM12 Alternate Function mapping */
enum { GPIO_AF_TIM13   =   0x09  };  /* TIM13 Alternate Function mapping */
enum { GPIO_AF_TIM14   =   0x09  };  /* TIM14 Alternate Function mapping */
/**
  * @brief   AF 10 selection
  */
enum {  GPIO_AF_OTG_FS    =  0xA };  /* OTG_FS Alternate Function mapping */
enum {  GPIO_AF_OTG_HS    =  0xA };  /* OTG_HS Alternate Function mapping */
enum {  GPIO_AF_OTG2_HS    =  0xA };  /* OTG_HS Alternate Function mapping */
/**
  * @brief   AF 11 selection
  */
enum { GPIO_AF_ETH       = 0x0B  }; /* ETHERNET Alternate Function mapping */
/**
  * @brief   AF 12 selection
  */
enum { GPIO_AF_FSMC         =  0xC  };  /* FSMC Alternate Function mapping */
enum { GPIO_AF_OTG_HS_FS    =  0xC  };  /* OTG HS configured in FS, Alternate Function mapping */
enum { GPIO_AF_SDIO         =  0xC  };  /* SDIO Alternate Function mapping */
/**
  * @brief   AF 13 selection
  */
enum { GPIO_AF_DCMI     =  0x0D  };  /* DCMI Alternate Function mapping */
/**
  * @brief   AF 15 selection
  */
enum {  GPIO_AF_EVENTOUT   =    0x0F };  /* EVENTOUT Alternate Function mapping */




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
  SYSCFG->EXTICR[GPIO_PinSource >> 0x02] &= ~tmp;
  SYSCFG->EXTICR[GPIO_PinSource >> 0x02] |= (((uint32_t)GPIO_PortSource) << (0x04 * (GPIO_PinSource & (uint8_t)0x03)));
}

/*----------------------------------------------------------*/
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
/*----------------------------------------------------------*/
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
/*----------------------------------------------------------*/
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
  *            @arg GPIO_AF_SWJ: Connect SWJ pins (SWD and JTAG)to AF0 (default after reset)
  *            @arg GPIO_AF_TRACE: Connect TRACE pins to AF0 (default after reset)
  *            @arg GPIO_AF_TIM1: Connect TIM1 pins to AF1
  *            @arg GPIO_AF_TIM2: Connect TIM2 pins to AF1
  *            @arg GPIO_AF_TIM3: Connect TIM3 pins to AF2
  *            @arg GPIO_AF_TIM4: Connect TIM4 pins to AF2
  *            @arg GPIO_AF_TIM5: Connect TIM5 pins to AF2
  *            @arg GPIO_AF_TIM8: Connect TIM8 pins to AF3
  *            @arg GPIO_AF_TIM9: Connect TIM9 pins to AF3
  *            @arg GPIO_AF_TIM10: Connect TIM10 pins to AF3
  *            @arg GPIO_AF_TIM11: Connect TIM11 pins to AF3
  *            @arg GPIO_AF_I2C1: Connect I2C1 pins to AF4
  *            @arg GPIO_AF_I2C2: Connect I2C2 pins to AF4
  *            @arg GPIO_AF_I2C3: Connect I2C3 pins to AF4
  *            @arg GPIO_AF_SPI1: Connect SPI1 pins to AF5
  *            @arg GPIO_AF_SPI2: Connect SPI2/I2S2 pins to AF5
  *            @arg GPIO_AF_SPI3: Connect SPI3/I2S3 pins to AF6
  *            @arg GPIO_AF_I2S3ext: Connect I2S3ext pins to AF7
  *            @arg GPIO_AF_USART1: Connect USART1 pins to AF7
  *            @arg GPIO_AF_USART2: Connect USART2 pins to AF7
  *            @arg GPIO_AF_USART3: Connect USART3 pins to AF7
  *            @arg GPIO_AF_UART4: Connect UART4 pins to AF8
  *            @arg GPIO_AF_UART5: Connect UART5 pins to AF8
  *            @arg GPIO_AF_USART6: Connect USART6 pins to AF8
  *            @arg GPIO_AF_CAN1: Connect CAN1 pins to AF9
  *            @arg GPIO_AF_CAN2: Connect CAN2 pins to AF9
  *            @arg GPIO_AF_TIM12: Connect TIM12 pins to AF9
  *            @arg GPIO_AF_TIM13: Connect TIM13 pins to AF9
  *            @arg GPIO_AF_TIM14: Connect TIM14 pins to AF9
  *            @arg GPIO_AF_OTG_FS: Connect OTG_FS pins to AF10
  *            @arg GPIO_AF_OTG_HS: Connect OTG_HS pins to AF10
  *            @arg GPIO_AF_ETH: Connect ETHERNET pins to AF11
  *            @arg GPIO_AF_FSMC: Connect FSMC pins to AF12
  *            @arg GPIO_AF_OTG_HS_FS: Connect OTG HS (configured in FS) pins to AF12
  *            @arg GPIO_AF_SDIO: Connect SDIO pins to AF12
  *            @arg GPIO_AF_DCMI: Connect DCMI pins to AF13
  *            @arg GPIO_AF_EVENTOUT: Connect EVENTOUT pins to AF15
  * @retval None
  */
/*----------------------------------------------------------*/
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
/*----------------------------------------------------------*/
#ifdef __cplusplus
namespace _internal {
namespace gpio_f4 {
#endif
/*----------------------------------------------------------*/
//Internal port to number conversion
static inline int _gpio_clock_port_to_number( GPIO_TypeDef* port )
{
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
}
/*----------------------------------------------------------*/
#ifdef __cplusplus
}}
#endif
/*----------------------------------------------------------*/

/*** Enable or disable CLK for selected port
 * @param[in] port GPIO porrt
 * @param[in] enable Enable disable flag
 */
static inline void gpio_clock_enable( GPIO_TypeDef* port, bool enable )
{
#ifdef __cplusplus
	using namespace stm32;
	using namespace _internal::gpio_f4;
#endif
	if(_gpio_clock_port_to_number( port ) < 0 )
		return;
	if( enable )
		RCC->AHB1ENR |=  _gpio_clock_port_to_number( port );
	else
		RCC->AHB1ENR &=  ~_gpio_clock_port_to_number( port );
}
/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------------------*/
#endif /* GPIO_F1X_H_ */
/*----------------------------------------------------------*/
