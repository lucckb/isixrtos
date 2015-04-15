/*
 * irqtimers.c
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */

#include <isix/prv/softtimers.h>
#include <isix/printk.h>
#include <isix/softtimers.h>
#include <isix/memory.h>
#include <isix/prv/list.h>
#include <string.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>

#ifdef ISIX_CONFIG_USE_TIMERS 

//! Global timer CTX structure
static struct vtimer_context tctx;

//! Command enumeration
enum command_e {
	cmd_add = 1,
	cmd_cancel = 2,
	cmd_execnow = 3
};

struct callable 
{
	osworkfunc_t fun;
	void *arg;
};
//! Command queue
typedef struct command 
{
	uint8_t cmd;
	union 
	{
		osvtimer_t tmr;
		struct callable work;
	};
} command_t;

//! Execute the callback and free busy flag
static inline void exec_timer_callback( osvtimer_t timer ) 
{
	if( timer->callback ) timer->callback( timer->arg );
	isix_sem_signal_isr( &timer->busy );
	//printk(">>>>>>>Execcallback(%p) @%u", timer,  isix_get_jiffies() );
}

//! Add to list with task order
static void add_list_with_prio( list_entry_t* list, osvtimer_t timer ) 
{
	//Insert on overflow waiting list in time order
	osvtimer_t waitl;
	list_for_each_entry(list,waitl,inode)
	{
		if( timer->jiffies < waitl->jiffies ) break;
	}
	list_insert_before( &waitl->inode, &timer->inode );
}

/** Function add timeout to list
 * @return How long should wait
 */
static void add_vtimer_to_list( osvtimer_t timer )
{
	ostick_t currj = isix_get_jiffies();
	timer->jiffies += timer->timeout;
	//printk("add_vtimer_to_list( tmr %p wake@ %u curr %u)", timer, timer->jiffies, currj );
    if( timer->jiffies < currj )
    {
    	//Insert on overflow waiting list in time order
		add_list_with_prio( tctx.pov_vtimer_list, timer );
    }
    else
    {
    	//Insert on waiting list in time order no overflow
		add_list_with_prio( tctx.vtimer_list, timer );
    }
}

//! Handle add to list
static void handle_add( osvtimer_t tmr, bool overflow )
{
	//Check for delayed task 
	bool handled = false;
	if( !overflow ) {
		if( isix_get_jiffies() >= tmr->jiffies+tmr->timeout ) {
			exec_timer_callback( tmr );
			handled = true;
		}
	} else {
		if( isix_get_jiffies() <= tmr->jiffies+tmr->timeout ) {
			exec_timer_callback( tmr );
			handled = true;
		}
	}
	if( handled && tmr->cyclic ) {
		tmr->jiffies = isix_get_jiffies();
		add_vtimer_to_list( tmr );
	}
	if( !handled ) {
		add_vtimer_to_list( tmr );
	}
	//printk("Timer %p ADD handled %i overflow %i", tmr, handled, overflow );
}

//! Handle time
static ostick_t handle_time( bool overflow )
{ 
	osvtimer_t vtimer;
	ostick_t currj = isix_get_jiffies();
	if( overflow )
	{
		//First execute all remaining task on old list
		osvtimer_t tmp;
		list_for_each_entry_safe( tctx.p_vtimer_list, vtimer, tmp, inode )
		{
			list_delete( &vtimer->inode );
			exec_timer_callback( vtimer );
			if( vtimer->cyclic ) 
				add_list_with_prio( tctx.pov_vtimer_list, vtimer );
		}
		//SWAP
		{
			list_entry_t *tmp = tctx.p_vtimer_list;
			tctx.p_vtimer_list = tctx.pov_vtimer_list;
			tctx.pov_vtimer_list = tmp;
		}
	}
	while( !list_isempty(tctx.p_vtimer_list) && currj >=
		(vtimer=list_first_entry(tctx.p_vtimer_list,inode,struct isix_vtimer))->jiffies
	)
	{
		list_delete( &vtimer->inode );
		exec_timer_callback( vtimer );
		if( vtimer->cyclic ) {
			vtimer->jiffies = isix_get_jiffies();
			add_vtimer_to_list( vtimer );
		}
	}
	ostick_t ret;
	if( !list_isempty(tctx.p_vtimer_list) ) 
	{
		vtimer = list_first_entry(tctx.p_vtimer_list,inode,struct isix_vtimer);
		ret = vtimer->jiffies - isix_get_jiffies();
	} 
	else {
		ret = (0U - currj) +1U;
	}
	//printk("nto>%u", ret );
	return ret;
}

//! Handle cancelation
static void handle_cancel( osvtimer_t tmr ) 
{
	if( list_is_elem_assigned( &tmr->inode ) ) {
		list_delete( &tmr->inode );
		isix_sem_signal_isr( &tmr->busy );
	}
}


//! Worker thread used for 
static void worker_thread( void* param ) 
{
	(void)param;
	ostick_t tout = (0U - isix_get_jiffies())+1U;
	ostick_t pjiff = 0;	//Previous jiffies for detect overflow
	for(command_t cmd;;) 
	{ 
		int code = isix_fifo_read(tctx.worker_queue, &cmd, tout);
		if( code == ISIX_EOK ) code = cmd.cmd;
		switch ( code ) 
		{
		case cmd_add:
			handle_add( cmd.tmr, isix_get_jiffies()<pjiff );
			break;
		case cmd_cancel:
			handle_cancel( cmd.tmr );
			break;
		case cmd_execnow:
			cmd.work.fun( cmd.work.arg );
			break;
		case ISIX_ETIMEOUT:
			break;
		default:
			return;
		}
		tout = handle_time( isix_get_jiffies()<pjiff );
		//Previous jiff for detect overflow
		pjiff = isix_get_jiffies();
	}
}

//! Called when function exits
void _isixp_vtimers_finalize() 
{
	if( tctx.worker_thread_id ) {
		isix_task_kill( tctx.worker_thread_id );
		isix_fifo_destroy( tctx.worker_queue );
	}
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
		//Create worker queue and task
		tctx.worker_queue = 
			isix_fifo_create( ISIX_CONFIG_TIMERS_CMD_QUEUE_SIZE, sizeof(command_t) );
		if( !tctx.worker_queue ) return !tctx.worker_queue;
		//FIXME that
		tctx.worker_thread_id = isix_task_create( worker_thread, NULL, 
			ISIX_PORT_SCHED_MIN_STACK_DEPTH*3, isix_get_min_priority()/2, 0 );
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
		printk("Cannot initialize worker thread");
		return NULL;
	}
	osvtimer_t timer = (osvtimer_t)isix_alloc(sizeof(struct isix_vtimer));
	if( !timer ) {
		return timer;
	}
	memset( timer, 0, sizeof(*timer) );
	isix_sem_create_limited( &timer->busy, 1, 1 );
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
	//printk("isix_vtimer_start(tmr: %p time: %u cy: %i)", timer, timeout, cyclic );
	if( !timer ) return ISIX_EINVARG;
	if( isix_sem_get_isr(&timer->busy) ) {
		//!Element is already assigned
		return ISIX_EBUSY;
	}
	//! Call @ jiffies
	timer->jiffies = isix_get_jiffies();
	timer->timeout = timeout;
	timer->callback = func;
	timer->arg = arg;
	timer->cyclic = cyclic;
	command_t cmd = { .cmd=cmd_add, .tmr=timer };
	return isix_fifo_write( tctx.worker_queue, &cmd, ISIX_TIME_INFINITE );
}

/** Stop the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_cancel( osvtimer_t timer ) 
{
	int ret = ISIX_EOK;
	do {
		if( !timer ) 
		{
			ret = ISIX_EINVARG;
			break;
		}
		command_t cmd = { .cmd=cmd_cancel, .tmr=timer };
		ret = isix_fifo_write( tctx.worker_queue, &cmd, ISIX_TIME_INFINITE );
		if( ret ) break;
		ret = isix_sem_wait( &timer->busy, ISIX_TIME_INFINITE );
	} while(0);
	return ret;
}


/** Check if timer is still active and it waits for an event
 * @param[in] timer TImer handler
 * @return Timer state true if active false if not or error
 * code if returned value is negative */
int isix_vtimer_is_active( osvtimer_t timer )
{
	if( !timer ) {
		return ISIX_EINVARG;
	}
	return isix_sem_getval(&timer->busy);
}

/** Destroy the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_destroy( osvtimer_t timer )
{
	int ret = ISIX_EOK;
	do {
		ret = isix_vtimer_cancel( timer );
		if( ret ) break;
		isix_free( timer );
	} while(0);
	return ret;
}


/** Only one function schedule task from an interrupt or 
 * other task context for delayed execution 
 * @param[in] func Function to call
 * @param[in[ arg Functon argument
 */
int isix_schedule_work_isr( osworkfunc_t func, void* arg )
{
	command_t cmd = { .cmd = cmd_execnow, .work = {func,arg} };
	return isix_fifo_write_isr( tctx.worker_queue, &cmd );
}


#endif /* ISIX_CONFIG_USE_TIMERS */

