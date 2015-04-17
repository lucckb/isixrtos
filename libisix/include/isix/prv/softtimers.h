/*
 * irqtimers.h
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */

#pragma once
#include <isix/config.h>
#include <isix/types.h>
#include <isix/prv/list.h>
#include <isix/prv/semaphore.h>
#include <isix/task.h>
#include <isix/fifo.h>

#ifdef ISIX_CONFIG_USE_TIMERS

//! Default queue size
#ifndef ISIX_CONFIG_TIMERS_CMD_QUEUE_SIZE 
#define ISIX_CONFIG_TIMERS_CMD_QUEUE_SIZE 8
#endif

struct isix_vtimer {
	ostick_t jiffies;	      	  		/* Next timeout handle */
	ostick_t timeout;			  		/* Timeout timer value */
	void (*callback)(void*); 			/* Next timer call */
	void *arg;					  		/* Function pointer */
	list_t inode;				  		/* Inode list */
	struct isix_semaphore busy;	  		/* busy sem */
	bool cyclic;						/* If timer is cyclic */
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

#else /*  ISIX_CONFIG_USE_TIMERS */

static inline void __attribute__((always_inline))
	_isixp_vtimers_finalize()
{

}

#endif /* ISIX_CONFIG_USE_TIMERS */
/*-----------------------------------------------------------------------*/
