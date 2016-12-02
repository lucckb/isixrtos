/*
 * =====================================================================================
 *
 *       Filename:  stm32f4x_gpio.h
 *
 *    Description:  LEGACY CODE for STM32 GPIO
 *
 *        Version:  1.0
 *        Created:  25.03.2016 19:31:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

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


/** Old compatibility def not recommended */
enum e_gpio_ospeed_legacy
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


