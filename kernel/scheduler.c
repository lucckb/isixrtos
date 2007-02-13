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
#define DEBUG


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
//Lock scheduler
int sched_lock(void)
{
    sched_lock_counter++;
   // printk("SchedLock: %d\n",sched_lock_counter);
    return sched_lock_counter;
}

/*-----------------------------------------------------------------------*/
//Unlock scheduler
int sched_unlock(void)
{
    reg_t irq_state = irq_disable();
    if(sched_lock_counter>0) sched_lock_counter--;
    irq_restore(irq_state);
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

TASK_FUNC(fun1,n)
{
    printk("func1(%08x)\n",(u32)n);
    while(1)
    {
        for(volatile int i=0;i<1000000;i++);
       printk("**func1(%08x)**\n",(u32)n);
        sched_yield();
    }
}

TASK_FUNC(fun2,n) 
{
   printk("Print list task(%08x)\n",(u32)n);
   
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
    //Other stuff
    printk_init(UART_BAUD(115200));
	printk("Hello from OSn\n");
}

/* This function start scheduler after main function */
void start_scheduler(void) __attribute__((noreturn));
void start_scheduler(void)
{
   scheduler_running = true;
   cpu_restore_context();
   while(1);    //Prevent compiler warning
}

int main(void)
{
    task_create(fun1,(void*)0x01010101,200,10);
    task_create(fun1,(void*)0x02020202,200,20);
    task_create(fun1,(void*)0x03030303,200,15);
    task_create(fun2,(void*)0x04040404,400,5);
    return 0;
}   

