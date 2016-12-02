/*
 * stm32dma.h
 *
 *  Created on: 17-07-2012
 *      Author: lucck
 */

#pragma once

#if defined(STM32MCU_MAJOR_TYPE_F1)||defined(STM32MCU_MAJOR_TYPE_F3)|| \
	defined(STM32MCU_MAJOR_TYPE_F37)
#include "stm32f1xx/stm32dma.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4)||defined(STM32MCU_MAJOR_TYPE_F2)
#include "stm32f4x/stm32dma.h"
#else
#error Selected MCU type is invalid
#endif

