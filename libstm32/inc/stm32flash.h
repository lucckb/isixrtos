/*
 * =====================================================================================
 *
 *       Filename:  stm32flash.h
 *
 *    Description: STM32 flash controller for all platforms
 *
 *        Version:  1.0
 *        Created:  02.10.2014 21:09:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#pragma once

#if defined(STM32MCU_MAJOR_TYPE_F1)
#error FLASH interface for stm32f1 platform not implemented
#elif defined(STM32MCU_MAJOR_TYPE_F37)
#include "stm32f37x/stm32flash.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
#include "stm32f2f4xx_flash.h"
#else
#error selected mcu is invalid
#endif

