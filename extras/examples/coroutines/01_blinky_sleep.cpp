/*
 * Four LEDs blinking at different rates, each driven by its own coroutine
 * that only sleeps. All of them share one RTOS thread.
 *
 * Author: Lucjan Bryndza
 */
#define CONFIG_ISIX_CPP_COROUTINES 1

#include "example_support.hpp"

#include <isix.h>
#include <foundation/sys/dbglog.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_gpio.h>

namespace {

// STM32F411E-DISCO user LEDs: green, orange, red, blue on PD12..PD15
constexpr uint32_t c_led_pins[] {
	LL_GPIO_PIN_12, LL_GPIO_PIN_13, LL_GPIO_PIN_14, LL_GPIO_PIN_15
};
constexpr unsigned c_periods_ms[] { 125, 250, 500, 1000 };

void leds_init()
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
	for (const auto pin : c_led_pins) {
		LL_GPIO_SetPinMode(GPIOD, pin, LL_GPIO_MODE_OUTPUT);
	}
}

isix::co::task<void> blink(uint32_t pin, unsigned period_ms)
{
	for (;;) {
		LL_GPIO_TogglePin(GPIOD, pin);
		co_await isix::co::sleep_ms(period_ms / 2);
	}
}

isix::co::task<void> heartbeat()
{
	for (unsigned seconds = 1;; ++seconds) {
		co_await isix::co::sleep_ms(1000);
		dbprintf("alive for %u s", seconds);
	}
}

}

int main()
{
	examples::console_init();
	leds_init();
	static isix::co::scheduler_thread executor;
	if (executor.start(4096, 3) != ISIX_EOK) {
		dbprintf("cannot start the executor");
		return -1;
	}
	for (unsigned n = 0; n < std::size(c_led_pins); ++n) {
		if (executor.post(blink(c_led_pins[n], c_periods_ms[n])) != ISIX_EOK) {
			dbprintf("out of memory for LED %u", n);
		}
	}
	if (executor.post(heartbeat()) != ISIX_EOK) {
		dbprintf("out of memory for the heartbeat");
	}
	isix_start_scheduler();
	return 0;
}
