#include <isix/config.h>
#include <isix/memory.h>
#include <isix/scheduler.h>
#include <isix/time.h>
#include <isix/semaphore.h>

//#define DEBUG

#ifdef DEBUG
#include <isix/printk.h>
#else
#define printk(...)
#endif

/*--------------------------------------------------------------*/
//Create semaphore
sem_t* sem_create(sem_t *sem,int val)
{
    if(sem==NULL)
    {
        sem = (sem_t*)kmalloc(sizeof(sem_t));
    }
    zero_memory(sem,sizeof(sem_t));
    sem->value = val;
    list_init(&sem->sem_task);
    printk("SemCreate: Create sem %08x val %d\n",sem,sem->value);
    return sem;
}

/*--------------------------------------------------------------*/
//Wait for semaphore P()
//TODO: priority inheritance
int sem_wait(sem_t *sem,unsigned long timeout)
{
    int res = 0;
    //Lock scheduler
    sched_lock();
    printk("SemWait: Operate on task %08x state %02x\n",current_task,current_task->state);
    if(sem && sem->value>0)
    {
        sem->value--;
        printk("SemWait: Decrement value %d\n",sem->value);
        sched_unlock();
        return res;
    }
    //If any task remove task from ready list
    if(timeout || sem)
    {
        if(current_task->state & TASK_READY)
        {
            current_task->state &= ~TASK_READY;
            delete_task_from_ready_list(current_task);
            printk("SemWait: Delete task from ready list\n");
        }
    }
    else
    {
        sched_unlock();
        return -1;
    }
    //Sleep in semaphore
    if(timeout)
    {
        //get current jiffies
        current_task->jiffies = get_jiffies();
        //add wakeup timeout
        current_task->jiffies += timeout;
        //Move task from ready list to waiting list
        add_task_to_waiting_list(current_task);
        current_task->state |= TASK_SLEEPING;
        printk("SemWait: Wait after %d ticks\n",current_task->jiffies);
    }
    if(sem)
    {
        add_task_to_sem_list(&sem->sem_task,current_task);
        current_task->state |= TASK_WAITING;
        current_task->sem = sem;
        printk("SemWait: Add task %08x to sem\n",current_task);
    }
    sched_unlock();
    sched_yield();
    //After yield not waiting for sem
    sched_lock();
    printk("SemWait: task %08x after wakeup\n",current_task);
    if(!(current_task->state & TASK_SLEEPING) && timeout)
    {
        res = -1;
        current_task->state &= ~TASK_SLEEPING;
        if(current_task->state & TASK_WAITING)
        {
            printk("SemWait: Timeout delete from sem list\n");
            list_delete(&current_task->inode_sem);
            current_task->state &= ~TASK_WAITING;
            current_task->sem = NULL;
        }
    }
    sched_unlock();
    return res;
}

/*--------------------------------------------------------------*/
//Sem signal V()
int __sem_signal(sem_t *sem,bool isr)
{
    //If not sem not release it
    if(!sem)
    {
        printk("SemSignal: No sem\n");
        return -1;
    }
    sched_lock();
    if(list_isempty(&sem->sem_task)==true)
    {
        sem->value++;
        printk("SemSignal: Waiting list is empty incval to %d\n",sem->value);
        sched_unlock();
        return 0;
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
    current_task->sem = NULL;
    if(add_task_to_ready_list(task_wake)<0)
    {
        sched_unlock();
        return -1;
    }
    if(task_wake->prio<current_task->prio && !isr)
    {
        printk("SemSignal: Yield processor higer prio\n");
        sched_unlock();
        sched_yield();
        return 0;
    }
    else
    {
        sched_unlock();
        return 0;
    }
}
/*--------------------------------------------------------------*/
//Get semaphore from isr
int sem_get_isr(sem_t *sem)
{
    if(!sem) return -1;
    int res = -1;
    sched_lock();
    if(sem && sem->value>0)
    {
        sem->value--;
        res = 0;
    }
    sched_unlock();
    return res;
}

/*--------------------------------------------------------------*/
//Sem value of semaphore
int sem_setval(sem_t *sem,int val)
{
    if(!sem) return -1;
    //Semaphore is used
    sched_lock();
    if(list_isempty(&sem->sem_task)==false)
    {
        sched_unlock();
        return -1;
    }
    sem->value = val;
    sched_unlock();
    return 0;
}

/*--------------------------------------------------------------*/
//Sem destroy
int sem_destroy(sem_t *sem)
{
   if(!sem) return -1;
    //Semaphore is used
   sched_lock();
   if(list_isempty(&sem->sem_task)==false)
   {
       sched_unlock();
       return -1;
   }
   kfree(sem);
   sched_unlock();
   return 0;
}


