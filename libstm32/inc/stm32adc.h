/*
 * stm32adc.h
 *
 *  Created on: 23-07-2012
 *      Author: lucck
 */

#ifndef STM32ADC_H_
#define STM32ADC_H_

#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/stm32adc.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/stm32adc.h"
#else
#error "Unknown device type"
#endif


#endif /* STM32ADC_H_ */
