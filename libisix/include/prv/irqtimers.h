/*
 * irqtimers.h
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */

#ifndef ISIX_PRV_IRQTIMERS_H_
#define ISIX_PRV_IRQTIMERS_H_
/*-----------------------------------------------------------------------*/
#include <isix/config.h>
#include <isix/types.h>
#include <prv/list.h>
/*-----------------------------------------------------------------------*/
#if ISIX_CONFIG_USE_TIMERS == ISIX_ON
/*-----------------------------------------------------------------------*/
//Initialize vtimers infrastructure
void _isixp_vtimer_init(void);
/*-----------------------------------------------------------------------*/
//Call timer funcs in the interrupt context
void _isixp_vtimer_handle_time(tick_t jiffies);
/*-----------------------------------------------------------------------*/
struct vtimer_struct
{
	tick_t jiffies;	      /* Next timeout handle */
	tick_t timeout;			 /* Timeout timer value */
	void (*timer_handler)(void*); /* Next timer call */
	void *arg;					  /* Function pointer */
	list_t inode;				  /* Innode list */
};
/*-----------------------------------------------------------------------*/
#else
static inline void _isixp_vtimer_init(void) {}
static inline void _isixp_vtimer_handle_time(tick_t jiffies) { (void)jiffies; }
#endif
/*-----------------------------------------------------------------------*/
#endif /* IRQTIMERS_H_ */
