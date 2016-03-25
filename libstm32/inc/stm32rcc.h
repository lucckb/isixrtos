/*
 * stm32dma.h
 *
 *  Created on: 17-07-2012
 *      Author: lucck
 */

#pragma once

#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/stm32rcc.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/stm32rcc.h"
#elif defined(STM32MCU_MAJOR_TYPE_F2)
#include "stm32f2x/stm32rcc.h"
#else
#error "Unknown device type"
#endif

