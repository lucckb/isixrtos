/*
 * =====================================================================================
 *
 *       Filename:  stm32i2c.h
 *
 *    Description:  STMI2C library
 *
 *        Version:  1.0
 *        Created:  29.03.2016 13:40:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#if defined(STM32MCU_MAJOR_TYPE_F37)||defined(STM32MCU_MAJOR_TYPE_F3)
#include "stm32i2c_v2.h"
#else
#include "stm32i2c_v1.h"
#endif
