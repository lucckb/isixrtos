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

enum command_e {
	cmd_add = 1,
	cmd_cancel = 2,
	cmd_destroy = 3,
	cmd_execnow = 4
};
//! Command queue
typedef struct command 
{
	uint8_t cmd;
	osvtimer_t tmr;
} command_t;

//Add data to the timer list
static void add_vtimer_to_list( osvtimer_t timer )
{
    timer->jiffies = isix_get_jiffies() + timer->timeout;
    if(timer->jiffies < isix_get_jiffies())
    {
    	//Insert on overflow waiting list in time order
    	osvtimer_t waitl;
    	list_for_each_entry(tctx.pov_vtimer_list,waitl,inode)
    	{
    	   if(timer->jiffies<waitl->jiffies) break;
    	}
    	list_insert_before(&waitl->inode,&timer->inode);
    }
    else
    {
    	//Insert on waiting list in time order no overflow
    	osvtimer_t waitl;
    	list_for_each_entry(tctx.p_vtimer_list,waitl,inode)
    	{
    	    if(timer->jiffies<waitl->jiffies) break;
    	}
    	list_insert_before(&waitl->inode,&timer->inode);
    }
}

//! Worker thread used for 
static void worker_thread( void* param ) 
{
	(void)param;
	ostick_t tout = ISIX_TIME_INFINITE;
	for(command_t cmd;;) 
	{
		switch ( isix_fifo_read(tctx.worker_queue, &cmd, tout) ) 
		{
		case cmd_add:
			break;
		case cmd_cancel:
			break;
		case ISIX_ETIMEOUT:
			break;
		default:
			return;
		}
		isix_wait_ms(50);
		printk("Command executed %i", cmd.cmd );
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
		tctx.worker_thread_id = isix_task_create( worker_thread, NULL, 
			ISIX_PORT_SCHED_MIN_STACK_DEPTH*2, isix_get_min_priority()/2, 0 );
		printk( "Lazy initialization workthr %p fifo %p", 
				tctx.worker_thread_id, tctx.worker_queue );
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
	isix_sem_create_limited( &timer->exit, 1, 1 );
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
	if( !timer ) {
		return ISIX_EINVARG;
	}
	if( isix_sem_get_isr(&timer->exit) ) {
		//!Element is already assigned
		return ISIX_EBUSY;
	}
	timer->jiffies = isix_get_jiffies() + timeout;
	timer->timeout = cyclic?timeout:0;
	timer->timer_handler = func;
	timer->arg = arg;
	timer->sa_exec = false;
	command_t cmd = { cmd_add, timer };
	return isix_fifo_write( tctx.worker_queue, &cmd, ISIX_TIME_INFINITE );
}

/** Stop the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_cancel( osvtimer_t timer ) 
{
	if( !timer ) {
		return ISIX_EINVARG;
	}
	command_t cmd = { cmd_cancel, timer };
	return isix_fifo_write( tctx.worker_queue, &cmd, ISIX_TIME_INFINITE );
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
	return isix_sem_getval(&timer->exit);
}

/** Destroy the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_destroy( osvtimer_t timer )
{
	int ret = ISIX_EOK;
	do {
		if( !timer ) {
			ret = ISIX_EINVARG;
			break;
		}
		if( schrun ) {
			command_t cmd = { cmd_destroy, timer };
			ret = isix_fifo_write( tctx.worker_queue, &cmd, ISIX_TIME_INFINITE );
			if( ret ) break;
			ret = isix_sem_wait( &timer->exit, ISIX_TIME_INFINITE );
			if( ret ) break;
		} 
		else {
			isix_free( timer );
		}
	} while(0);
	return ret;
}


/** Only one function schedule task from an interrupt or 
 * other task context for delayed execution 
 * @param[in] func  Function to execute
 * @param[in] func Function to call
 * @param[in[ arg Functon argument
 */
int isix_schedule_work_isr( osvtimer_t timer, osworkfunc_t func, void* arg )
{
	if( !timer ) {
		return ISIX_EINVARG;
	}
	if( isix_sem_get_isr(&timer->exit) ) {
		//!Element is already assigned
		return ISIX_EBUSY;
	}
	timer->timer_handler = func;
	timer->arg = arg;
	timer->sa_exec = true;
	return ISIX_EOK;
}


#endif /* ISIX_CONFIG_USE_TIMERS */

