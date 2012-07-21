/*
 * stm32fmc.hpp
 *
 *  Created on: 21-07-2012
 *      Author: lucck
 */

#ifndef STM32FMC_HPP_
#define STM32FMC_HPP_

#if defined(STM32MCU_MAJOR_TYPE_F1)
#include "stm32f1xx/stm32f1fmc.hpp"
#else
#error Selected MCU type is invalid for this module
#endif



#endif /* STM32FMC_HPP_ */
