/*
 * Timeouts and cancellation: a slow request is limited with with_timeout(),
 * and a long operation is stopped by the user button (EXTI0 interrupt) with
 * when_any(). The loser of when_any is canceled and finishes before the
 * caller continues.
 *
 * Author: Lucjan Bryndza
 */
#define CONFIG_ISIX_CPP_COROUTINES 1

#include "example_support.hpp"

#include <isix.h>
#include <foundation/sys/dbglog.h>
#include <isix/arch/irq.h>
#include <isix/arch/isr_vectors.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_exti.h>
#include <stm32_ll_gpio.h>
#include <stm32_ll_system.h>

namespace {

isix::co::event* g_button {};

// B1 user button on PA0: rising edge raises EXTI0
void button_init()
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);
	LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_0);
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_0);
	isix::request_irq(EXTI0_IRQn);
}

//! A request whose duration the caller does not control
isix::co::task<int> request(unsigned duration_ms)
{
	co_await isix::co::sleep_ms(duration_ms);
	co_return static_cast<int>(duration_ms);
}

isix::co::task<void> wait_for_button(isix::co::event& button)
{
	co_await button.wait_for(isix::ms2tick(10000));
}

isix::co::task<void> demo(isix::co::event& button)
{
	for (;;) {
		for (const unsigned ms : { 20U, 200U }) {
			const auto r = co_await isix::co::with_timeout(request(ms), isix::ms2tick(50));
			if (r) {
				dbprintf("request of %u ms finished: %d", ms, *r);
			} else {
				dbprintf("request of %u ms: %s", ms, isix_strerror(r.error()));
			}
		}
		auto slow = request(5000);
		auto pressed = wait_for_button(button);
		dbprintf("press the user button to stop the 5 s request");
		const auto first = co_await isix::co::when_any(slow, pressed);
		dbprintf("%s finished first, the other was canceled (%s)",
			first == 0 ? "the request" : "the button", isix_strerror(first == 0 ? pressed.error() : slow.error()));
	}
}

}

ISIX_ISR_VECTOR(exti0_isr_vector)
{
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
	if (g_button) {
		g_button->set_isr();
	}
}

int main()
{
	examples::console_init();
	static isix::co::scheduler_thread executor;
	if (executor.start(4096, 3) != ISIX_EOK) {
		dbprintf("cannot start the executor");
		return -1;
	}
	static isix::co::event button(executor.sched());
	g_button = &button;
	button_init();
	if (executor.post(demo(button)) != ISIX_EOK) {
		dbprintf("out of memory for the demo");
	}
	isix_start_scheduler();
	return 0;
}
