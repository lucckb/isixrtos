#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <prv/scheduler.h>
#include <isix/semaphore.h>
#include <prv/semaphore.h>
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
	port_atomic_init( &sem->sem_task_count, 0 );
    list_init(&sem->sem_task);
    isix_printk("Create sem %08x val %d",sem,sem->value);
    return sem;
}

/*--------------------------------------------------------------*/
//Wait for semaphore P()
//TODO: priority inheritance
int isix_sem_wait(sem_t *sem, tick_t timeout)
{
    //If nothing to to - exit
    if(sem==NULL && timeout==0) return ISIX_EINVARG;
    isix_printk("Operate on task %08x state %02x",_isix_current_task,_isix_current_task->state);
    //Lock scheduler
    _isixp_enter_critical();
    if( sem && port_atomic_sem_dec( &sem->value ) > 0 )
    {
        isix_printk("Decrement value %d",sem->value->value);
		_isixp_exit_critical();
        return ISIX_EOK;
    }
    //If any task remove task from ready list
    if(timeout || sem)
    {
        if(_isix_current_task->state & TASK_READY)
        {
            _isix_current_task->state &= ~(TASK_READY| TASK_RUNNING );
            _isixp_delete_task_from_ready_list(_isix_current_task);
            isix_printk("Delete task from ready list");
        }
    }
    else
    {
        _isixp_exit_critical();
        return ISIX_EINVARG;
    }
    //Sleep in semaphore
    if(timeout)
    {
    	//Add to waiting list
    	_isixp_add_task_to_waiting_list(_isix_current_task,timeout);
        _isix_current_task->state |= TASK_SLEEPING;
        isix_printk("Wait after %d ticks",_isix_current_task->jiffies);
    }
    if(sem)
    {
    	if(_isix_current_task->sem)
    	{
    	   isix_bug("Task assigned to non empty sem");
    	}
    	_isixp_add_task_to_sem_list(&sem->sem_task,_isix_current_task);
        _isix_current_task->state |= TASK_WAITING;
        _isix_current_task->sem = sem;
		port_atomic_inc( &sem->sem_task_count );
        isix_printk("Add task %08x to sem",_isix_current_task);
    }
    _isixp_exit_critical();
    isix_yield();
    isix_printk("task %08x after wakeup reason %d", _isix_current_task,
    	(_isix_current_task->state&TASK_SEM_WKUP)?ISIX_EOK:ISIX_ETIMEOUT);
    return (_isix_current_task->state&TASK_SEM_WKUP)?ISIX_EOK:ISIX_ETIMEOUT;
}
/*--------------------------------------------------------------*/
//Sem signal V()
int _isixp_sem_signal( sem_t *sem, bool isr )
{
    //If not sem not release it
    if(!sem)
    {
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
	else 
	{
		if( port_atomic_read( &sem->sem_task_count ) <=0 )
		{
			_isixp_exit_critical();
			return ISIX_EOK;
		}
        if( list_isempty(&sem->sem_task) )
		{
			isix_bug("List is empty. Atomic wait count mismatch");
		}
		//Decrement again
		port_atomic_sem_dec( &sem->value ); 
		//List is not empty wakeup high priority task
		task_t *task_wake = list_get_first(&sem->sem_task,inode_sem,task_t);
		isix_printk("Task to wakeup %08x",task_wake);
		//Remove from time list
		if(task_wake->state & TASK_SLEEPING)
		{
			list_delete(&task_wake->inode);
			task_wake->state &= ~TASK_SLEEPING;
		}
		//Task in waiting list is always in waking state
		//Reschedule is needed wakeup task have higer prio then current prio
		if(!task_wake->sem)
		{
			isix_bug("Sem is empty");
		}
		if(task_wake->sem != sem)
		{
			isix_bug("Task is not assigned to semaphore");
		}
		task_wake->state &= ~TASK_WAITING;
		task_wake->state |= TASK_READY | TASK_SEM_WKUP;
		task_wake->sem = NULL;
		list_delete(&task_wake->inode_sem);
		port_atomic_dec( &sem->sem_task_count );
		if(_isixp_add_task_to_ready_list(task_wake)<0)
		{
			_isixp_exit_critical();
			return ISIX_ENOMEM;
		}
		if(task_wake->prio<_isix_current_task->prio && !isr)
		{
			isix_printk("Yield processor higer prio");
			_isixp_exit_critical();
			isix_yield();
			return ISIX_EOK;
		}
		else
		{
			_isixp_exit_critical();
			return ISIX_EOK;
		}
	}
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
   if(list_isempty(&sem->sem_task)==false)
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

