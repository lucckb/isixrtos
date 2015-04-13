/*
 * irqtimers.c
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */

#include <isix/prv/softtimers.h>
#include <isix/softtimers.h>
#include <isix/memory.h>
#include <isix/prv/list.h>
#include <string.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>

#ifdef ISIX_CONFIG_USE_TIMERS 

//! Global timer CTX structure
static struct vtimer_context tctx;

//! Worker thread used for 
static void worker_thread( void* param ) 
{
	(void)param;
}

//! Lazy initializatize worker thread only if feature is used
static bool lazy_initalize()
{
	if( !tctx.worker_thread_id ) 
	{
		list_init( &tctx.vtimer_list[0] );
		list_init( &tctx.vtimer_list[1] );
		//Initialize overflow waiting list
		tctx.p_vtimer_list =   &tctx.vtimer_list[0];
		tctx.pov_vtimer_list = &tctx.vtimer_list[1];
		tctx.worker_thread_id = isix_task_create( worker_thread, NULL, 
			ISIX_PORT_SCHED_MIN_STACK_DEPTH*2, isix_get_min_priority()/2, 0 );
	}
	return !tctx.worker_thread_id;
}

/** Create virtual timer  object
 * @return Virtual timer or null ptr if cannot be created
 */
osvtimer_t isix_vtimer_create( void ) 
{
	//Lazy initialize worker thread (if it was not created)
	if( lazy_initalize() ) {
		return NULL;
	}
	osvtimer_t timer = (osvtimer_t)isix_alloc(sizeof(struct isix_vtimer));
	memset( timer, 0, sizeof(*timer) );
	if( !timer ) return timer;
	return timer;
}

/**
 * Start one shoot timer execution
 * @param timer	Pointer to the timer structure
 * @param fun Function to be called 
 * @param arg Argument passed to the function
 * @param timeout Timeout
 * @param[in] Cyclic is cyclic timer
 * @return success if ISIX_EOK else isix error
 */
int isix_vtimer_start( osvtimer_t timer, osvtimer_callback func, 
		void* arg, ostick_t timeout, bool cyclic )
{
	(void)timer; (void)func; (void)arg;	(void)timeout; (void)cyclic;
	return -1;
}

/** Stop the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_cancel( osvtimer_t timer ) 
{
	(void)timer;
	return -1;
}


/** Check if timer is still active and it waits for an event
 * @param[in] timer TImer handler
 * @return Timer state true if active false if not or error
 * code if returned value is negative */
int isix_vtimer_is_active( osvtimer_t timer )
{
	(void)timer;
	return -1;
}


/** Destroy the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_destroy( osvtimer_t timer )
{
	(void)timer;
	return -1;
}


/** Only one function schedule task from an interrupt or 
 * other task context for delayed execution 
 * @param[in] func Function to call
 * @param[in[ arg Functon argument
 */
int isix_schedule_work_isr( osworkfunc_t func, void* arg )
{
	(void)func; (void)arg;
	return -1;
}


/** Schedule task at selected time
 * @param[in] timer Timer handle 
 * @param[in] func  Function to execute
 * @param[in] arg   Function argument 
 * @param[in] time  Schedule wait time
 */
int isix_schedule_delayed_work_isr( osvtimer_t timer, 
		osworkfunc_t func, void* arg, ostick_t wtime ) 
{
	(void)timer; (void)func; (void)arg; (void)wtime;
	return -1;
}


#endif /* ISIX_CONFIG_USE_TIMERS */

