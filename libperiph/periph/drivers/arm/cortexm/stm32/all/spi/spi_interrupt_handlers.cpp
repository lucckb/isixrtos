/*
 * =====================================================================================
 *
 *       Filename:  spi_interrupt_handlers.cpp
 *
 *    Description:  SPI interrupt handlers
 *
 *        Version:  1.0
 *        Created:  10.08.2018 22:00:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <stm32f3xx_ll_spi.h>
#include <periph/core/error.hpp>
#include <cstdlib>
#include <functional>
#include "spi_interrupt_handlers.hpp"

namespace periph::drivers::_handlers {

namespace {
#if defined(SPI3)
	std::function<void()> spi_vector[3]= { std::abort, std::abort, std::abort }
#elif defined(SPI2)
	std::function<void()> spi_vector[2] = { std::abort, std::abort };
#elif defined(SPI1)
	std::function<void()> spi_vector[1] = { std::abort };
#endif
}


	int register_handler(const SPI_TypeDef * const spi, std::function<void()> callback ) {
#ifdef SPI1
		if(spi==SPI1) {
			spi_vector[0] = callback;
			return error::success;
		}
#endif
#ifdef SPI2
		if(spi==SPI2) {
			spi_vector[1] = callback;
			return error::success;
		}
#endif
#ifdef SPI3
		if(spi==SPI3) {
			spi_vector[2] = callback;
			return error::success;
		}
#endif
		return error::inval;
	}

//Interrupts handlers
extern "C" {

#ifdef SPI1
	__attribute__((interrupt)) void spi1_isr_vector() {
		spi_vector[0]();
	}
#endif
#ifdef SPI2
	__attribute__((interrupt)) void spi2_isr_vector() {
		spi_vector[1]();
	}
#endif
#ifdef SPI3
	__attribute__((interrupt)) void spi3_isr_vector() {
		spi_vector[2]();
	}
#endif
}

}
