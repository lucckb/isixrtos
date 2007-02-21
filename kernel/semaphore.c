#include <isix/config.h>
#include <isix/memory.h>
#include <isix/scheduler.h>
#include <isix/semaphore.h>
#include <isix/time.h>


#define DEBUG

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
    bzero(sem,sizeof(sem_t));
    sem->value = val;
    list_init(&sem->sem_task);
    return sem;
}

/*--------------------------------------------------------------*/
//Add task to semaphore list
static void add_task_to_sem_list(list_entry_t *sem_list,task_t *task)
{
   //Scheduler lock
    sched_lock();
   //Insert on waiting list in time order
    task_t *taskl;
    list_for_each_entry(sem_list,taskl,inode)
    {
       if(taskl->prio<task->prio) break;
    }
    printk("MoveTaskToWaiting: insert in time list at %08x\n",taskl);
    list_insert_after(&taskl->inode_sem,&task->inode_sem);
    //Scheduler unlock
    sched_unlock();

}
/*--------------------------------------------------------------*/
//TODO: current task sem jest niezbedy
//Wait for semaphore P()
//TODO: priority inheritance detect reason of exit Add state
int sem_wait(sem_t *sem,time_t timeout)
{
    int res = 0;
    //Lock scheduler
    sched_lock();
    if(sem && sem->value>0)
    {
        sem->value--;
        sched_unlock();
        return res;
    }
    //If any task remove task from ready list
    if(timeout || sem)
    {
        current_task->state &= ~TASK_READY;
        list_delete(&current_task->inode);
    }
    //Sleep in semaphore
    if(timeout)
    {
        //get current time
        time(&current_task->time);
        //add wakeup timeout
        current_task->time += timeout;
        //Move task from ready list to waiting list
        add_task_to_waiting_list(current_task);
        current_task->state |= TASK_SLEEPING;
        printk("SemWait: Wait after %d ticks\n",current_task->time);
    }
    if(sem)
    {
        add_task_to_sem_list(&sem->sem_task,current_task);
        current_task->state |= TASK_WAITING;
        printk("SemWait: Add task %08x to sem\n",current_task);
    }
    sched_unlock();
    sched_yield();
    //After yield not waiting for sem
    sched_lock();
    if(!(current_task->state&TASK_SLEEPING) && timeout) res = -1;
    sched_unlock();
    return res;
}

/*--------------------------------------------------------------*/
//Sem signal V()
int __sem_signal(sem_t *sem,bool isr)
{
    //If not sem not release it
    if(!sem) return -1;
    sched_lock();
    if(list_isempty(&sem->sem_task)==true)
    {
        sem->value++;
        sched_unlock();
        return 0;
    }
    //List is not empty wakeup high priority task
    task_t *task_wake = list_get_first(&sem->sem_task,inode_sem,task_t);
    printk("SemSignal: Task to wakeup %08x\n",task_wake);
    //Remove from time list
    if(task_wake->time) list_delete(&task_wake->inode);
    //Reschedule is needed wakeup task have higer prio then current prio
    list_delete(&task_wake->inode_sem);
    task_wake->state &= ~TASK_WAITING;
    task_wake->state |= TASK_READY;
    add_task_to_ready_list(task_wake);
    sched_unlock();
    if(task_wake->prio<current_task->prio && !isr)
    {
        printk("SemSignal: Yield current process\n");
        sched_yield();
    }
    return 0;
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


