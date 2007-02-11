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
task_t * volatile current_task = NULL;

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
    return sched_lock_counter;
}

/*-----------------------------------------------------------------------*/
//Unlock scheduler
int sched_unlock(void)
{
    reg_t irq_state = irq_disable();
    if(sched_lock_counter>0) sched_lock_counter--;
    irq_restore(irq_state);
    return sched_lock_counter;
}
/*-----------------------------------------------------------------------*/

task_t *task1,*task2;

/*-----------------------------------------------------------------------*/
void scheduler(void)
{
    //If scheduler is locked switch context is disable
    if(sched_lock_counter) return;
    //Dummy task switch example
    static u8 n=0;
    if(n) current_task = task1;
    else current_task = task2;
    n = !n;
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
    }
    //Priority not found allocate priority node
    prio_i = (task_ready_t*)kmalloc(sizeof(task_ready_t));
    //If malloc return NULL then failed
    if(prio_i==NULL) return -1;
    //Assign priority
    prio_i->prio = task->prio;
    //Initialize and add at end of list
    list_init(&prio_i->task_list);
    list_insert_end(&prio_i->task_list,&task->inode);
    list_insert_end(&ready_task,&prio_i->inode);
    printk("AddTaskToReadyList: Add new node %08x with prio %d\n",prio_i,prio_i->prio);
    sched_unlock();
    return 0;
}
/*-----------------------------------------------------------------------*/
//TODO: REMOVE IT AFTER TEST

//TODO: REMOVE IT AFTER TEST
//static unsigned char stk1[200];
//static unsigned char stk2[200];

//static task_t task1,task2;


TASK_FUNC(fun1,n)
{
    const register reg_t tmp asm("r13");
    printk("func1(%08x)\n",(u32)n);
    while(1)
    {
    for(volatile int i=0;i<1000000;i++);
    printk("Hello from fun1 SP=%08x\n",tmp);
    cpu_yield();
    }
}

TASK_FUNC(fun2,n) 
{
   const register reg_t tmp asm("r13");
   printk("func2(%08x)\n",(u32)n);
   
   while(1)
    {
    for(volatile int i=0;i<1000000;i++);
    printk("Hello from fun2 SP=%08x\n",tmp);
    cpu_yield();
    }
}

void init_os(void) __attribute__((noreturn));


void init_os(void)
{
	//Initialize ready task list
    list_init(&ready_task);
    //Other stuff
    printk_init(UART_BAUD(115200));
	printk("Hello from OSn\n");
    //task1.top_stack = (reg_t*)&stk1[200-4];
    //task2.top_stack = (reg_t*)&stk2[200-4];
    //task1.top_stack = task_init_stack(task1.top_stack,fun1,(void*)0x10203040);
    //task2.top_stack = task_init_stack(task2.top_stack,fun2,(void*)0xaabbccdd);
    //printk("sp1=%08x sp2=%08x\n",(u32)task1.top_stack,(u32)task2.top_stack);
    
    task1 = task_create(fun1,(void*)0x50607080,200,10);
    task2 = task_create(fun2,(void*)0x10203040,200,10);
    current_task = task1;
    cpu_restore_context();
    while(1);    //Prevent compile warning
}

int main(void)
{
    return 0;
}   

