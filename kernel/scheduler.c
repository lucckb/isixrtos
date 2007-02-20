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
//Global scheler time
volatile time_t sched_time;

/*-----------------------------------------------------------------------*/
//Lock scheduler
int sched_lock(void)
{
    reg_t irq_s = irq_disable();
    sched_lock_counter++;
    irq_restore(irq_s);
    // printk("SchedLock: %d\n",sched_lock_counter);
    return sched_lock_counter;
}

/*-----------------------------------------------------------------------*/
//Unlock scheduler
int sched_unlock(void)
{
    reg_t irq_s = irq_disable();
    if(sched_lock_counter>0) sched_lock_counter--;
    irq_restore(irq_s);
    //printk("SchedUnlock: %d\n",sched_lock_counter);
    return sched_lock_counter;
}
/*-----------------------------------------------------------------------*/

//Scheduler is called in switch context
void scheduler(void)
{
   //If scheduler is locked switch context is disable
    if(sched_lock_counter) return;
    //Remove executed task and add at end
    list_delete(&current_task->inode);
    list_insert_end(&current_prio->task_list,&current_task->inode);
    //Get first ready prio
    printk("Scheduler: prev prio %d prio list %08x\n",current_prio->prio,current_prio);
    current_prio = list_get_first(&ready_task,inode,task_ready_t);
    printk("Scheduler: new prio %d prio list %08x\n",current_prio->prio,current_prio);
    //Get first ready task
    printk("Scheduler: prev task %08x\n",current_task);
    current_task = list_get_first(&current_prio->task_list,inode,task_t);
    printk("Scheduler: new task %08x\n",current_task);
}

/*-----------------------------------------------------------------------*/
//Time call from isr
void scheduler_time(void)
{
    //Increment sys tick
    sched_time++;
    //If scheduler is locked switch context is disable
    if(sched_lock_counter) return;
    task_t *task_c = list_get_first(&waiting_task,inode,task_t);
    if(sched_time>=task_c->time)
    {
        if(add_task_to_ready_list(task_c)<0)
        {
            printk("SchedulerTime: Error in add task to ready list\n");
        }
        if(task_c->sem) task_c->sem = NULL;
        task_c->time = 0;
        list_delete(&task_c->inode);
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
//Move selected task to waiting list
void add_task_to_waiting_list(task_t *task)
{
    //Scheduler lock
    sched_lock();
    //Check for task on priority structure
    if(list_isempty(&task->prio_elem->task_list)==true)
    {
        //Task list is empty remove element
        printk("MoveTaskToWaiting: Remove prio list elem\n");
        kfree(task->prio_elem);
    }
   //Insert on waiting list in time order
    task_t *waitl;
    list_for_each_entry(&waiting_task,waitl,inode)
    {
       if(waitl->time<task->time) break;
    }
    printk("MoveTaskToWaiting: insert in time list at %08x\n",waitl);
    list_insert_after(&waitl->inode,&task->inode);
    //Scheduler unlock
    sched_unlock();
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
        for(volatile int i=0;i<1000000;i++);
       // sched_lock();
    //    printk("**func1 %d time %d**\n",*p,sched_time);
    printk("%c",*p);
       // sched_unlock();
//        *p = *p + 1;
        //sched_yield();
    }
}

TASK_FUNC(fun2,n)
{
   //printk("Print list task(%08x)\n",(u32)n);
   
   while(1)
   {
        for(volatile int i=0;i<1000000;i++);
        task_ready_t *i;
        list_for_each_entry(&ready_task,i,inode)
        {
            printk("List inode %08x prio %d\n",(unsigned int)i,i->prio);
        }
        sched_yield();
   }
}


/* Initialize base OS structure before call main */
void init_os(void)
{
	//Initialize ready task list
    list_init(&ready_task);
    //Initialize waiting list
    list_init(&waiting_task);
    //Other stuff
    printk_init(UART_BAUD(115200));
	printk("Hello from OSn\n");
}

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

char cnt1='c',cnt2='z';
// TODO: Main function temp only for tests
int main(void)
{
    task_create(fun1,&cnt1,200,10);
    task_create(fun1,(void*)0x02020202,200,20);
    task_create(fun1,(void*)0x03030303,200,15);
    task_create(fun1,&cnt2,400,10);
    return 0;
}

