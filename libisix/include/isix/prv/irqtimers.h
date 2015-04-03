/*
 * irqtimers.h
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */

#pragma once
/*-----------------------------------------------------------------------*/
#include <isix/config.h>
#include <isix/types.h>
#include <isix/prv/list.h>
/*-----------------------------------------------------------------------*/
#ifdef ISIX_CONFIG_USE_TIMERS
/*-----------------------------------------------------------------------*/
//Initialize vtimers infrastructure
void _isixp_vtimer_init(void);
/*-----------------------------------------------------------------------*/
//Call timer funcs in the interrupt context
void _isixp_vtimer_handle_time(ostick_t jiffies);
/*-----------------------------------------------------------------------*/
struct isix_vtimer
{
	ostick_t jiffies;	      		  /* Next timeout handle */
	ostick_t timeout;			 	  /* Timeout timer value */
	void (*timer_handler)(void*); /* Next timer call */
	void *arg;					  /* Function pointer */
	bool one_shoot;				  /* Is a one shoot timer */
	list_t inode;				  /* Inode list */
};
/*-----------------------------------------------------------------------*/
#else
static inline void _isixp_vtimer_init(void) {}
static inline void _isixp_vtimer_handle_time(ostick_t jiffies) { (void)jiffies; }
#endif
/*-----------------------------------------------------------------------*/
