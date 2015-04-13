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
#include <isix/task.h>
#include <isix/fifo.h>

#ifdef ISIX_CONFIG_USE_TIMERS

struct isix_vtimer
{
	ostick_t jiffies;	      		  /* Next timeout handle */
	ostick_t timeout;			 	  /* Timeout timer value */
	void (*timer_handler)(void*); /* Next timer call */
	void *arg;					  /* Function pointer */
	bool one_shoot;				  /* Is a one shoot timer */
	list_t inode;				  /* Inode list */
};

struct vtimer_context {
	//List entry for the virtual timers
	list_entry_t vtimer_list[2];
	//Overflowed and not overflowed list
	list_entry_t *p_vtimer_list;
	list_entry_t *pov_vtimer_list;
	ostask_t worker_thread_id;
	osfifo_t worker_queue;
} ;

#endif
/*-----------------------------------------------------------------------*/
