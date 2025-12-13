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
#include <stm32_ll_rcc.h>
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


static uint32_t tim_apb1_clk_hz()
{

    LL_RCC_ClocksTypeDef clk;
    LL_RCC_GetSystemClocksFreq(&clk);
     if (LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_1) {
        return clk.PCLK1_Frequency;
     } else if( LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_2) {
        return clk.PCLK1_Frequency * 2U;
     } else if( LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_4) {
        return clk.PCLK1_Frequency * 4U;
     } else if( LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_8) {
        return clk.PCLK1_Frequency * 8U;
     } else if( LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_16) {
        return clk.PCLK1_Frequency * 16U;
     }
     throw std::logic_error("unable to determine tim freq");
     return 0;
}

struct tim_div_t {
    uint16_t psc;
    uint16_t arr;
};

static tim_div_t compute_tim_div_us(uint64_t total_div)
{
    tim_div_t r{};

    if (total_div == 0)
        throw std::logic_error("invalid timer period");

    if (total_div <= 0x10000ULL) {
        r.psc = 0;
        r.arr = static_cast<uint16_t>(total_div - 1);
    } else {
        uint64_t psc = (total_div + 0xFFFFULL) / 0x10000ULL; // ceil
        uint64_t arr = total_div / psc;

        if (psc > 0x10000ULL)
            throw std::logic_error("timer period too large");

        r.psc = static_cast<uint16_t>(psc - 1);
        r.arr = static_cast<uint16_t>(arr - 1);
    }

    return r;
}


void periodic_timer_setup(timer_handler_t cb, uint32_t period_us)
{
    if (initialized) throw std::logic_error("Timer already initialized");

    uint32_t tim_clk = tim_apb1_clk_hz();
    uint64_t total_div = (uint64_t)tim_clk * period_us / 1'000'000ULL;

    auto div = compute_tim_div_us(total_div);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
    __sync_synchronize();
    LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_TIM3);
    __sync_synchronize();
    LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_TIM3);
    __sync_synchronize();

    normal_handler = cb;

    LL_TIM_InitTypeDef tim{};
    tim.Prescaler = div.psc;
    tim.CounterMode = LL_TIM_COUNTERMODE_UP;
    tim.Autoreload = div.arr;
    tim.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    tim.RepetitionCounter = 0;
    LL_TIM_Init(TIM3, &tim);

    LL_TIM_SetCounter(TIM3, 0);
    LL_TIM_GenerateEvent_UPDATE(TIM3);
    LL_TIM_ClearFlag_UPDATE(TIM3);

    isix::set_irq_priority(TIM3_IRQn, {1, 7});
    isix::request_irq(TIM3_IRQn);

    LL_TIM_EnableIT_UPDATE(TIM3);
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
