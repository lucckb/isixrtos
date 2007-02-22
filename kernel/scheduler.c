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
//Global scheler time
volatile time_t sched_time;

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
void scheduler(void)
{
   //If scheduler is locked switch context is disable
    if(sched_lock_counter) return;
    //Remove executed task and add at end
    if(current_task->state & TASK_READY)
    {
        list_delete(&current_task->inode);
        list_insert_end(&current_prio->task_list,&current_task->inode);
        current_task->state &= ~TASK_RUNNING;
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
}

/*-----------------------------------------------------------------------*/
//Time call from isr
void scheduler_time(void)
{
    //Increment sys tick
    sched_time++;
    printk(".");
    //If scheduler is locked switch context is disable
    if(sched_lock_counter) return;
    if(list_isempty(&waiting_task)) return;
    task_t *task_c = list_get_first(&waiting_task,inode,task_t);
    if(sched_time>=task_c->time)
    {
        printk("SchedulerTime: sched_time %d task_time %d\n",sched_time,task_c->time);
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
    //sched_lock();
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
      //      sched_unlock();
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
    //sched_unlock();
    return 0;
}

/*-----------------------------------------------------------------------*/
//Delete task from ready list
void delete_task_from_ready_list(task_t *task)
{
    //Scheduler lock
//   sched_lock();
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
//   sched_unlock();
}

/*-----------------------------------------------------------------------*/
//Move selected task to waiting list
void add_task_to_waiting_list(task_t *task)
{
    //Scheduler lock
//    sched_lock();
    //Insert on waiting list in time order
    task_t *waitl;
    list_for_each_entry(&waiting_task,waitl,inode)
    {
       if(waitl->time<task->time) break;
    }
    printk("MoveTaskToWaiting: insert in time list at %08x\n",&waitl->inode);
    list_insert_after(&waitl->inode,&task->inode);
    //Scheduler unlock
//    sched_unlock();
}

/*--------------------------------------------------------------*/
//Add task to semaphore list
void add_task_to_sem_list(list_entry_t *sem_list,task_t *task)
{
   //Scheduler lock
//    sched_lock();
   //Insert on waiting list in time order
    task_t *taskl;
    list_for_each_entry(sem_list,taskl,inode)
    {
       if(taskl->prio<task->prio) break;
    }
    printk("MoveTaskToWaiting: insert in time list at %08x\n",taskl);
    list_insert_after(&taskl->inode_sem,&task->inode_sem);
    //Scheduler unlock
//    sched_unlock();

}
/*-----------------------------------------------------------------------*/
//Add task list to delete 
void add_task_to_delete_list(task_t *task)
{
    //lock scheduler
//    sched_lock();
    list_insert_end(&dead_task,&task->inode);
//    sched_unlock();
}

/*-----------------------------------------------------------------------*/
//Idle task function do nothing and lower priority
//TODO: Add task to delete :)
TASK_FUNC(idle_task,p)
{
    while(1)
    {
#ifndef  CONFIG_USE_PREEMPTION
        sched_yield();
#endif
    }
}
/*-----------------------------------------------------------------------*/
#undef printk
#include <isix/printk.h>

TASK_FUNC(fun1,n)
{
    char *p = (char*)n;
    //printk("func1(%08x)\n",(u32)n);
    while(1)
    {
    //     for(volatile int i=0;i<1000000;i++);
        sem_wait(NULL,HZ*2);
    //    printk("***********************************\n");
                // sched_lock();
    //    printk("**func1 %d time %d**\n",*p,sched_time);
    printk("%c",*p);
       // sched_unlock();
//        *p = *p + 1;
        //sched_yield();
    }
}

#if 0
void print_rdy(void)
{
        task_ready_t *i;
        list_for_each_entry(&ready_task,i,inode)
        {
            printk("List inode %08x prio %d\n",(unsigned int)i,i->prio);
        }
}

void print_bsy(void)
{
        task_t *i;
        list_for_each_entry(&waiting_task,i,inode)
        {
            printk("List inode %08x prio %d\n",(unsigned int)i,i->time);
        }
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
	printk("Hello from OSn\n");
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
// TODO: Main function temp only for tests
char cnt1='c',cnt2='z';
int main(void)
{
    task_create(fun1,&cnt1,300,10);
//     task_create(fun1,(void*)0x02020202,200,20);
    //task_create(fun1,(void*)0x03030303,200,15);
    task_create(fun1,&cnt2,400,10);
    return 0;
}

