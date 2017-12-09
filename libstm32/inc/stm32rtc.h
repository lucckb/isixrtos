/*
 * =====================================================================================
 *
 *       Filename:  stm32rtc.h
 *
 *    Description:  STM32 RTC library implementation
 *
 *        Version:  1.0
 *        Created:  16.07.2014 12:53:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */

#pragma once

#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/stm32rtc.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2) || \
	defined(STM32MCU_MAJOR_TYPE_F37) || defined(STM32MCU_MAJOR_TYPE_F3)
#include "stm32f4x/stm32rtc.h"
#else
#error "Unknown device type"
#endif

