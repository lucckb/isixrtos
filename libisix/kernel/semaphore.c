#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <prv/scheduler.h>
#include <isix/semaphore.h>
#include <prv/semaphore.h>
#include <string.h>
#include <prv/multiple_objects.h>
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
	port_atomic_sem_init(&sem->value, val );
    sem->limit_value = limit_val;
    //Set sem type
    sem->type = IHANDLE_T_SEM;
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
    isix_printk("Operate on task %08x state %02x",isix_current_task,isix_current_task->state);
    if( sem && port_atomic_sem_dec( &sem->value ) > 0 )
    {
        isix_printk("Decrement value %d",sem->value->value);
        return ISIX_EOK;
    }
    //Lock scheduler
    isixp_enter_critical();
    //If any task remove task from ready list
    if(timeout || sem)
    {
        if(isix_current_task->state & TASK_READY)
        {
            isix_current_task->state &= ~(TASK_READY| TASK_RUNNING );
            isixp_delete_task_from_ready_list(isix_current_task);
            isix_printk("Delete task from ready list");
        }
    }
    else
    {
        isixp_exit_critical();
        return ISIX_EINVARG;
    }
    //Sleep in semaphore
    if(timeout)
    {
    	//Add to waiting list
    	isixp_add_task_to_waiting_list(isix_current_task,timeout);
        isix_current_task->state |= TASK_SLEEPING;
        isix_printk("Wait after %d ticks",isix_current_task->jiffies);
    }
    if(sem)
    {
    	if(isix_current_task->sem)
    	{
    	   isix_bug("Task assigned to non empty sem");
    	}
    	isixp_add_task_to_sem_list(&sem->sem_task,isix_current_task);
        isix_current_task->state |= TASK_WAITING;
        isix_current_task->sem = sem;
        isix_printk("Add task %08x to sem",isix_current_task);
    }
    isixp_exit_critical();
    isix_yield();
    isix_printk("task %08x after wakeup reason %d", isix_current_task,
    	(isix_current_task->state&TASK_SEM_WKUP)?ISIX_EOK:ISIX_ETIMEOUT);
    return (isix_current_task->state&TASK_SEM_WKUP)?ISIX_EOK:ISIX_ETIMEOUT;
}
/*--------------------------------------------------------------*/
#if ISIX_CONFIG_USE_MULTIOBJECTS == ISIX_ON
//Wakeup from iterate over muliple objects
static int isixp_wakeup_multiple( task_t *task_wake )
{
	isix_printk("task state %02x",task_wake->state);
	//Remove from time list
	if(task_wake->state & TASK_SLEEPING)
	{
	    list_delete(&task_wake->inode);
	    task_wake->state &= ~TASK_SLEEPING;
	    isix_printk("Remove task %08x from time list",task_wake );
	}
	task_wake->state &= ~TASK_WAITING_MULTIPLE;
	task_wake->state |= TASK_READY | TASK_MULTIPLE_WKUP;
	if(isixp_add_task_to_ready_list(task_wake)<0)
	{
	     return ISIX_ENOMEM;
	}
	return ISIX_EOK;
}
#endif
/*--------------------------------------------------------------*/
//Sem signal V()
int isixp_sem_signal( sem_t *sem, bool isr )
{
    //If not sem not release it
    if(!sem)
    {
        isix_printk("No sem");
        return ISIX_EINVARG;
    }
	//If one need increment
	//TODO: Fix this
	if( port_atomic_sem_inc( &sem->value, sem->limit_value ) != 1 )
    {
        isix_printk("Waiting list is empty incval to %d",sem->value);
#if ISIX_CONFIG_USE_MULTIOBJECTS == ISIX_ON 
        //Only for multiple objs
        {
		  //TODO: Multiple object needs reimplementation
          isixp_enter_critical();
		  int ret = isixp_wakeup_multiple_waiting_tasks( sem, isixp_wakeup_multiple );
          if(ret>0)
          {
        	  if(ret<isix_current_task->prio && !isr)
       		  {
        		 isix_printk("Yield processor higer prio wktask %d main %d",ret,isix_current_task->prio);
        		 isixp_exit_critical();
       			 isix_yield();
    			 isixp_enter_critical();
       		  }
          }
          else { isixp_exit_critical(); return ret; }
        }
#endif
        return ISIX_EOK;
    }
	else 
	{
		isixp_exit_critical();
        if( list_isempty(&sem->sem_task) )
		{
			isix_bug( "Semaphore task not empty" );
		}
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
		if(isixp_add_task_to_ready_list(task_wake)<0)
		{
			isixp_exit_critical();
			return ISIX_ENOMEM;
		}
		if(task_wake->prio<isix_current_task->prio && !isr)
		{
			isix_printk("Yield processor higer prio");
			isixp_exit_critical();
			isix_yield();
			return ISIX_EOK;
		}
		else
		{
			isixp_exit_critical();
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
    isixp_enter_critical();
    if(list_isempty(&sem->sem_task)==false)
    {
        isixp_exit_critical();
        return ISIX_EBUSY;
    }
    if(sem->limit_value > ISIX_SEM_ULIMITED)
    {
         if( val > sem->limit_value )
         {
           isix_printk("Limit value to %d",val);
           val  = sem->limit_value;
         }
    }
    port_atomic_sem_write_val( &sem->value, val );
    isixp_exit_critical();
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
   isixp_enter_critical();
   if(list_isempty(&sem->sem_task)==false)
   {
       isixp_exit_critical();
       return ISIX_EBUSY;
   }
   if(!sem->static_mem) isix_free(sem);
   isixp_exit_critical();
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
	if(isix_scheduler_running)
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

