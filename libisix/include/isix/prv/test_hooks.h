/*
 * Copyright (c) 2025 Lucjan Bryndza
 *
 * Test-only kernel hooks, compiled in when CONFIG_ISIX_TEST_HOOKS is set.
 */
#pragma once

#include <isix/types.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef CONFIG_ISIX_TEST
#ifdef __cplusplus
extern "C" {
#endif
//! Set the system jiffies counter
void _isixp_test_set_jiffies( ostick_t v );
//! Advance the system time by the number of ticks
void _isixp_test_advance_jiffies( ostick_t dt );

//! Tickless idle event counters
struct isix_tickless_stats {
	uint32_t sleeps;			//! Long period armed
	uint32_t enter_aborted;		//! Tick boundary reached before the long period was loaded
	uint32_t wake_armed;		//! Early wakeup before the first boundary
	uint32_t wake_armed_passed;	//! Early wakeup after the boundary but before its handler
	uint32_t wake_sleeping;		//! Early wakeup inside the long period
	uint32_t expired;			//! Long period ended
	uint32_t restarts;			//! Tick phase restarts
	uint32_t ticks_caught_up;	//! Ticks accounted in batches
	uint32_t tpp;				//! Timer cycles per tick
};
//! Read the tickless counters
void _isixp_test_tickless_stats( struct isix_tickless_stats* out );
//! Clear the tickless counters
void _isixp_test_tickless_stats_reset(void);
//! Tickless state machine state, zero when the periodic tick runs
int _isixp_test_tickless_state(void);
//! Limit the long period to the number of ticks, zero removes the limit
void _isixp_test_tickless_set_max_ticks( ostick_t max_ticks );
//! Use the real wfi in the idle even if the build uses nop for the debugger
void _isixp_test_tickless_force_wfi( bool enable );
//! Split the elapsed part of the long period into the whole ticks and the first period
void _isixp_test_tickless_split( uint32_t long_cycles, uint32_t cvr, uint32_t tpp,
	ostick_t* n, uint32_t* first );
#ifdef __cplusplus
}
#endif
#endif

#if CONFIG_ISIX_TEST_HOOKS
#ifdef __cplusplus
extern "C" {
#endif

//! Places in the kernel where the test can inject its code
enum isix_test_point {
	//! Idle task detached a zombie task, arg is the task identifier only
	isix_tp_task_cleanup,
	//! Semaphore signal left the critical section, arg is the semaphore
	isix_tp_sem_signal_notify,
	//! Heap allocator entered its protected section, arg is null (the hook must not allocate or block)
	isix_tp_heap_locked,
	//! Heap allocator is about to leave its protected section, arg is null
	isix_tp_heap_unlocking,
	//! Tickless idle is about to load the long period, arg is null (the hook may only poll the timer)
	isix_tp_tickless_pre_arm,
	//! Tickless idle loaded the long period, arg is null
	isix_tp_tickless_armed,
	//! Early wakeup in the armed state before the period update, arg is null
	isix_tp_tickless_wake_armed,
	//! Early wakeup inside the long period after the counter read, arg is a pointer to the read value
	isix_tp_tickless_wake_sleeping,
	//! Tick interrupt in the armed state before it leaves the state, arg is null
	isix_tp_tickless_isr_armed,
	//! Tick interrupt handler entered, before the kernel masks the interrupts, arg is null
	isix_tp_tickless_isr_entry
};

//! Test hook called at the selected points, null when not used
extern void (*volatile _isixp_test_hook)( enum isix_test_point point, void* arg );

//! Current critical section nesting counter
int _isixp_test_critical_count(void);

//! Check the heap structures consistency, 0 when consistent, negative rule number otherwise
int _isixp_heap_check(void);

#ifdef __cplusplus
}
#endif

#define ISIX_TEST_POINT( POINT, ARG ) \
	do { if( _isixp_test_hook ) _isixp_test_hook( (POINT), (ARG) ); } while(0)
#else
#define ISIX_TEST_POINT( POINT, ARG ) do {} while(0)
#endif
