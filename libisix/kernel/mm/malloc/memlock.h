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
#include <isix/prv/mutex.h>
#include <isix/prv/scheduler.h>
#include <isix/arch/irq_cpu.h>


static inline __attribute__((always_inline))
void mm_lock_init( struct isix_mutex* mtx )
{
	if( !isix_mutex_create(mtx) ) {
		isix_bug("Memlock create failed");
	}
}

static inline __attribute__((always_inline))
void mm_lock_lock( struct isix_mutex* mtx )
{
    if(_isix_port_is_in_isr()) {
            isix_bug("memory allocator called from interrupt context");
    }
	if(schrun) {
		if( isix_mutex_lock(mtx) ) {
			isix_bug("Memlock lock failed");
		}
	}
}


static inline __attribute__((always_inline))
void mm_lock_unlock( struct isix_mutex* mtx )
{
    if(_isix_port_is_in_isr()) {
            isix_bug("memory allocator called from interrupt context");
    }
	if(schrun) {
		if( isix_mutex_unlock(mtx) ) {
			isix_bug("Memlock unlock failed");
		}
	}
}
