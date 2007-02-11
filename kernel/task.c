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
    //If priority is then error ocurred
    if(priority>=SCHED_IDLE_PRIORITY) return NULL;
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
    //Assign task priority
    task->prio = priority;
    //Create initial task stack context
    task->top_stack = task_init_stack(task->top_stack,task_func,func_param);
    
    //TODO: Other SCHEDULER STUFF
    add_task_to_ready_list(task); 
    //Return task ID for other operation
    return task;
}
/*-----------------------------------------------------------------------*/

