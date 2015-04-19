#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <isix/semaphore.h>
#include <isix/prv/semaphore.h>
#include <string.h>
#include <isix/port_atomic.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>

#ifndef ISIX_DEBUG_SEMAPHORE
#define ISIX_DEBUG_SEMAPHORE ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_SEMAPHORE == ISIX_DBG_ON
#include <isix/printk.h>
#else
#undef printk
#define printk(...) do {} while(0)
#endif

//Create semaphore
ossem_t isix_sem_create_limited( ossem_t sem, int val, int limit_val )
{
	const bool static_mem = (sem!=NULL);
	if(limit_val<0)
	{
		return NULL;
	}
	if(sem==NULL)
    {
        sem = (ossem_t)isix_alloc(sizeof(struct isix_semaphore));
        if(sem==NULL) return NULL;
    }
    memset( sem, 0, sizeof(*sem) );
    sem->static_mem = static_mem;
	port_atomic_sem_init( &sem->value, val, limit_val );
    list_init( &sem->wait_list );
    printk("Create sem %p val %i",sem,(int)sem->value.value);
    return sem;
}

//Wait for semaphore P()
//TODO: priority inheritance
int isix_sem_wait(ossem_t sem, ostick_t timeout)
{
	printk("sem: Wait task %p on %p tout %i", currp, sem, timeout );
	//TODO: Wait in separate function
    if( !sem ) {
		printk("No sem");
		return ISIX_EINVARG;
	}
	_isixp_enter_critical();
	//Consistency check
	if( port_atomic_sem_dec(&sem->value) > 0 )
    {
        //printk("-----  dec value %i",(int)sem->value.value);
		_isixp_exit_critical();
        return ISIX_EOK;
    }
	//printk("---- Add to list %p----- ", currp );
	_isixp_set_sleep_timeout( OSTHR_STATE_WTSEM, timeout ); 
	_isixp_add_to_prio_queue( &sem->wait_list, currp );
	currp->obj.sem = sem;
	_isixp_exit_critical();
	isix_yield();
	return currp->obj.dmsg;
}

//Sem signal V()
int _isixp_sem_signal( ossem_t sem, bool isr )
{ 
	printk("sem: Signal on %p isr %i", sem, isr );
	if(!sem) {
        printk("No sem");
        return ISIX_EINVARG;
    }
	_isixp_enter_critical();
	if( port_atomic_sem_inc( &sem->value ) > 1 )
    {
        //printk("Waiting list is empty incval to %i",(int)sem->value.value );
		_isixp_exit_critical();
        return ISIX_EOK;
    }	
	ostask_t task = _isixp_remove_from_prio_queue( &sem->wait_list );
	//printk("Task to wakeup %p", task );
	if( task ) {	//Task can be deleted for EX
		//Decrement again because are thrd on list
		port_atomic_sem_dec( &sem->value );
		if( task->state == OSTHR_STATE_WTSEM ) {
			if( !isr ) _isixp_wakeup_task( task, ISIX_EOK );
			else _isixp_wakeup_task_i( task, ISIX_EOK );
		} else {
			_isixp_exit_critical();
		}
	} else {
		_isixp_exit_critical();
	}
	return ISIX_EOK;
}

//Get semaphore from isr
int isix_sem_get_isr(ossem_t sem)
{
    if(!sem) return ISIX_EINVARG;
    int res = ISIX_EBUSY;
    if( sem && port_atomic_sem_dec(&sem->value) ) 
    {
        res = ISIX_EOK;
    }
    return res;
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
	if( !isr ) {
		_isixp_do_reschedule( wkup_task );
	} else {
		_isixp_exit_critical();
	}
}

//Sem value of semaphore
int _isixp_sem_reset( ossem_t sem, int val, bool isr )
{
    if( !sem ) return ISIX_EINVARG;
	if( port_atomic_sem_read_val(&sem->value) < 0 )  {
		return ISIX_EINVARG;
	}
    //Semaphore is used
    _isixp_enter_critical();
    port_atomic_sem_write_val( &sem->value, val );
	sem_wakeup_all( sem, ISIX_ERESET, isr );
    return ISIX_EOK;
}

//Get value of semaphore
int isix_sem_getval(ossem_t sem)
{
    if(!sem) return ISIX_EINVARG;
    return port_atomic_sem_read_val( &sem->value );
}

//Sem destroy
int isix_sem_destroy(ossem_t sem)
{
   if(!sem) return ISIX_EINVARG;
    //! Semaphore is used
   _isixp_enter_critical();
	sem_wakeup_all( sem, ISIX_EDESTROY, false );
   if(!sem->static_mem) isix_free(sem);
   return ISIX_EOK;
}

