/*
 * =====================================================================================
 *
 *       Filename:  fifo_lock.h
 *
 *    Description:  FIFO lock API
 *
 *        Version:  1.0
 *        Created:  19.04.2015 19:14:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once 
#include <isix/prv/scheduler.h>
#include <isix/prv/fifo.h>
#include <isix/fifo.h>


//! Private function for lock
static inline __attribute__((always_inline)) 
void _fifo_lock( const osfifo_t fifo )
{
	if( fifo->flags & isix_fifo_f_noirq ) {
		_isixp_lock_scheduler();
	} else {
		_isixp_enter_critical();
	}
}

//! Private function for unlock
static inline __attribute__((always_inline)) 
void _fifo_unlock( const osfifo_t fifo )
{
	if( fifo->flags & isix_fifo_f_noirq ) {
		_isixp_unlock_scheduler();
	} else {
		_isixp_exit_critical();
	}
}


#ifdef ISIX_CONFIG_FIFO_EVENT_NOTIFY
#define ISIX_FIFO_EVENT_INVALID_BITS 0xff
/** Private function called when event API notification should be made
 * @param[in] fifo Input fifo handle
 */
void _isixp_fifo_rxavail_event_raise( osfifo_t fifo, bool isr );
#else /*ISIX_CONFIG_FIFO_EVENT_NOTIFY*/
#define _isixp_fifo_rxavail_event_raise(x,y) do {} while(0)
#endif /*ISIX_CONFIG_FIFO_EVENT_NOTIFY*/
