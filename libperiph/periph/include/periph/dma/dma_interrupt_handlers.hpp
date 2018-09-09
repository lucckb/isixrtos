/*
 * =====================================================================================
 *
 *       Filename:  dma_interrupt_handlers.hpp
 *
 *    Description:  DMA interrupt handlers header file
 *
 *        Version:  1.0
 *        Created:  09.09.2018 21:39:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <functional>

namespace periph::drivers::dma::_handlers {
	int register_handler(int channel, std::function<void()> callback);
}
