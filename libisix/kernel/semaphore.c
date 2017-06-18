#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <isix/semaphore.h>
#include <isix/prv/semaphore.h>
#include <string.h>
#include <isix/arch/sem_atomic.h>
#include <isix/prv/scheduler.h>


//Note data abort should mark clrex in CM3

#ifdef CONFIG_ISIX_LOGLEVEL_SEMAPHORE
#undef CONFIG_ISIX_LOGLEVEL 
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_SEMAPHORE
#endif
#include <isix/prv/printk.h>

//Create semaphore
ossem_t isix_sem_create_limited( ossem_t sem, int val, int limit_val )
{
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
	pr_info("sem: Wait task %p on %p tout %i", currp, sem, timeout );
	//TODO: Wait in separate function
    if( !sem ) {
		pr_err("No sem");
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	//Consistency check
	if( _isix_port_atomic_sem_dec(&sem->value) < 0 )
    {
		pr_debug("Add to list %p", currp );
		_isixp_set_sleep_timeout( OSTHR_STATE_WTSEM, timeout );
		_isixp_add_to_prio_queue( &sem->wait_list, currp );
		currp->obj.sem = sem;
		isix_exit_critical();
		isix_yield();
		return currp->obj.dmsg;
    }
	else
	{
		isix_exit_critical();
		return ISIX_EOK;
	}
}

//Sem signal V()
int _isixp_sem_signal( ossem_t sem, bool isr )
{ 
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
		isix_exit_critical();
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
		isix_exit_critical();
	}
}

//Sem value of semaphore
int _isixp_sem_reset( ossem_t sem, int val, bool isr )
{
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

