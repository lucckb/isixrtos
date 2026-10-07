/*
 * Copyright (c) 2025 Lucjan Bryndza
 *
 * System timer of the Cortex-M core based on SysTick.
 *
 * The counter is never stopped, so the phase of the tick is kept. The tickless idle
 * loads one long period at the next tick boundary: the state is armed until that
 * boundary, sleeping while the long period runs and off with the periodic tick.
 */
#include <isix/config.h>
#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/assert.h>
#include <isix/prv/test_hooks.h>
#include <isix/arch/ostimer.h>
#include <isix/cortexm/systick_regs.h>
#include <isix/cortexm/systick.h>
#include <isix/cortexm/scb_regs.h>
#include <stdint.h>

// Timer cycles in one tick, valid for both SysTick clock sources
static uint32_t g_tick_cycles = 1U;

static inline bool tick_pending(void)
{
	return (SCB_ICSR & SCB_ICSR_PENDSTSET) != 0U;
}

#if CONFIG_ISIX_TICKLESS

static inline void tick_clear_pending(void)
{
	SCB_ICSR = SCB_ICSR_PENDSTCLR;
}

// The tick interrupt handler was entered and not left
static inline bool tick_handler_active(void)
{
	return (SCB_SHCSR & SCB_SHCSR_SYSTICKACT) != 0U;
}

// The counter is 24 bit
#define SYSTIMER_RANGE 0x1000000UL
// Next boundary closer than this to the restart is counted at once
#define SYSTIMER_MIN_FIRST_CYCLES 64U
// Cycles between the counter sample and the counter restart
#ifndef SYSTIMER_RESTART_SKEW
#define SYSTIMER_RESTART_SKEW 59U
#endif

enum { st_off, st_armed, st_sleeping };

static volatile uint8_t g_state;
// Length of the long period in ticks
static ostick_t g_expected;

#ifdef CONFIG_ISIX_TEST
static struct isix_tickless_stats g_stats;
static ostick_t g_test_max_ticks;
#define TL_STAT( field ) ( ++g_stats.field )
#else
#define TL_STAT( field ) do {} while(0)
#endif

struct split_res {
	ostick_t n;
	uint32_t first;
};

// Whole ticks that have run in the long period and the first period that keeps the tick phase
static struct split_res systimer_split( uint32_t long_cycles, uint32_t cvr, uint32_t tpp, uint32_t skew )
{
	const uint32_t ran = (cvr < long_cycles) ? long_cycles - 1U - cvr : 0U;
	struct split_res res = {
		.n = (ostick_t)(ran / tpp),
		.first = tpp - (ran % tpp),
	};
	// The period starts later than the sample
	res.first = (res.first > skew) ? res.first - skew : 0U;
	if( tpp >= 2U * SYSTIMER_MIN_FIRST_CYCLES && res.first < SYSTIMER_MIN_FIRST_CYCLES ) {
		res.n++;
		res.first += tpp;
	}
	return res;
}

// Restart the counter with the first period, the boundary that was passed is counted by the caller
static ostick_t systimer_restart( uint32_t long_cycles, uint32_t cvr, uint32_t tpp )
{
	const struct split_res split = systimer_split( long_cycles, cvr, tpp, SYSTIMER_RESTART_SKEW );
	STK_RVR = (split.first < 2U ? 2U : split.first) - 1U;
	STK_CVR = 0U;
	// Wait until the counter has reloaded from the new period
	while( STK_CVR == 0U ) {}
	STK_RVR = tpp - 1U;
	tick_clear_pending();
	g_state = st_off;
	TL_STAT(restarts);
#ifdef CONFIG_ISIX_TEST
	g_stats.ticks_caught_up += split.n;
#endif
	return split.n;
}

ostick_t _isix_port_systimer_max_ticks(void)
{
	// The rest of the current tick is the first jiffy
	ostick_t max_ticks = (ostick_t)(SYSTIMER_RANGE / g_tick_cycles);
#ifdef CONFIG_ISIX_TEST
	if( g_test_max_ticks && g_test_max_ticks < max_ticks ) {
		max_ticks = g_test_max_ticks;
	}
#endif
	return max_ticks < 2U ? 1U : max_ticks + 1U;
}

ostick_t _isix_port_systimer_set_timeout( ostick_t timeout )
{
	const uint32_t tpp = g_tick_cycles;
#ifdef CONFIG_ISIX_TEST
	g_stats.tpp = tpp;
#endif
	if( g_state != st_off || timeout < 2U ) {
		return 1U;
	}
	ostick_t ticks = timeout - 1U;
	if( ticks > (ostick_t)CONFIG_ISIX_TICKLESS_TIMER_COMPENSATION_TICKS ) {
		ticks -= (ostick_t)CONFIG_ISIX_TICKLESS_TIMER_COMPENSATION_TICKS;
	}
	const ostick_t max_ticks = _isix_port_systimer_max_ticks() - 1U;
	if( ticks > max_ticks ) {
		ticks = max_ticks;
	}
	// The long period must be longer than the tick period
	if( ticks < 2U ) {
		return 1U;
	}
	g_expected = ticks;
	ISIX_TEST_POINT( isix_tp_tickless_pre_arm, NULL );
	STK_RVR = ticks * tpp - 1U;
	ISIX_TEST_POINT( isix_tp_tickless_armed, NULL );
	if( tick_pending() && STK_CVR < tpp ) {
		// The boundary was reached before the reload update
		TL_STAT(enter_aborted);
		STK_RVR = tpp - 1U;
		return 1U;
	}
	TL_STAT(sleeps);
	g_state = st_armed;
	return ticks + 1U;
}

bool _isix_port_systimer_is_sleeping(void)
{
	return g_state != st_off;
}

ostick_t _isix_port_systimer_resync(void)
{
	// The tick handler was entered before this nested interrupt and accounts the boundary itself
	if( g_state == st_off || tick_handler_active() ) {
		return 0U;
	}
	const uint32_t tpp = g_tick_cycles;
	const uint32_t long_cycles = g_expected * tpp;
	if( g_state == st_armed ) {
		TL_STAT(wake_armed);
		ISIX_TEST_POINT( isix_tp_tickless_wake_armed, NULL );
		STK_RVR = tpp - 1U;
		const uint32_t cvr = STK_CVR;
		if( cvr < tpp ) {
			g_state = st_off;
			return 0U;
		}
		// The boundary passed before the reload update and the long period is running
		TL_STAT(wake_armed_passed);
		tick_clear_pending();
		return 1U + systimer_restart( long_cycles, cvr, tpp );
	}
	uint32_t cvr = STK_CVR;
	if( tick_pending() ) {
		return 0U;	// The long period ended, the tick handler accounts it
	}
	TL_STAT(wake_sleeping);
	ISIX_TEST_POINT( isix_tp_tickless_wake_sleeping, &cvr );
	return systimer_restart( long_cycles, cvr, tpp );
}

#ifdef CONFIG_ISIX_TEST
void _isixp_test_tickless_stats( struct isix_tickless_stats* out )
{
	isix_enter_critical();
	*out = g_stats;
	isix_exit_critical();
}

void _isixp_test_tickless_stats_reset(void)
{
	isix_enter_critical();
	g_stats = (struct isix_tickless_stats){ .tpp = g_stats.tpp };
	isix_exit_critical();
}

int _isixp_test_tickless_state(void)
{
	return g_state;
}

void _isixp_test_tickless_set_max_ticks( ostick_t max_ticks )
{
	g_test_max_ticks = max_ticks;
}

void _isixp_test_tickless_split( uint32_t long_cycles, uint32_t cvr, uint32_t tpp,
	ostick_t* n, uint32_t* first )
{
	const struct split_res res = systimer_split( long_cycles, cvr, tpp, 0U );
	*n = res.n;
	*first = res.first;
}
#endif /* CONFIG_ISIX_TEST */

#endif /* CONFIG_ISIX_TICKLESS */

void _isix_port_systimer_init( unsigned long core_freq )
{
	if( !systick_set_frequency(CONFIG_ISIX_HZ, core_freq) ) {
		isix_bug( "Unable to configure frequency for systick");
	}
	g_tick_cycles = STK_RVR + 1U;
	systick_interrupt_enable();
	systick_counter_enable();
}

uint32_t _isix_port_systimer_subtick( uint32_t* cycles_per_tick )
{
	const uint32_t tpp = g_tick_cycles;
	bool pending;
	uint32_t cvr;
	// A boundary between the two reads changes the meaning of the counter
	do {
		pending = tick_pending();
		cvr = STK_CVR;
	} while( pending != tick_pending() );
	// Cycles accounted before the period the counter runs now and the length of that period
	uint32_t before = pending ? tpp : 0U;
	uint32_t period = tpp;
#if CONFIG_ISIX_TICKLESS
	const uint32_t long_cycles = g_expected * tpp;
	if( g_state == st_armed && pending ) {
		period = long_cycles;
	} else if( g_state == st_sleeping ) {
		before = pending ? long_cycles : 0U;
		period = pending ? tpp : long_cycles;
	}
#endif
	*cycles_per_tick = tpp;
	return before + (cvr < period ? period - 1U - cvr : 0U);
}

void _isix_port_systimer_isr(void)
{
	ostick_t ticks = 1U;
#if CONFIG_ISIX_TICKLESS
	ISIX_TEST_POINT( isix_tp_tickless_isr_entry, NULL );
#endif
	// Interrupts that wake tasks must not run between the timer state change and the announce
	isix_enter_critical();
#if CONFIG_ISIX_TICKLESS
	if( g_state == st_armed ) {
		// First boundary, the long period has just been loaded
		STK_RVR = g_tick_cycles - 1U;
		ISIX_TEST_POINT( isix_tp_tickless_isr_armed, NULL );
		g_state = st_sleeping;
	} else if( g_state == st_sleeping ) {
		g_state = st_off;
		TL_STAT(expired);
#ifdef CONFIG_ISIX_TEST
		g_stats.ticks_caught_up += g_expected;
#endif
		ticks = g_expected;
	}
#endif
	_isixp_systimer_announce( ticks );
	isix_exit_critical();
}
