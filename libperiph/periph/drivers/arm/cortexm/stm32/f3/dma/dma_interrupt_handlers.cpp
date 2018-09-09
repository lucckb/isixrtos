/*
 * =====================================================================================
 *
 *       Filename:  dma_isr_vectors.cpp
 *
 *    Description:  DMA ISR vectors source file
 *
 *        Version:  1.0
 *        Created:  09.09.2018 21:16:28
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <stm32f3xx_ll_dma.h>
#include <functional>
#include <periph/core/error.hpp>
#include <periph/dma/dma_interrupt_handlers.hpp>

namespace periph::drivers::dma::_handlers {

namespace {
	constexpr auto vectors_siz = 7;
	std::function<void()> vectors[vectors_siz];
}

//Register handler
int register_handler(int channel, std::function<void()> callback)
{
	if(channel >= vectors_siz) {
		return error::inval;
	}
	vectors[channel] = callback;
	return error::success;
}


extern "C" {

	__attribute__((interrupt)) void dma1_channel1_isr_vector() {
		vectors[0]();
	}
	__attribute__((interrupt)) void dma1_channel2_isr_vector() {
		vectors[1]();
	}
	__attribute__((interrupt)) void dma1_channel3_isr_vector() {
		vectors[2]();
	}
	__attribute__((interrupt)) void dma1_channel4_isr_vector() {
		vectors[3]();
	}
	__attribute__((interrupt)) void dma1_channel5_isr_vector() {
		vectors[4]();
	}
	__attribute__((interrupt)) void dma1_channel6_isr_vector() {
		vectors[5]();
	}
	__attribute__((interrupt)) void dma1_channel7_isr_vector() {
		vectors[6]();
	}
}


}

