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

	//Serial debug interface
	constexpr pin ser0_pins[] {
		{ pinfunc::txd, gpio::num::PA2 },
		{}
	};

	//SPI controller
	constexpr pin spi1_pins[] {
		{ pinfunc::miso, gpio::num::PB4 },	//MISO config
		{ pinfunc::mosi, gpio::num::PB5 },	//MOSI config
		{ pinfunc::sck, gpio::num::PB3 },	//SCK config
		{ pinfunc::cs0, gpio::num::PB6 },	//DI_CS (Display)
		{ pinfunc::cs1, gpio::num::PB7 },	//MEM_CS (Memory)
		{}
	};

	constexpr device_conf spi1_conf {
		{},
		SPI1_IRQn,
		1,7,	//! IRQ prio subprio
		 device_conf::fl_dma		//! Use DMA transfer
	};




	constexpr device devices[]
	{
		{
			"serial0", reinterpret_cast<uintptr_t>(USART2),
			bus::apb1, LL_GPIO_AF_7,
			unsigned(std::log2(LL_APB1_GRP1_PERIPH_USART2)),
			ser0_pins,
			nullptr
		},
		{
			"spi1", reinterpret_cast<uintptr_t>(SPI1),
			bus::apb2, LL_GPIO_AF_5,
			unsigned(std::log2(LL_APB2_GRP1_PERIPH_SPI1)),
			spi1_pins,
			&spi1_conf
		},
		{}
	};
}

//! The machine config
constexpr configuration the_machine_config {
	clk,
	devices
};

}
