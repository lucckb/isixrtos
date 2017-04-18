#pragma once


#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
	void systick_set_reload(uint32_t value);
	bool systick_set_frequency(uint32_t freq, uint32_t ahb);
	uint32_t systick_get_reload(void);
	uint32_t systick_get_value(void);
	void systick_set_clocksource(uint8_t clocksource);
	void systick_interrupt_enable(void);
	void systick_interrupt_disable(void);
	void systick_counter_enable(void);
	void systick_counter_disable(void);
	bool systick_get_countflag(void);
	void systick_clear(void);
	uint32_t systick_get_calib(void);

#ifdef __cplusplus
}
#endif
