/*
 * stm32pwr.h
 *
 *  Created on: 24-07-2012
 *      Author: lucck
 */

#ifndef STM32PWR_H_
#define STM32PWR_H_


#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/stm32pwr.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/stm32pwr.h"
#else
#error "Unknown device type"
#endif


#endif /* STM32PWR_H_ */
