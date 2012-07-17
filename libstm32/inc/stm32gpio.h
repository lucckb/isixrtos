/*
 * stm32gpio.h
 *
 *  Created on: 17-07-2012
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef STM32GPIO_H_
#define STM32GPIO_H_
/*----------------------------------------------------------*/

#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/gpio_f1x.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/gpio_f4x.h"
#else
#error Selected MCU type is invalid
#endif



/*----------------------------------------------------------*/
#endif /* STM32GPIO_H_ */
/*----------------------------------------------------------*/
