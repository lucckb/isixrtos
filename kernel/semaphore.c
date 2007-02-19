#include <isix/config.h>
#include <isix/memory.h>
#include <isix/scheduler.h>
#include <isix/semaphore.h>
#include <isix/time.h>

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
//Wait for semaphore P()
//TODO: priority inheritance
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
    //Sleep in semaphore
    if(timeout)
    {
        //get current time
        time(&current_task->time);
        //add wakeup timeout
        current_task->time += timeout;
        //Move task from ready list to waiting list
        add_task_to_list(NULL,current_task);
        printk("SemWait: Wait after %d ticks\n",current_task->time);
    }
    if(sem!=NULL)
    {
        add_task_to_list(&sem->sem_task,current_task);
        current_task->sem = sem;
        printk("SemWait: Add task %08x to sem\n",current_task);
    }
    //Remove from ready list
    list_delete(&current_task->inode);
    sched_unlock();
    sched_yield();
    //After yield not waiting for sem
    sched_lock();
    current_task->sem=NULL;
    if(current_task->time==0) res = -1;
    sched_unlock();
    return res;
}

