/*
 * =====================================================================================
 *
 *       Filename:  spi_interrupt_handlers.hpp
 *
 *    Description:  SPI interrupt handlers
 *
 *        Version:  1.0
 *        Created:  10.08.2018 22:24:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <stm32_ll_spi.h>
#include <functional>

namespace periph::drivers::spi::_handlers {

	int register_handler(const SPI_TypeDef * const spi, std::function<void()> callback);
}
