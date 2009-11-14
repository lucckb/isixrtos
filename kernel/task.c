#include <isix/config.h>
#include <isix/types.h>
#include <prv/scheduler.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <prv/semaphore.h>
#include <string.h>

#ifndef ISIX_DEBUG_TASK
#define ISIX_DEBUG_TASK ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_TASK == ISIX_DBG_ON
#include <isix/printk.h>
#else
#define printk(...)
#endif

/*-----------------------------------------------------------------------*/
//Align Mask
#define ALIGN_MASK 0x03
//Align Bytes
#define ALIGN_BYTES 4
//Magic value for stack checking
#define MAGIC_FILL_VALUE 0x55
/*-----------------------------------------------------------------------*/
/* Create task function */
task_t* isix_task_create(task_func_ptr_t task_func, void *func_param, unsigned long  stack_depth, prio_t priority)
{
	printk("TaskCreate: Create task with prio %d",priority);
    if(isix_get_max_priority()< priority )
    {
    	return NULL;
    }
	//If stack length is small error
    if(stack_depth<PORT_SCHED_MIN_STACK_DEPTH) return NULL;
    //Alignement
    if(stack_depth & ALIGN_MASK)
    {
        stack_depth += ALIGN_BYTES - (stack_depth & ALIGN_MASK);
    }
    //Allocate task_t structure
    task_t *task = (task_t*)isix_alloc(sizeof(task_t));
    printk("TaskCreate: Alloc task struct %08x\n",task);
    //No free memory
    if(task==NULL) return NULL;
    //Zero task structure
    memset(task,0,sizeof(task_t));
    //Try Allocate stack for task
    task->init_stack = isix_alloc(stack_depth);
    printk("TaskCreate: Alloc stack mem %08x\n",task->init_stack);
    if(task->init_stack==NULL)
    {
        //Free allocated stack memory
        isix_free(task);
        return NULL;
    }
#ifdef CONFIG_STACK_GROWTH
     task->top_stack = (unsigned long*)(((char*)task->init_stack) + stack_depth - sizeof(long));
#else
     task->top_stack = task->init_stack;
#endif
#if CONFIG_TASK_STACK_CHECK==ISIX_ON
    memset(task->init_stack,MAGIC_FILL_VALUE,stack_depth);
#endif
    printk("TaskCreate: Top stack SP=%08x\n",task->top_stack);
    //Assign task priority
    task->prio = priority;
    //Task is ready
    task->state = TASK_READY;
    //Create initial task stack context
    task->top_stack = isixp_task_init_stack(task->top_stack,task_func,func_param);
    //Lock scheduler
    isixp_enter_critical();
    //Add task to ready list
    if(isixp_add_task_to_ready_list(task)<0)
    {
        //Free allocated innode
        printk("TaskCreate: Add task to ready list failed.",);
        isix_free(task->top_stack);
        isix_free(task);
	    isixp_exit_critical();
	    return NULL;
    }
    if(isix_scheduler_running==false)
    {
        //Scheduler not running assign task
        if(isix_current_task==NULL) isix_current_task = task;
        else if(isix_current_task->prio>task->prio) isix_current_task = task;
    }
    isixp_exit_critical();
    if(isix_current_task->prio>task->prio && isix_scheduler_running==true)
    {
        //New task have higer priority then current task
	    printk("TaskCreate: Call scheduler new prio %d > old prio %d\n",task->prio,isix_current_task->prio);
        isix_yield();
    }
    return task;
}

/*-----------------------------------------------------------------------*/
/*Change task priority function
 * task - task pointer structure if NULL current prio change
 * new_prio - new priority                                  */
int isixp_task_change_prio(task_t *task,prio_t new_prio,bool yield)
{
	if(isix_get_max_priority()< new_prio )
	{
	   return ISIX_ENOPRIO;
	}
	isixp_enter_critical();
    task_t *taskc = task?task:isix_current_task;
    //Save task prio
    prio_t prio = taskc->prio;
    if(prio==new_prio)
    {
        isixp_exit_critical();
        return ISIX_EOK;
    }
    bool yield_req = false;
    if(taskc->state & TASK_READY)
    {
        printk("Change prio of ready task\n");
        isixp_delete_task_from_ready_list(taskc);
        //Assign new prio
        taskc->prio = new_prio;
        //Add task to ready list
        if(isixp_add_task_to_ready_list(taskc)<0)
        {
            isixp_exit_critical();
            return ISIX_ENOMEM;
        }
        if(new_prio<prio && !(isix_current_task->state&TASK_RUNNING) ) yield_req = true;
    }
    else if(taskc->state & TASK_WAITING)
    {
        printk("Change prio of task waiting on sem\n");
        list_delete(&taskc->inode_sem);
        //Assign new prio
        taskc->prio = new_prio;
        isixp_add_task_to_sem_list(&taskc->sem->sem_task,taskc);
    }
    isixp_exit_critical();
    //Yield processor
    if(yield_req && yield)
    {
        printk("CPUYield request\n");
        isix_yield();
    }
    printk("New prio %d\n",new_prio);
    return ISIX_EOK;
}

/*-----------------------------------------------------------------------*/
//Delete task pointed by struct task
int isix_task_delete(task_t *task)
{
    isixp_enter_critical();
    task_t *taskd = task?task:isix_current_task;
    printk("Task: %08x(SP %08x) to delete",task,taskd->init_stack);
    if(taskd->state & TASK_READY)
    {
       //Task is ready remove from read
        isixp_delete_task_from_ready_list(taskd);
        printk("Remove from ready list\n");
    }
    else if(taskd->state & TASK_SLEEPING)
    {
        //Task sleeping remove from sleeping
        list_delete(&taskd->inode);
        printk("Remove from sleeping list");
    }
    //Task waiting for sem remove from waiting list
    if(taskd->state & TASK_WAITING)
    {
       list_delete(&taskd->inode_sem);
       taskd->sem = NULL;
       printk("Remove from sem list");
    }
    //Add task to delete list
    taskd->state = TASK_DEAD;
    isixp_add_task_to_delete_list(taskd);
    if(task==NULL || task==isix_current_task)
    {
        isixp_exit_critical();
        printk("Current task yield req");
        isix_yield();
        return ISIX_EOK;
    }
    else
    {
        isixp_exit_critical();
        return ISIX_EOK;
    }
}

/*-----------------------------------------------------------------------*/
//Get current thread handler
task_t * isix_task_self(void)
{
    task_t *t = isix_current_task;
    return t;
}

/*-----------------------------------------------------------------------*/
//Stack check for fill value
#if CONFIG_TASK_STACK_CHECK == ISIX_ON

#ifndef CONFIG_STACK_GROWTH
#error isix_free_stack_space() for grown stack not implemented yet
#endif

size_t isix_free_stack_space(const task_t *task)
{
	size_t usage=0;
	unsigned char *bStack = (unsigned char*)task->init_stack;
	while(*bStack==MAGIC_FILL_VALUE)
	{
		bStack++;
		usage++;
	}
	return usage;
}
#endif
/*-----------------------------------------------------------------------*/
