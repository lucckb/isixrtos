/*
 * stm32dma.h
 *
 *  Created on: 17-07-2012
 *      Author: lucck
 */

#ifndef STM32DMA_H_
#define STM32DMA_H_

#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/stm32dma.h"
#elif defined(STM32MCU_MAJOR_TYPE_F4)
#include "stm32f4x/stm32dma.h"
#elif defined(STM32MCU_MAJOR_TYPE_F2)
#include "stm32f2x/stm32dma.h"
#else
#error Selected MCU type is invalid
#endif


#endif /* STM32DMA_H_ */
