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

#include <stm32f4xx_ll_dma.h>
#include <functional>
#include <periph/core/error.hpp>
#include <periph/dma/dma_interrupt_handlers.hpp>

namespace periph::drivers::dma::_handlers {

namespace {
	constexpr auto vectors_siz = 16;
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
	//DMA1
	__attribute__((interrupt)) void dma1_stream0_isr_vector() {
		vectors[0]();
	}
	__attribute__((interrupt)) void dma1_stream1_isr_vector() {
		vectors[1]();
	}
	__attribute__((interrupt)) void dma1_stream2_isr_vector() {
		vectors[2]();
	}
	__attribute__((interrupt)) void dma1_stream3_isr_vector() {
		vectors[3]();
	}
	__attribute__((interrupt)) void dma1_stream4_isr_vector() {
		vectors[4]();
	}
	__attribute__((interrupt)) void dma1_stream5_isr_vector() {
		vectors[5]();
	}
	__attribute__((interrupt)) void dma1_stream6_isr_vector() {
		vectors[6]();
	}
	__attribute__((interrupt)) void dma1_stream7_isr_vector() {
		vectors[7]();
	}
	//DMA2
	__attribute__((interrupt)) void dma2_stream0_isr_vector() {
		vectors[8]();
	}
	__attribute__((interrupt)) void dma2_stream1_isr_vector() {
		vectors[9]();
	}
	__attribute__((interrupt)) void dma2_stream2_isr_vector() {
		vectors[10]();
	}
	__attribute__((interrupt)) void dma2_stream3_isr_vector() {
		vectors[11]();
	}
	__attribute__((interrupt)) void dma2_stream4_isr_vector() {
		vectors[12]();
	}
	__attribute__((interrupt)) void dma2_stream5_isr_vector() {
		vectors[13]();
	}
	__attribute__((interrupt)) void dma2_stream6_isr_vector() {
		vectors[14]();
	}
	__attribute__((interrupt)) void dma2_stream7_isr_vector() {
		vectors[15]();
	}

}


}

