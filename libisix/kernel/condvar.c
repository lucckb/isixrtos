#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <isix/condvar.h>
#include <isix/prv/condvar.h>
#include <isix/scheduler.h>
#include <isix/prv/scheduler.h>
#include <isix/prv/mutex.h>

#ifdef CONFIG_ISIX_LOGLEVEL_CONDVAR
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_CONDVAR
#endif
#include <isix/prv/printk.h>

// Condvar create
oscondvar_t isix_condvar_create(void)
{
	oscondvar_t cv = (oscondvar_t)isix_alloc(sizeof(struct isix_condvar));
	if( cv ) {
		list_init( &cv->wait_list );
	}
	return cv;
}

// Condvar signal single thread
int _isixp_condvar_signal( oscondvar_t cv, bool isr )
{
	if( !cv ) {
		pr_err( "Invalid condvar " );
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	if( !list_isempty( &cv->wait_list ) )
	{
		ostask_t task = _isixp_remove_from_prio_queue( &cv->wait_list );
		if( task->state == OSTHR_STATE_WTCOND )
		{
			if( !isr ) {
				_isixp_wakeup_task( task, ISIX_EOK );
			} else {
				_isixp_wakeup_task_i( task, ISIX_EOK );
			}
		}
		else
		{
			isix_bug("Invalid state task->state != OSTHR_STATE_WTCOND" );
		}
	}
	else
	{
		isix_exit_critical();
	}
	return ISIX_EOK;
}

// Condvar broadcast
static inline void condvar_broadcast_msg( oscondvar_t cv, osmsg_t msg, bool isr )
{
	isix_enter_critical();
	ostask_t wkup_task = NULL;
	ostask_t t;
	while( (t=_isixp_remove_from_prio_queue(&cv->wait_list) ) )
	{
		//!Assign first task it is a prioritized list highest first
		if( !wkup_task ) wkup_task = t;
		_isixp_wakeup_task_l( t, msg );
	}
	if( wkup_task && !isr) {
		_isixp_do_reschedule( wkup_task );
	} else {
		isix_exit_critical();
	}
}

int _isixp_condvar_broadcast( oscondvar_t cv, bool isr )
{
	if( !cv ) {
		pr_err( "Invalid condvar " );
		return ISIX_EINVARG;
	}
	condvar_broadcast_msg( cv, ISIX_EOK, isr );
	return ISIX_EOK;
}

//! Condition variable wait
int isix_condvar_wait( oscondvar_t cv, ostick_t timeout )
{
	if( !cv ) {
		pr_err( "Invalid condvar " );
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	//Get the current mutex
	osmtx_t mtx = _isixp_get_top_currt_mutex();
	int ret;
	if( (ret=isix_mutex_unlock(mtx)) ) {
		isix_exit_critical();
		return ret;
	}
	pr_debug("Add to list %p", currp );
	_isixp_set_sleep_timeout( OSTHR_STATE_WTCOND, timeout );
	_isixp_add_to_prio_queue( &cv->wait_list, currp );
	currp->obj.cond = cv;
	isix_exit_critical();
	isix_yield();
	isix_enter_critical();
	ret =  currp->obj.dmsg;
	if( ret == ISIX_EOK  ) {
		//NOTE: Lock again only properly unlocked mutex
		ret = isix_mutex_lock( mtx );
		if( !ret ) ret = currp->obj.dmsg;
	}
	isix_exit_critical();
	return ret;
}


//! Destroy the condvar
int isix_condvar_destroy( oscondvar_t cv )
{
	if( !cv ) {
		pr_err( "Invalid condvar " );
		return ISIX_EINVARG;
	}
	condvar_broadcast_msg( cv, ISIX_EDESTROY, false );
	isix_free( cv );
	return ISIX_EOK;
}

