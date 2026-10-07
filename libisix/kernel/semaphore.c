#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <isix/semaphore.h>
#include <isix/prv/semaphore.h>
#include <string.h>
#include <isix/arch/sem_atomic.h>
#include <isix/prv/scheduler.h>
#include <isix/assert.h>
#include <isix/prv/test_hooks.h>
#if CONFIG_ISIX_SEM_EVENT_NOTIFY
#include <isix/events.h>
#define ISIX_SEM_EVENT_INVALID_BITS 0xff
#endif


//Note data abort should mark clrex in CM3

#ifdef CONFIG_ISIX_LOGLEVEL_SEMAPHORE
#undef CONFIG_ISIX_LOGLEVEL 
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_SEMAPHORE
#endif
#include <isix/prv/printk.h>

//Create semaphore
ossem_t isix_sem_create_limited( ossem_t sem, int val, int limit_val )
{
    isix_assert_isr();
	const bool static_mem = (sem!=NULL);
	if( limit_val < 0 )
	{
		pr_err("Semaphore invalid limit %i", limit_val );
		return NULL;
	}
	if( sem == NULL )
    {
        sem = (ossem_t)isix_alloc(sizeof(struct isix_semaphore));
        if( sem == NULL ) {
			pr_err("No available memory");
			return NULL;
		}
    }
	memset( sem, 0, sizeof(*sem) );
	sem->static_mem = static_mem;
	_isix_port_atomic_sem_init( &sem->value, val, limit_val );
	list_init( &sem->wait_list );
	pr_info("Create sem %p val %i",sem,(int)sem->value.value);
	return sem;
}

//Wait for semaphore P()
int isix_sem_wait(ossem_t sem, ostick_t timeout)
{
    isix_assert_isr();
	pr_info("sem: Wait task %p on %p tout %i", currp, sem, timeout );
	//TODO: Wait in separate function
    if( !sem ) {
		pr_err("No sem");
		return ISIX_EINVARG;
	}
	if( timeout == ISIX_TIME_DONTWAIT ) {
		return _isix_port_atomic_sem_trydec(&sem->value)>0?ISIX_EOK:ISIX_ETIMEOUT;
	}
	const ostick_t start = isix_get_jiffies();
	for(;;)
	{
		isix_enter_critical();
		//Consistency check
		if( _isix_port_atomic_sem_dec(&sem->value) >= 0 )
		{
			isix_exit_critical();
			return ISIX_EOK;
		}
		ostick_t tout = timeout;
		if( timeout != ISIX_TIME_INFINITE )
		{
			// Remaining time after the wait was interrupted by suspend
			const ostick_t elapsed = isix_get_jiffies() - start;
			if( elapsed >= timeout ) {
				_isixp_sem_fast_signal( sem );
				isix_exit_critical();
				return ISIX_ETIMEOUT;
			}
			tout = timeout - elapsed;
		}
		pr_debug("Add to list %p", currp );
		/* Set before wait-list insert so a tick never sees WTSEM with stale obj.sem. */
		currp->obj.sem = sem;
		currp->wait_aborted = false;
		_isixp_set_sleep_timeout( OSTHR_STATE_WTSEM, tout );
		_isixp_add_to_prio_queue( &sem->wait_list, currp );
		isix_exit_critical();
		isix_yield();
		// Resumed after suspend without a token, wait again
		if( !currp->wait_aborted ) {
			return currp->obj.dmsg;
		}
	}
}

//Sem signal V()
int _isixp_sem_signal( ossem_t sem, bool isr )
{ 
    isix_assert_isr(isr);
	pr_info("sem: Signal on %p isr %i", sem, isr );
	if(!sem) {
        pr_err("No sem");
        return ISIX_EINVARG;
    }
	isix_enter_critical();
	if( _isix_port_atomic_sem_inc( &sem->value ) <= 0 )
    {
		ostask_t task = _isixp_remove_from_prio_queue( &sem->wait_list );
		pr_debug("Task to wakeup %p", task );
		if( task ) {	//Task can be deleted for EX
			if( task->state == OSTHR_STATE_WTSEM ) {
				if( !isr ) _isixp_wakeup_task( task, ISIX_EOK );
				else _isixp_wakeup_task_i( task, ISIX_EOK );
			} else {
				isix_exit_critical();
			}
		} else {
			isix_exit_critical();
		}
		return ISIX_EOK;
    }	
	else
	{
		pr_debug("Waiting list is empty incval to %i",(int)sem->value.value );
#if CONFIG_ISIX_SEM_EVENT_NOTIFY
		// The semaphore may be destroyed right after leaving the critical section
		const osevent_t ev = (osevent_t)atomic_load( (atomic_uintptr_t*)&sem->evt );
		const uint8_t bitno = atomic_load( &sem->bitno );
#endif
		isix_exit_critical();
		ISIX_TEST_POINT( isix_tp_sem_signal_notify, sem );
#if CONFIG_ISIX_SEM_EVENT_NOTIFY
		if( ev && bitno <= 31 ) {
			if( !isr ) isix_event_set( ev, 1U<<bitno );
			else isix_event_set_isr( ev, 1U<<bitno );
		}
#endif
		return ISIX_EOK;
	}
}

//Get semaphore from isr
int isix_sem_trywait(ossem_t sem)
{
    if(!sem) return ISIX_EINVARG;
    return _isix_port_atomic_sem_trydec(&sem->value)>0?ISIX_EOK:ISIX_EBUSY;
}

//! Wakeup semaphore tasks with selected messages
static void sem_wakeup_all( ossem_t sem, osmsg_t msg, bool isr )
{
	ostask_t wkup_task = NULL;
	ostask_t t;
	while( (t=_isixp_remove_from_prio_queue(&sem->wait_list) ) )
	{
		//!Assign first task it is a prioritized list highest first
		if( !wkup_task ) wkup_task = t;
		_isixp_wakeup_task_l( t, msg );
	}
	if( wkup_task && !isr) {
		_isixp_do_reschedule( wkup_task );
	} else {
		_isixp_exit_critical_isr( wkup_task );
	}
}

//Sem value of semaphore
int _isixp_sem_reset( ossem_t sem, int val, bool isr )
{
    isix_assert_isr(isr);
    if( !sem ) { 
		pr_err("No sem");
		return ISIX_EINVARG; 
	}
    //Semaphore is used
    isix_enter_critical();
    _isix_port_atomic_sem_write_val( &sem->value, val );
	sem_wakeup_all( sem, ISIX_ERESET, isr );
    return ISIX_EOK;
}

//Get value of semaphore
int isix_sem_getval(ossem_t sem)
{
    if( !sem ) { 
		pr_err("No sem");
		return ISIX_EINVARG;
	}
    return _isix_port_atomic_sem_read_val( &sem->value );
}

//Sem destroy
int isix_sem_destroy(ossem_t sem)
{
    isix_assert_isr();
	if( !sem ) {
		pr_err("No sem");
		return ISIX_EINVARG;
	}
	//! Semaphore is used
	isix_enter_critical();
	sem_wakeup_all( sem, ISIX_EDESTROY, false );
	if(!sem->static_mem) isix_free(sem);
	return ISIX_EOK;
}


#if CONFIG_ISIX_SEM_EVENT_NOTIFY

int isix_sem_event_connect( ossem_t sem, osevent_t evt, int bit )
{
	isix_assert_isr();
	if( !sem || !evt || bit < 0 || bit > 31 ) {
		return ISIX_EINVARG;
	}
	uintptr_t expected = 0;
	if( !atomic_compare_exchange_strong((atomic_uintptr_t*)&sem->evt,
				&expected, (uintptr_t)evt) ) {
		return ISIX_EBUSY;
	}
	atomic_store( &sem->bitno, bit );
	return ISIX_EOK;
}

int isix_sem_event_disconnect( ossem_t sem, osevent_t evt )
{
	isix_assert_isr();
	if( !sem || !evt ) {
		return ISIX_EINVARG;
	}
	uintptr_t expected = (uintptr_t)evt;
	if( !atomic_compare_exchange_strong((atomic_uintptr_t*)&sem->evt,
				&expected, (uintptr_t)NULL) ) {
		return ISIX_EBUSY;
	}
	atomic_store( &sem->bitno, ISIX_SEM_EVENT_INVALID_BITS );
	return ISIX_EOK;
}

#endif /* CONFIG_ISIX_SEM_EVENT_NOTIFY */
