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


#ifndef  _stm32flash_INC
#define  _stm32flash_INC


#if defined(STM32MCU_MAJOR_TYPE_F1) 
#error FLASH interface for stm32f1 platform not implemented
#else
#include "stm32f2f4xx_flash.h"
#endif

#endif   /* ----- #ifndef stm32flash_INC  ----- */
