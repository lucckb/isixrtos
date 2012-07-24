/*
 * stm32exti.h
 *
 *  Created on: 24-07-2012
 *      Author: lucck
 */

#ifndef STM32EXTI_H_
#define STM32EXTI_H_

#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/stm32exti.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/stm32exti.h"
#else
#error "Unknown device type"
#endif



#endif /* STM32EXTI_H_ */
