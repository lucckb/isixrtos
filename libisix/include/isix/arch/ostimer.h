#pragma once

#include <isix/config.h>
#include <isix/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned long _isix_port_get_hres_jiffies_timer_value(void);
unsigned long _isix_port_get_hres_jiffies_timer_max_value(void);
unsigned long _isix_port_get_core_freq(void);

#ifdef CONFIG_ISIX_TICKLESS
void _isix_port_configure_tickless_oneshot_timer(ostick_t ticks_until_timeout, unsigned long ahb_freq);
void _isix_port_restore_periodic_tick_timer(unsigned long ahb_freq);
uint32_t _isix_port_tickless_get_oneshot_timer_current_value(void);
uint32_t _isix_port_tickless_get_oneshot_timer_reload_value(void);
#endif

void isix_pre_sleep_hook(ostick_t expected_sleep_ticks);
void isix_post_sleep_hook(ostick_t actual_sleep_ticks);

#ifdef __cplusplus
}
#endif
