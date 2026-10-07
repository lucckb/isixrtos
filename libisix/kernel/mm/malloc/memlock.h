/*
 * =====================================================================================
 *
 *       Filename:  memlock.h
 *
 *    Description:  Memory lock mechanism
 *
 *        Version:  1.0
 *        Created:  18.06.2017 19:00:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <isix/config.h>
#include <isix/prv/scheduler.h>
#include <isix/arch/irq_cpu.h>
#include <isix/assert.h>
#include <isix/prv/test_hooks.h>

/* The heap runs with preemption disabled so a task cannot be killed
 * or suspended inside the allocator. Interrupts stay enabled. */
static inline __attribute__((always_inline))
void mm_lock_lock( void )
{
	if(_isix_port_is_in_isr()) {
		isix_bug("memory allocator called from interrupt context");
	}
	if(schrun) {
		_isixp_lock_scheduler();
		ISIX_TEST_POINT( isix_tp_heap_locked, NULL );
	}
}


static inline __attribute__((always_inline))
void mm_lock_unlock( void )
{
	if(_isix_port_is_in_isr()) {
		isix_bug("memory allocator called from interrupt context");
	}
	if(schrun) {
		ISIX_TEST_POINT( isix_tp_heap_unlocking, NULL );
		_isixp_unlock_scheduler();
	}
}
