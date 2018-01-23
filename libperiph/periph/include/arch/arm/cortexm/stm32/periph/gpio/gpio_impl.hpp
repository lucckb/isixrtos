#pragma once

#ifdef STM32MCU_MAJOR_TYPE_F1
#include "gpio_impl_v1.hpp"
#else
#include "gpio_impl_v2.hpp"
#endif

namespace periph {
namespace gpio {
#ifdef STM32MCU_MAJOR_TYPE_F1
	namespace impl = periph::gpio::stm32::gpio_v1;
#else
	namespace impl = periph::gpio::stm32::gpio_v2;
#endif

}}
