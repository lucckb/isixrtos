#include <isix/config.h>
#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/task.h>
#include <isix/memory.h>


//TODO: Disable at create end of module
#define DEBUG

#ifdef DEBUG
#include <isix/printk.h>
#else
#define printk(...)
#endif


/*-----------------------------------------------------------------------*/
/* Create task function */
task_t* task_create(task_func_ptr_t task_func, void *func_param,reg_t stack_depth, prio_t priority)
{
    printk("TaskCreate: Create task with prio %d\n",priority);
    //If stack length is small error
    if(stack_depth<SCHED_MIN_STACK_DEPTH) return NULL;
    //Allocate task_t structure
    task_t *task = (task_t*)kmalloc(sizeof(task_t));
    printk("TaskCreate: Alloc task struct %08x\n",task);
    //No free memory
    if(task==NULL) return NULL;
    //Try Allocate stack for task
    task->top_stack = (reg_t*)kmalloc(stack_depth);
    printk("TaskCreate: Alloc stack mem %08x\n",task->top_stack);
    if(task->top_stack==NULL)
    {
        //Free allocated stack memory
        kfree(task);
        return NULL;
    }
#ifdef CONFIG_STACK_GROWTH
     task->top_stack = (reg_t*)((char*)task->top_stack + stack_depth - 4);
#endif
    printk("TaskCreate: Top stack SP=%08x\n",task->top_stack);
    //Assign task priority
    task->prio = priority;
    //Set time to 0
    task->time = 0;
    //Task is ready
    task->state = TASK_READY;
    //Create initial task stack context
    task->top_stack = task_init_stack(task->top_stack,task_func,func_param);
    //Lock scheduler
    sched_lock();
    //Add task to ready list
    if(add_task_to_ready_list(task)<0)
    {
        //Free allocated innode
        printk("TaskCreate: Add task to ready list failed\n");
        kfree(task->top_stack);
        kfree(task);
	    sched_unlock();
	    return NULL;
    }
    if(scheduler_running==false)
    {
        //Scheduler not running assign task
        if(current_task==NULL) current_task = task;
        else if(current_task->prio>task->prio) current_task = task;
    }
    sched_unlock();
    if(current_task->prio>task->prio && scheduler_running==true)
    {
        //New task have higer priority then current task
	    printk("TaskCreate: Call scheduler new prio %d > old prio %d\n",task->prio,current_task->prio);
        sched_yield();
    }
    return task;
}
/*-----------------------------------------------------------------------*/
//TODO: Add other task function task_delete,task_change prio , etc.
