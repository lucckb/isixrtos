/*
 * stm32lib.h
 *
 *  Created on: 16-07-2012
 *      Author: lucck
 */

#ifndef STM32LIB_H_
#define STM32LIB_H_

#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/stm32f10x_lib.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/stm32f4xx_lib.h"
#elif defined(STM32MCU_MAJOR_TYPE_F2)
#include "stm32f2x/stm32f2xx_lib.h"
#else
#error Selected MCU type is invalid
#endif

#include "stm32gpio.h"
#include "stm32fxxx_usart.h"
#endif /* STM32LIB_H_ */
