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
#include <stm32_ll_rcc.h>
#include <stm32_ll_system.h>
#include <stm32_ll_tim.h>
#include <stm32_ll_bus.h>
#include <isix/arch/irq_platform.h>
#include <isix/arch/irq.h>
#include <isix/arch/isr_vectors.h>
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
void periodic_timer_setup( timer_handler_t normal, uint32_t maxval )
{
	if( initialized ) {
		throw std::logic_error("Timer already initialized");
	}
	LL_APB1_GRP1_EnableClock( LL_APB1_GRP1_PERIPH_TIM3 );
	__sync_synchronize();
	LL_APB1_GRP1_ForceReset( LL_APB1_GRP1_PERIPH_TIM3 );
	__sync_synchronize();
	LL_APB1_GRP1_ReleaseReset( LL_APB1_GRP1_PERIPH_TIM3 );
	__sync_synchronize();
	isix::set_irq_priority(TIM3_IRQn, {1, 7});
	LL_TIM_InitTypeDef tim_init { .Prescaler {1},
		.CounterMode {LL_TIM_COUNTERMODE_UP}, .Autoreload {maxval&0xffff},
		.ClockDivision {maxval>>16U} , .RepetitionCounter {0}
	};
	LL_TIM_Init(TIM3, &tim_init);
	LL_TIM_EnableIT_UPDATE(TIM3);
	normal_handler = normal;
	isix::request_irq(TIM3_IRQn);
	LL_TIM_EnableCounter(TIM3);
	initialized = true;
}
//Stop the priodic timer
void periodic_timer_stop() noexcept {

	if( !initialized ) {
		return;
	}
	LL_TIM_DisableIT_UPDATE(TIM3);
	LL_TIM_DisableCounter(TIM3);
	isix::free_irq(TIM3_IRQn);
	initialized = false;
}

ISIX_ISR_VECTOR(tim3_isr_vector)
{
    LL_TIM_ClearFlag_UPDATE(TIM3);
    normal_handler();
}

}}	//Detail
