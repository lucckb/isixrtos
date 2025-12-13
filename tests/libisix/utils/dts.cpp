#include <cmath>
#include <periph/dt/dts.hpp>
#include <periph/gpio/gpio_numbers.hpp>
#include <periph/dt/dts_config.hpp>
#include <isix/arch/irq.h>
#include <stm32_ll_usart.h>
#include <stm32_ll_gpio.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_rcc.h>



namespace periph::dt::_dts_config {

namespace {
	constexpr clock clk[] {
		{	bus::ahb1, []() -> unsigned {
			 LL_RCC_ClocksTypeDef clk;
			  LL_RCC_GetSystemClocksFreq(&clk);
			  return clk.HCLK_Frequency;
			}
		},
		{	bus::apb1, []() -> unsigned {
			 LL_RCC_ClocksTypeDef clk;
			  LL_RCC_GetSystemClocksFreq(&clk);
			  return clk.PCLK1_Frequency;
			}
		},
		{	bus::apb2, []() -> unsigned {
			 LL_RCC_ClocksTypeDef clk;
			  LL_RCC_GetSystemClocksFreq(&clk);
			  return clk.PCLK2_Frequency;
			}
		},
		{	bus::cpu, []() -> unsigned {
			 LL_RCC_ClocksTypeDef clk;
			  LL_RCC_GetSystemClocksFreq(&clk);
			  return clk.SYSCLK_Frequency;
			}
		},
		{}
	};

// TODO fix this
#if defined(STM32F411xE)
    constexpr pin ser0_pins[] {
		{ pinfunc::txd, gpio::num::PA2 },
		{}
	};
#elif defined(STM32F405xx)
	//Serial debug interface
	constexpr pin ser0_pins[] {
		{ pinfunc::txd, gpio::num::PA9 },
		{}
	};
#endif

	constexpr device devices[]
	{

#if defined(STM32F411xE)
		{
			"serial0", reinterpret_cast<uintptr_t>(USART2),
			bus::apb1, LL_GPIO_AF_7,
			unsigned(std::log2(LL_APB1_GRP1_PERIPH_USART2)),
			ser0_pins,
			nullptr
		},
#elif defined(STM32F405xx)
		{
			"serial0", reinterpret_cast<uintptr_t>(USART1),
			bus::apb2, LL_GPIO_AF_7,
			unsigned(std::log2(LL_APB2_GRP1_PERIPH_USART1)),
			ser0_pins,
			nullptr
		},
#else
#error unknown board
#endif
		{}
	};
}

//! The machine config
constexpr configuration the_machine_config {
	clk,
	devices
};

}
