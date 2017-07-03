/*
 * =====================================================================================
 *
 *       Filename:  timer_interrupt.cpp
 *
 *    Description:  Timer interrupt API handling for testing IRQ primitives
 *
 *        Version:  1.0
 *        Created:  12.12.2013 21:55:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include "timer_interrupt.hpp"
#include <stm32rcc.h>
#include <stm32system.h>
#include <stm32tim.h>
#include <atomic>
#include <stdexcept>

namespace tests {
namespace detail {

//Private namespace handlers
namespace {
	timer_handler_t normal_handler;
	std::atomic<bool> initialized {};
}


//Handle function for periodic interrupt
void periodic_timer_setup( timer_handler_t normal, uint16_t timeval )
{
	using namespace stm32;
	if( initialized ) {
		throw std::logic_error("Timer already initialized");
	}
	rcc_apb1_periph_clock_cmd( RCC_APB1Periph_TIM3, true );
	rcc_apb1_periph_reset_cmd( RCC_APB1Periph_TIM3, true );
	__sync_synchronize();
	rcc_apb1_periph_reset_cmd( RCC_APB1Periph_TIM3, false );
	__sync_synchronize();
	nvic_set_priority( TIM3_IRQn , 1, 1 );
	tim_timebase_init( TIM3, 0, TIM_CounterMode_Up, timeval, 0, 0 );
	tim_it_config( TIM3, TIM_IT_Update, true );
	normal_handler = normal;
	nvic_irq_enable( TIM3_IRQn, true );
	tim_cmd( TIM3, true );
	initialized = true;
}
//Stop the priodic timer
void periodic_timer_stop() noexcept {

	if( !initialized ) {
		return;
	}
	using namespace stm32;
	tim_it_config( TIM3, TIM_IT_Update, false );
	tim_cmd( TIM3, false );
	nvic_irq_enable( TIM3_IRQn, false );
	initialized = false;
}

extern "C" {
	void __attribute__((interrupt)) tim3_isr_vector() noexcept
	{
		stm32::tim_clear_it_pending_bit( TIM3, TIM_IT_Update );
		normal_handler();
	}
}	//ExternC

}}	//Detail
