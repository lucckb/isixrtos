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

/* ------------------------------------------------------------------ */ 
#if defined(STM32MCU_MAJOR_TYPE_F1)
#error RTC for stm32f1 platform is not supported yet
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/stm32rtc.h"
#elif defined(STM32MCU_MAJOR_TYPE_F2)
#include "stm32f2x/stm32rtc.h"
#else
#error "Unknown device type"
#endif

/* ------------------------------------------------------------------ */ 