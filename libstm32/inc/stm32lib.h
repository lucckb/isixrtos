/*
 * stm32lib.h
 *
 *  Created on: 16-07-2012
 *      Author: lucck
 */

#pragma once

#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/stm32f10x_lib.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/stm32f4xx_lib.h"
#elif defined(STM32MCU_MAJOR_TYPE_F2)
#include "stm32f2x/stm32f2xx_lib.h"
#elif defined(STM32MCU_MAJOR_TYPE_F37)
#include "stm32f37x/stm32f37x_lib.h"
#elif defined(STM32MCU_MAJOR_TYPE_F7)
#include "stm32f7x/stm32f7xx_lib.h"
#elif defined(STM32MCU_MAJOR_TYPE_F3)
#include "stm32f30x/stm32f30x_lib.h"
#else
#error Selected MCU type is invalid
#endif

#include "stm32gpio.h"

