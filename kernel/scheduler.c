#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <isix/scheduler.h>
#include <asm/context.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <asm/interrupt.h>
#include <isix/list.h>

//TODO: Remove at end debug of module
//#define DEBUG


#ifdef DEBUG
#include <isix/printk.h>
#else
#define printk(...)
#endif

/*-----------------------------------------------------------------------*/
//Current task pointer
volatile bool scheduler_running;

/*-----------------------------------------------------------------------*/
//Current task pointer
task_t * volatile current_task = NULL;

/*-----------------------------------------------------------------------*/
//Current prio list pointer
static task_ready_t * volatile current_prio = NULL;

/*-----------------------------------------------------------------------*/
//Sched lock counter
static volatile int sched_lock_counter = 0;

/*-----------------------------------------------------------------------*/
//Binary tree of task ready to execute
static list_entry_t ready_task;

/*-----------------------------------------------------------------------*/
//Task waiting for event
static list_entry_t waiting_task;

/*-----------------------------------------------------------------------*/
//Task waiting for event
static list_entry_t dead_task;


/*-----------------------------------------------------------------------*/
//Global jiffies
volatile u64 jiffies;

/*-----------------------------------------------------------------------*/
//Lock scheduler
int sched_lock(void)
{
    reg_t irq_s = irq_disable();
    sched_lock_counter++;
    irq_restore(irq_s);
    printk("SchedLock: %d\n",sched_lock_counter);
    return sched_lock_counter;
}

/*-----------------------------------------------------------------------*/
//Unlock scheduler
int sched_unlock(void)
{
    reg_t irq_s = irq_disable();
    if(sched_lock_counter>0) sched_lock_counter--;
    irq_restore(irq_s);
    printk("SchedUnlock: %d\n",sched_lock_counter);
    return sched_lock_counter;
}
/*-----------------------------------------------------------------------*/
//Scheduler is called in switch context
void schedule(void)
{
   //If scheduler is locked switch context is disable
    if(sched_lock_counter) return;
    sched_lock();
    //Remove executed task and add at end
    if(current_task->state & TASK_READY)
    {
        current_task->state &= ~TASK_RUNNING;
        list_delete(&current_task->inode);
        list_insert_end(&current_prio->task_list,&current_task->inode);
    }
    //Get first ready prio
    printk("Scheduler: prev prio %d prio list %08x\n",current_prio->prio,current_prio);
    current_prio = list_get_first(&ready_task,inode,task_ready_t);
    printk("Scheduler: new prio %d prio list %08x\n",current_prio->prio,current_prio);
    //Get first ready task
    printk("Scheduler: prev task %08x\n",current_task);
    current_task = list_get_first(&current_prio->task_list,inode,task_t);
    current_task->state |= TASK_RUNNING;
    printk("Scheduler: new task %08x\n",current_task);
    sched_unlock();
}

/*-----------------------------------------------------------------------*/
//Time call from isr
void schedule_time(void)
{
    //Increment sys tick
    jiffies++;
    //If scheduler is locked switch context is disable
    if(sched_lock_counter) return;
    if(list_isempty(&waiting_task)) return;
    task_t *task_c = list_get_first(&waiting_task,inode,task_t);
    if(jiffies>=task_c->jiffies)
    {
        printk("SchedulerTime: sched_time %d task_time %d\n",jiffies,task_c->jiffies);
        task_c->state &= ~TASK_SLEEPING;
        task_c->state |= TASK_READY;
        list_delete(&task_c->inode);
        if(add_task_to_ready_list(task_c)<0)
        {
            printk("SchedulerTime: Error in add task to ready list\n");
        }
    }
}

/*-----------------------------------------------------------------------*/
//Add assigned task to ready list 
int add_task_to_ready_list(task_t *task)
{
    //Scheduler lock
    sched_lock();
    //Find task equal entry
    task_ready_t *prio_i;
    list_for_each_entry(&ready_task,prio_i,inode)
    {
        //If task equal entry is found add this task to end list
        if(prio_i->prio==task->prio)
        {
            printk("AddTaskToReadyList: found prio %d equal node %08x\n",prio_i->prio,prio_i);
            //Set pointer to priority struct
            task->prio_elem = prio_i;
            //Add task at end of ready list
            list_insert_end(&prio_i->task_list,&task->inode);
            //Unlock scheduler
            sched_unlock();
            return 0;
        }
        else if(prio_i->prio<task->prio)
        {
           printk("AddTaskToReadyList: Insert %d node %08x\n",prio_i->prio,prio_i);
           break;
        }
    }
    //Priority not found allocate priority node
    task_ready_t *prio_n = (task_ready_t*)kmalloc(sizeof(task_ready_t));
    //If malloc return NULL then failed
    if(prio_n==NULL) return -1;
    //Assign priority
    prio_n->prio = task->prio;
    //Set pointer to priority struct
    task->prio_elem = prio_n;
    //Initialize and add at end of list
    list_init(&prio_n->task_list);
    list_insert_end(&prio_n->task_list,&task->inode);
    list_insert_after(&prio_i->inode,&prio_n->inode);
    if(scheduler_running==false)
    {
        if(current_prio==NULL) current_prio = prio_n;
        else if(current_prio->prio>prio_n->prio) current_prio = prio_n;
    }
    printk("AddTaskToReadyList: Add new node %08x with prio %d\n",prio_n,prio_n->prio);
    sched_unlock();
    return 0;
}

/*-----------------------------------------------------------------------*/
//Delete task from ready list
void delete_task_from_ready_list(task_t *task)
{
    //Scheduler lock
   sched_lock();
   list_delete(&task->inode);
   //Check for task on priority structure
   if(list_isempty(&task->prio_elem->task_list)==true)
   {
        //Task list is empty remove element
        printk("DeleteTskFromRdyLst: Remove prio list elem\n");
        list_delete(&task->prio_elem->inode);
        kfree(task->prio_elem);
   }
   //Scheduler unlock
   sched_unlock();
}

/*-----------------------------------------------------------------------*/
//Move selected task to waiting list
void add_task_to_waiting_list(task_t *task)
{
    //Scheduler lock
    sched_lock();
    //Insert on waiting list in time order
    task_t *waitl;
    list_for_each_entry(&waiting_task,waitl,inode)
    {
       if(waitl->jiffies<task->jiffies) break;
    }
    printk("MoveTaskToWaiting: insert in time list at %08x\n",&waitl->inode);
    list_insert_after(&waitl->inode,&task->inode);
    //Scheduler unlock
    sched_unlock();
}

/*--------------------------------------------------------------*/
//Add task to semaphore list
void add_task_to_sem_list(list_entry_t *sem_list,task_t *task)
{
   //Scheduler lock
    sched_lock();
   //Insert on waiting list in time order
    task_t *taskl;
    list_for_each_entry(sem_list,taskl,inode_sem)
    {
       if(taskl->prio<task->prio) break;
    }
    printk("MoveTaskToWaiting: insert in time list at %08x\n",taskl);
    list_insert_after(&taskl->inode_sem,&task->inode_sem);
    //Scheduler unlock
    sched_unlock();

}
/*-----------------------------------------------------------------------*/
//Add task list to delete
void add_task_to_delete_list(task_t *task)
{
    //lock scheduler
    sched_lock();
    list_insert_end(&dead_task,&task->inode);
    sched_unlock();
}

/*-----------------------------------------------------------------------*/
//Dead task are clean by this procedure called from idle task
//One idle call clean one dead tasks
static inline void cleanup_tasks(void)
{
    if(list_isempty(&dead_task)) return;
    sched_lock();
    task_t *task_del = list_get_first(&dead_task,inode,task_t);
    list_delete(&task_del->inode);
    if(task_del->state & TASK_DEAD)
    {
        printk("CleanupTasks: Task to delete is %08x stack SP %08x\n",task_del,task_del->init_stack);
        kfree(task_del->init_stack);
        kfree(task_del);
    }
    sched_unlock();
}

/*-----------------------------------------------------------------------*/
//Idle task function do nothing and lower priority
TASK_FUNC(idle_task,p)
{
    while(1)
    {
        //Cleanup free tasks
        cleanup_tasks();
#ifndef  CONFIG_USE_PREEMPTION
        sched_yield();
#endif
    }
}
/*-----------------------------------------------------------------------*/

#if 0
#undef printk
#include <isix/printk.h>
void print_rdy(void)
{
        task_ready_t *i;
        task_t *j;
        sched_lock();
        printk("-------Ready tasks -------------\n");
        list_for_each_entry(&ready_task,i,inode)
        {
            printk("List inode %08x prio %d\n",(unsigned int)i,i->prio);
            list_for_each_entry(&i->task_list,j,inode)
            {
                printk("\t task %08x prio %d state %d\n",j,j->prio,j->state);
            }
        }
        printk("------ Sleeping tasks -------------\n");
        list_for_each_entry(&waiting_task,j,inode)
        {
            printk("Task: %08x prio: %d state %d jiffies %d\n",j,j->prio,j->state,j->jiffies);
        }
        sched_unlock();
}


#endif

/*-----------------------------------------------------------------------*/
/* Initialize base OS structure before call main */
void init_os(void)
{
	//Initialize ready task list
    list_init(&ready_task);
    //Initialize waiting list
    list_init(&waiting_task);
    //Initialize dead task
    list_init(&dead_task);
    //Other stuff
    printk_init(UART_BAUD(115200));
    //Create idle task
    task_create(idle_task,NULL,SCHED_MIN_STACK_DEPTH,SCHED_IDLE_PRIORITY);
}

/*-----------------------------------------------------------------------*/
/* This function start scheduler after main function */
void start_scheduler(void) __attribute__((noreturn));
void start_scheduler(void)
{
   scheduler_running = true;
   //System time
   sys_time_init();
   //Restore context and run OS
   cpu_restore_context();
   while(1);    //Prevent compiler warning
}

/*-----------------------------------------------------------------------*/
