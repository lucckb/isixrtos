/*
 * Copyright (c) 2025 Lucjan Bryndza
 *
 * Private interface between the kernel and the port of the system timer.
 */
#pragma once

#include <isix/config.h>
#include <isix/types.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** The system timer produces the kernel tick and, with the tickless idle, one long
 * timeout instead of the periodic ticks.
 *
 * The port owns the timer cycles, the phase of the tick, the timer width and its wrap.
 * The kernel owns the jiffies counter and the decision about the sleep.
 * All functions except _isix_port_systimer_init are called by the kernel inside the
 * kernel critical section. The tick boundaries stay in phase: set_timeout and resync
 * never shift them.
 */

//! Frequency of the core clock the timer was configured for
unsigned long _isix_port_get_core_freq(void);

/** Configure the timer and start the periodic tick, called once from the hardware init
 * @param[in] core_freq Core clock frequency in Hz
 */
void _isix_port_systimer_init( unsigned long core_freq );

/** Account the ticks and call isix_systime_handler, called by the port from the timer interrupt
 * in the kernel critical section
 * @param[in] ticks Number of the ticks that elapsed since the previous announce
 */
void _isixp_systimer_announce( ostick_t ticks );

/** Timer cycles since the last tick that was announced
 * @param[out] cycles_per_tick Timer cycles in one tick
 * @return Elapsed timer cycles, it is greater than a tick when announces are pending
 */
uint32_t _isix_port_systimer_subtick( uint32_t* cycles_per_tick );

/** Timer interrupt service, called from the interrupt vector of the timer */
void _isix_port_systimer_isr(void);

#if CONFIG_ISIX_TICKLESS
/** Longest timeout the timer can wait in one shot
 * @return Number of ticks, 1 when only the periodic tick is possible
 */
ostick_t _isix_port_systimer_max_ticks(void);

/** Move the next timer interrupt to the tick boundary the given number of ticks after the last announced one
 * @param[in] ticks Requested timeout in ticks, 1 or less keeps the periodic tick
 * @return Timeout that was programmed, 1 when the periodic tick is kept
 */
ostick_t _isix_port_systimer_set_timeout( ostick_t ticks );

//! Return true when a timeout set by _isix_port_systimer_set_timeout is waiting
bool _isix_port_systimer_is_sleeping(void);

/** Return to the periodic tick and consume the ticks that elapsed
 * @return Whole ticks to announce, zero when the timer interrupt is about to do it
 */
ostick_t _isix_port_systimer_resync(void);
#endif

//! Called before the core is put to sleep for the expected number of ticks
void isix_pre_sleep_hook(ostick_t expected_sleep_ticks);
//! Called after the sleep with the number of ticks that really elapsed
void isix_post_sleep_hook(ostick_t actual_sleep_ticks);

#ifdef __cplusplus
}
#endif
