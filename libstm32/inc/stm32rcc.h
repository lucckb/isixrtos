/*
 * stm32dma.h
 *
 *  Created on: 17-07-2012
 *      Author: lucck
 */

#ifndef STM32RCC_H_
#define STM32RCC_H_

#if defined(STM32MCU_MAJOR_TYPE_F1)
#error rcc not implemented
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/stm32rcc.h"
#else
#error Selected MCU type is invalid
#endif


#endif /* STM32DMA_H_ */
