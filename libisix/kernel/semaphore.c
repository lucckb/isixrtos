#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <isix/prv/scheduler.h>
#include <isix/semaphore.h>
#include <isix/prv/semaphore.h>
#include <string.h>
#include <isix/port_atomic.h>

#ifndef ISIX_DEBUG_SEMAPHORE
#define ISIX_DEBUG_SEMAPHORE ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_SEMAPHORE == ISIX_DBG_ON
#include <isix/printk.h>
#else
#undef isix_printk
#define isix_printk(...) do {} while(0)
#endif

//Current task simple def
#define currp _isix_current_task
/*--------------------------------------------------------------*/
//Create semaphore
sem_t* isix_sem_create_limited(sem_t *sem, int val, int limit_val)
{
	const bool static_mem = (sem!=NULL);
	if(limit_val<0)
	{
		return NULL;
	}
	if(sem==NULL)
    {
        sem = (sem_t*)isix_alloc(sizeof(sem_t));
        if(sem==NULL) return NULL;
    }
    memset(sem,0,sizeof(sem_t));
    sem->static_mem = static_mem;
	port_atomic_sem_init(&sem->value, val, limit_val );
    list_init(&sem->sem_task);
    isix_printk("Create sem %08x val %d",sem,sem->value);
    return sem;
}

/*--------------------------------------------------------------*/
//Wait for semaphore P()
//TODO: priority inheritance
int isix_sem_wait(sem_t *sem, tick_t timeout)
{
	//TODO: Wait in separate function
    if( !sem ) {
		isix_printk("No sem");
		return ISIX_EINVARG;
	}
	_isixp_enter_critical();
	//Consistency check
	if( port_atomic_sem_dec(&sem->value) > 0 )
    {
        isix_printk("Decrement value %d",sem->value->value);
		_isixp_exit_critical();
        return ISIX_EOK;
    }
	currp->obj.sem = sem;
	_isixp_add_to_prio_queue( &sem->sem_task, currp );
	int err = _isixp_goto_sleep_timeout( THR_STATE_WTSEM, timeout ); 
	_isixp_exit_critical();
	return err;
}
/*--------------------------------------------------------------*/
//Sem signal V()
int _isixp_sem_signal( sem_t *sem, bool isr )
{ 
	if(!sem) {
        isix_printk("No sem");
        return ISIX_EINVARG;
    }
	_isixp_enter_critical();
	if( port_atomic_sem_inc( &sem->value ) > 1 )
    {
        isix_printk("Waiting list is empty incval to %d",sem->value);
		_isixp_exit_critical();
        return ISIX_EOK;
    }
	if( list_isempty(&sem->sem_task) )
	{
		isix_bug("List is empty. Atomic wait count mismatch");
	}
	//Decrement again
	port_atomic_sem_dec( &sem->value );
	isix_printk("Task to wakeup %08x",task_wake);
	task_t* task = _isixp_remove_from_prio_queue( &sem->sem_task );
	port_atomic_sem_dec( &sem->value );
	if( !isr ) _isixp_wakeup_task( task, ISIX_EOK );
	else _isixp_wakeup_task( task, ISIX_EOK );
	return ISIX_EOK;
}
/*--------------------------------------------------------------*/
//Get semaphore from isr
int isix_sem_get_isr(sem_t *sem)
{
    if(!sem) return ISIX_EINVARG;
    int res = ISIX_EBUSY;
    if( sem && port_atomic_sem_dec(&sem->value) ) 
    {
        res = ISIX_EOK;
    }
    return res;
}

/*--------------------------------------------------------------*/
//Sem value of semaphore
int isix_sem_setval(sem_t *sem, int val)
{
    if(!sem) return ISIX_EINVARG;
    //Semaphore is used
    _isixp_enter_critical();
    if(list_isempty(&sem->sem_task)==false)
    {
        _isixp_exit_critical();
        return ISIX_EBUSY;
    }
    port_atomic_sem_write_val( &sem->value, val );
    _isixp_exit_critical();
    return ISIX_EOK;
}

/*--------------------------------------------------------------*/
//Get value of semaphore
int isix_sem_getval(sem_t *sem)
{
    if(!sem) return ISIX_EINVARG;
    return port_atomic_sem_read_val( &sem->value );
}

/*--------------------------------------------------------------*/
//Sem destroy
int isix_sem_destroy(sem_t *sem)
{
   if(!sem) return ISIX_EINVARG;
    //Semaphore is used
   _isixp_enter_critical();
   if( !list_isempty(&sem->sem_task) )
   {
       _isixp_exit_critical();
       return ISIX_EBUSY;
   }
   if(!sem->static_mem) isix_free(sem);
   _isixp_exit_critical();
   return ISIX_EOK;
}

/*--------------------------------------------------------------*/
//! Convert ms to ticks
tick_t isix_ms2tick(unsigned long ms)
{
	tick_t ticks = (ISIX_CONFIG_HZ * ms)/1000UL;
	if(ticks==0) ticks++;
	return ticks;
}

/*--------------------------------------------------------------*/
//! Isix wait selected amount of time
int isix_wait(tick_t timeout)
{
	if(_isix_scheduler_running)
	{
		//If scheduler is running delay on semaphore
		return isix_sem_wait(NULL,timeout);
	}
	else
	{
		//If scheduler is not running delay on busy wait
		tick_t t1 = isix_get_jiffies();
		if(t1+timeout>t1)
		{
			t1+= timeout;
			while(t1>isix_get_jiffies()) port_idle_cpu();
		}
		else
		{
			t1+= timeout;
			while(t1<isix_get_jiffies()) port_idle_cpu();
		}
		return ISIX_EOK;
	}
}

/*--------------------------------------------------------------*/

