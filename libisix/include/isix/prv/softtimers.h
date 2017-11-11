/*
 * irqtimers.h
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */

#pragma once
#ifndef _ISIX_KERNEL_CORE_
#	error This is private header isix kernel headers cannot be used by app
#endif

#include <isix/config.h>
#include <isix/types.h>
#include <isix/prv/list.h>
#include <isix/task.h>
#include <isix/fifo.h>
#include <stdatomic.h>

#if CONFIG_ISIX_USE_TIMERS

//! Default queue size
#ifndef CONFIG_ISIX_TIMERS_CMD_QUEUE_SIZE 
#define CONFIG_ISIX_TIMERS_CMD_QUEUE_SIZE 16
#endif


struct isix_vtimer {
	ostick_t jiffies;	      	  		/* Next timeout handle */
	ostick_t timeout;			  		/* Timeout timer value */
	void (*callback)(void*); 			/* Next timer call */
	void *arg;					  		/* Function pointer */
	list_t inode;				  		/* Inode list */
	bool cyclic;						/* If timer is cyclic */
	atomic_bool is_active;				/* Is timer currently active */
};

struct vtimer_context {
	//List entry for the virtual timers
	list_entry_t _vtimer_list_[2];
	//Overflowed and not overflowed list
	list_entry_t* p_vtimer_list;	//Normal list 
	list_entry_t* pov_vtimer_list;	//Overflow list
	ostask_t worker_thread_id;
	osfifo_t worker_queue;
} ;

//Finalize function
void _isixp_vtimers_finalize();

#else /*  CONFIG_ISIX_USE_TIMERS */

static inline void __attribute__((always_inline))
	_isixp_vtimers_finalize()
{

}

#endif /* CONFIG_ISIX_USE_TIMERS */
