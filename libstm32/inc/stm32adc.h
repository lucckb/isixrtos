/*
 * stm32adc.h
 *
 *  Created on: 23-07-2012
 *      Author: lucck
 */

#pragma once

#if defined(STM32MCU_MAJOR_TYPE_F1) || defined(STM32MCU_MAJOR_TYPE_F37)
#include "stm32f1xx/stm32adc.h"
#elif defined(STM32MCU_MAJOR_TYPE_F3)
#include "stm32f30x/stm32adc.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
#include "stm32f4x/stm32adc.h"
#else
#error "Unknown device type"
#endif


