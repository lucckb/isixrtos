#include <isix/types.h>
#include <isix/config.h>
#include <isix/memory.h>
#include <prv/scheduler.h>
#include <isix/semaphore.h>
#include <prv/semaphore.h>
#include <string.h>

#ifndef ISIX_DEBUG_SEMAPHORE
#define ISIX_DEBUG_SEMAPHORE ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_SEMAPHORE == ISIX_DBG_ON
#include <isix/printk.h>
#else
#define printk(...)
#endif


/*--------------------------------------------------------------*/
//Create semaphore
sem_t* isix_sem_create_isr(sem_t *sem,int val,int interrupt)
{
    if(sem==NULL)
    {
        sem = (sem_t*)isix_alloc(sizeof(sem_t));
        if(sem==NULL) return NULL;
    }
    memset(sem,0,sizeof(sem_t));
    sem->value = val;
    sem->intno = interrupt;
    list_init(&sem->sem_task);
    printk("SemCreate: Create sem %08x val %d\n",sem,sem->value);
    return sem;
}

/*--------------------------------------------------------------*/
//Wait for semaphore P()
//TODO: priority inheritance
int isix_sem_wait(sem_t *sem, tick_t timeout)
{
    //If nothing to to - exit
    if(sem==NULL && timeout==0) return ISIX_EINVARG;
    //Lock scheduler
    isixp_sched_lock();
    if(sem && sem->intno!=-1) port_isr_lock();
    printk("SemWait: Operate on task %08x state %02x\n",isix_current_task,isix_current_task->state);
    if(sem && sem->value>0)
    {
        sem->value--;
        printk("SemWait: Decrement value %d\n",sem->value);
        if(sem->intno!=-1) port_isr_unlock();
        isixp_sched_unlock();
        return ISIX_EOK;
    }
    //If any task remove task from ready list
    if(timeout || sem)
    {
        if(isix_current_task->state & TASK_READY)
        {
            isix_current_task->state &= ~(TASK_READY| TASK_RUNNING );
            isixp_delete_task_from_ready_list(isix_current_task);
            printk("SemWait: Delete task from ready list\n");
        }
    }
    else
    {
        if(sem && sem->intno!=-1) port_isr_unlock();
        isixp_sched_unlock();
        return ISIX_EINVARG;
    }
    //Sleep in semaphore
    if(timeout)
    {
    	//Add to waiting list
    	isixp_add_task_to_waiting_list(isix_current_task,timeout);
        isix_current_task->state |= TASK_SLEEPING;
        printk("SemWait: Wait after %d ticks\n",isix_current_task->jiffies);
    }
    if(sem)
    {
        isixp_add_task_to_sem_list(&sem->sem_task,isix_current_task);
        isix_current_task->state |= TASK_WAITING;
        isix_current_task->sem = sem;
        printk("SemWait: Add task %08x to sem\n",isix_current_task);
    }
    if(sem && sem->intno!=-1) port_isr_unlock();
    isixp_sched_unlock();
    isix_sched_yield();
    if(sem && sem->intno!=-1) port_isr_unlock();
    printk("SemWait: task %08x after wakeup reason %d\n",isix_current_task,sem->sem_ret);
    return sem->sem_ret;
}

/*--------------------------------------------------------------*/
//Sem signal V()
int isixp_sem_signal(sem_t *sem,bool isr)
{
    //If not sem not release it
    if(!sem)
    {
        printk("SemSignal: No sem\n");
        return ISIX_EINVARG;
    }
    isixp_sched_lock();
    if(isr==false && sem->intno!=-1) port_isr_lock();
    if(list_isempty(&sem->sem_task)==true)
    {
        sem->value++;
        printk("SemSignal: Waiting list is empty incval to %d\n",sem->value);
        if(isr==false && sem->intno!=-1) port_isr_unlock();
        isixp_sched_unlock();
        return ISIX_EOK;
    }
    //List is not empty wakeup high priority task
    task_t *task_wake = list_get_first(&sem->sem_task,inode_sem,task_t);
    printk("SemSignal: Task to wakeup %08x\n",task_wake);
    //Remove from time list
    if(task_wake->state & TASK_SLEEPING)
    {
        list_delete(&task_wake->inode);
    }
    //Task in waiting list is always in waking state
    //Reschedule is needed wakeup task have higer prio then current prio
    list_delete(&task_wake->inode_sem);
    task_wake->state &= ~TASK_WAITING;
    task_wake->state |= TASK_READY;
    sem->sem_ret = 0;
    isix_current_task->sem = NULL;
    if(isixp_add_task_to_ready_list(task_wake)<0)
    {
        if(isr==false && sem->intno!=-1) port_isr_unlock();
        isixp_sched_unlock();
        return ISIX_ENOMEM;
    }
    if(task_wake->prio<isix_current_task->prio && !isr)
    {
        printk("SemSignal: Yield processor higer prio\n");
        if(isr==false && sem->intno!=-1) port_isr_unlock();
        isixp_sched_unlock();
        isix_sched_yield();
        return ISIX_EOK;
    }
    else
    {
        if(isr==false && sem->intno!=-1) port_isr_unlock();
        isixp_sched_unlock();
        return ISIX_EOK;
    }
}

/*--------------------------------------------------------------*/
//Get semaphore from isr
int isix_sem_get_isr(sem_t *sem)
{
    if(!sem) return ISIX_EINVARG;
    int res = ISIX_EBUSY;
    isixp_sched_lock();
    if(sem && sem->value>0)
    {
        sem->value--;
        res = ISIX_EOK;
    }
    isixp_sched_unlock();
    return res;
}

/*--------------------------------------------------------------*/
//Sem value of semaphore
int isix_sem_setval(sem_t *sem,int val)
{
    if(!sem) return ISIX_EINVARG;
    //Semaphore is used
    isixp_sched_lock();
    if(sem->intno!=-1) port_isr_lock();
    if(list_isempty(&sem->sem_task)==false)
    {
        if(sem->intno!=-1) port_isr_unlock();
        isixp_sched_unlock();
        return ISIX_EBUSY;
    }
    sem->value = val;
    if(sem->intno!=-1) port_isr_unlock();
    isixp_sched_unlock();
    return ISIX_EOK;
}

/*--------------------------------------------------------------*/
//Get value of semaphore
int isix_sem_getval(sem_t *sem)
{
    if(!sem) return ISIX_EINVARG;
    return sem->value;
}

/*--------------------------------------------------------------*/
//Sem destroy
int isix_sem_destroy(sem_t *sem)
{
   if(!sem) return ISIX_EINVARG;
    //Semaphore is used
   isixp_sched_lock();
   if(list_isempty(&sem->sem_task)==false)
   {
       isixp_sched_unlock();
       return ISIX_EBUSY;
   }
   isix_free(sem);
   isixp_sched_unlock();
   return ISIX_EOK;
}


