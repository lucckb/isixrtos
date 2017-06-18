/*
 * irqtimers.c
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */
// FIXME:  WAit for cancel fix needed
#include <isix/prv/softtimers.h>
#include <isix/softtimers.h>
#include <isix/memory.h>
#include <isix/prv/list.h>
#include <string.h>
#include <isix/prv/scheduler.h>
#include <isix/arch/core.h>

#ifdef CONFIG_ISIX_LOGLEVEL_VTIMERS
#undef CONFIG_ISIX_LOGLEVEL 
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_VTIMERS
#endif
#include <isix/prv/printk.h>

#if CONFIG_ISIX_USE_TIMERS 

//! Global timer CTX structure
static struct vtimer_context tctx;

//! Command enumeration
enum command_e {
	cmd_add = 1,
	cmd_cancel = 2,
	cmd_execnow = 3,
	cmd_delete = 4
};

//! Callable object structure
struct callable_param
{
	osworkfunc_t fun;
	void *arg;
};

//! Start command config
struct start_param {
	osvtimer_t tmr;	//! Timer identifier
	osvtimer_callback callback;	//! Timer callback
	void* args;		//! Callback argument
	ostick_t tout;	//! Timer timeout
	ostick_t itime;	//! Input time
	bool cyclic;
};


//! Command queue
typedef struct command 
{
	int8_t cmd;
	union 
	{
		osvtimer_t generic_args;
		struct callable_param work_args;
		struct start_param start_args;
	};
} command_t;

//! Execute the callback and free busy flag
static inline void exec_timer_callback( osvtimer_t timer ) 
{
	if( timer->callback ) timer->callback( timer->arg );
	if( !timer->cyclic ) {
		atomic_store( &timer->is_active, false );
	}
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
static void add_vtimer_to_list( ostick_t tnow, osvtimer_t timer )
{
	ostick_t currj = tnow;
	timer->jiffies += timer->timeout;
    if( timer->jiffies < currj )
    {
    	//Insert on overflow waiting list in time order
		pr_debug("ovr add %p jiff %08x now %08x", timer, timer->jiffies, tnow );
		add_list_with_prio( tctx.pov_vtimer_list, timer );
    }
    else
    {
    	//Insert on waiting list in time order no overflow
		add_list_with_prio( tctx.p_vtimer_list, timer );
    }
}

//! Handle add to list
static void handle_add( ostick_t tnow, const struct start_param* param )
{
	//Handle and fill the inputs
	osvtimer_t tmr =  param->tmr;
	tmr->jiffies = param->itime;
	tmr->timeout = param->tout;
	tmr->callback = param->callback;
	tmr->arg = param->args;
	tmr->cyclic = param->cyclic;
	atomic_store( &tmr->is_active, true );
	//Check for delayed task 
	bool handled = false;
	ostick_t tdiff = tnow>=tmr->jiffies?tnow-tmr->jiffies:tmr->jiffies-tnow;
	if( tdiff >= tmr->timeout ) 
	{
		exec_timer_callback( tmr );
		handled = true;
	}
	if( !handled ) {
		add_vtimer_to_list( tnow, tmr );
	} else {
		if( tmr->cyclic ) {
			tmr->jiffies = tnow;
			add_vtimer_to_list( tnow, tmr );
		}
	}
}

//Switch timer list if overflow
static void switch_timer_list( ostick_t tnow )
{
	osvtimer_t vtimer,tmp;
	//First execute all remaining task on old list
	list_for_each_entry_safe( tctx.p_vtimer_list, vtimer, tmp, inode )
	{
		list_delete( &vtimer->inode );
		exec_timer_callback( vtimer );
		if( vtimer->cyclic ) 
		{
			vtimer->jiffies = tnow;
			if( tnow > vtimer->jiffies + vtimer->timeout ) {
				add_list_with_prio( tctx.pov_vtimer_list, vtimer );
			}
		}
	}
	//SWAP
	{
		list_entry_t *tmp = tctx.p_vtimer_list;
		tctx.p_vtimer_list = tctx.pov_vtimer_list;
		tctx.pov_vtimer_list = tmp;
	}
}

//! Handle time
static ostick_t handle_time( ostick_t tnow, bool overflow )
{ 
	osvtimer_t vtimer;
	ostick_t ret;
	if( overflow ) {
		switch_timer_list( tnow );
	}
	while( !list_isempty(tctx.p_vtimer_list) && tnow >=
		(vtimer=list_first_entry(tctx.p_vtimer_list,inode,struct isix_vtimer))->jiffies
	)
	{
		exec_timer_callback( vtimer );
		list_delete( &vtimer->inode );
		if( vtimer->cyclic ) 
		{
			vtimer->jiffies = tnow;
			add_vtimer_to_list( tnow, vtimer );
		}
	}
	if( !list_isempty(tctx.p_vtimer_list) ) 
	{
		vtimer = list_first_entry(tctx.p_vtimer_list,inode,struct isix_vtimer);
		ret = vtimer->jiffies - tnow;
		pr_debug("nto %u:%p>%u", tnow,vtimer,ret );
	} 
	else {
		ret = 0U - tnow;
		pr_debug("ntko %u", ret );
	}
	return ret;
}

//! Handle cancelation
static void handle_cancel( osvtimer_t tmr ) 
{
	if( list_is_elem_assigned( &tmr->inode ) ) {
		list_delete( &tmr->inode );
		atomic_store( &tmr->is_active , false );
	}
}


//! Worker thread used for 
static void worker_thread( void* param ) 
{
	(void)param;
	ostick_t tout = 1U;
	ostick_t pjiff = 0;	//Previous jiffies for detect overflow
	for(command_t cmd;;) 
	{ 
		int code = isix_fifo_read(tctx.worker_queue, &cmd, tout);
		if( code == ISIX_EOK ) code = cmd.cmd;
		ostick_t tnow = isix_get_jiffies();
		switch ( code ) 
		{
		case cmd_add:
			handle_cancel( cmd.start_args.tmr );
			handle_add( tnow, &cmd.start_args );
			break;
		case cmd_cancel:
			handle_cancel( cmd.generic_args );
			break;
		case cmd_execnow:
			cmd.work_args.fun( cmd.work_args.arg );
			break;
		case cmd_delete:
			handle_cancel( cmd.generic_args );
			isix_free( cmd.generic_args );
			break;
		case ISIX_ETIMEOUT:
			break;
		default:
			return;
		}
		tout = handle_time( tnow, tnow<pjiff );
		//Previous jiff for detect overflow
		pjiff = tnow;
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
int isix_vtimer_initialize( void )
{
	if( !tctx.worker_thread_id ) 
	{
		list_init( &tctx._vtimer_list_[0] );
		list_init( &tctx._vtimer_list_[1] );
		//Initialize overflow waiting list
		tctx.p_vtimer_list =   &tctx._vtimer_list_[0];
		tctx.pov_vtimer_list = &tctx._vtimer_list_[1];
		//Create worker queue and task
		tctx.worker_queue = 
			isix_fifo_create( CONFIG_ISIX_TIMERS_CMD_QUEUE_SIZE, sizeof(command_t) );
		if( !tctx.worker_queue ) return !tctx.worker_queue;
		tctx.worker_thread_id = isix_task_create( worker_thread, NULL, 
			ISIX_PORT_SCHED_MIN_STACK_DEPTH*4, isix_get_min_priority()/2, 0 );
	}
	return tctx.worker_thread_id?ISIX_EOK:ISIX_ENOMEM;
}


/** Create virtual timer  object
 * @return Virtual timer or null ptr if cannot be created
 */
osvtimer_t isix_vtimer_create( void ) 
{
	//Lazy initialize worker thread (if it was not created)
	if( isix_vtimer_initialize() ) {
		pr_err("Cannot initialize worker thread");
		return NULL;
	}
	osvtimer_t timer = (osvtimer_t)isix_alloc(sizeof(struct isix_vtimer));
	if( !timer ) {
		return timer;
	}
	memset( timer, 0, sizeof(*timer) );
	pr_info("vtimer created %p", timer );
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
int _isixp_vtimer_start( osvtimer_t timer, osvtimer_callback func, 
		void* arg, ostick_t timeout, bool cyclic, bool isr )
{
	pr_info("isix_vtimer_start(tmr: %p time: %u cy: %i)", timer, timeout, cyclic );
	if( !timer ) return ISIX_EINVARG;
	//! Call @ jiffies
	command_t cmd = { 
		.cmd=cmd_add, 
		.start_args = { 
			.tmr = timer,
			.callback = func,
			.args = arg,
			.tout = timeout,
			.itime = isix_get_jiffies(),
			.cyclic = cyclic
		}
	};
	if( schrun ) {
		if( !isr ) {
			return isix_fifo_write( tctx.worker_queue, &cmd, ISIX_TIME_INFINITE );
		} else {
			return isix_fifo_write_isr( tctx.worker_queue, &cmd );
		}
	} else {
		handle_add( isix_get_jiffies(), &cmd.start_args );
		return ISIX_EOK;
	}
}

/** Stop the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int _isixp_vtimer_cancel( osvtimer_t timer, bool isr )
{
	int ret = ISIX_EOK;
	do {
		if( !timer ) {
			ret = ISIX_EINVARG;
			break;
		}
		if( schrun ) {
			command_t cmd = { .cmd=cmd_cancel, .generic_args=timer };
			if( !isr ) {
				ret = isix_fifo_write( tctx.worker_queue, &cmd, ISIX_TIME_INFINITE );
			} else {
				ret = isix_fifo_write_isr( tctx.worker_queue, &cmd );
			}
		} else {
			handle_cancel( timer );
		}
		if( ret ) break;
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
	return atomic_load(&timer->is_active);
}

/** Destroy the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_destroy( osvtimer_t timer )
{
	int ret = ISIX_EOK;
	do {
		if( schrun ) {
			command_t cmd = { .cmd=cmd_delete, .generic_args=timer };
			ret = isix_fifo_write( tctx.worker_queue, &cmd, ISIX_TIME_INFINITE );
			if( ret ) break;
		} else {
			handle_cancel( timer );
			isix_free( timer );
		}
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
	command_t cmd = { .cmd = cmd_execnow, .work_args = {func,arg} };
	return isix_fifo_write_isr( tctx.worker_queue, &cmd );
}

/** Function modify the next timer timeout
 * it should be called only from from the timer
 * callback not from normal context function
 * @param[in] timer to modify
 * @param[in] New timeout or cancel
 */
int isix_vtimer_mod( osvtimer_t timer, ostick_t new_timeout ) 
{
	if( !timer && !timer->cyclic ) {
		return ISIX_EINVARG;
	}
	if( new_timeout == OSVTIMER_CB_CANCEL ) {
	 	timer->cyclic = false;
	} else {
		timer->timeout = new_timeout;
	}
	return ISIX_EOK;
}

#endif /* CONFIG_ISIX_USE_TIMERS */

