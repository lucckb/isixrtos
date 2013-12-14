#include <isix/config.h>
#include <isix/types.h>
#include <prv/scheduler.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <prv/semaphore.h>
#include <prv/multiple_objects.h>
#include <string.h>

#ifndef ISIX_DEBUG_TASK
#define ISIX_DEBUG_TASK ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_TASK == ISIX_DBG_ON
#include <isix/printk.h>
#else
#undef isix_printk
#define isix_printk(...) do {} while(0)
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
	isix_printk("TaskCreate: Create task with prio %d",priority);
    if(isix_get_min_priority()< priority )
    {
    	return NULL;
    }
	//If stack length is small error
    if(stack_depth<ISIX_PORT_SCHED_MIN_STACK_DEPTH) return NULL;
    //Alignement
    if(stack_depth & ALIGN_MASK)
    {
        stack_depth += ALIGN_BYTES - (stack_depth & ALIGN_MASK);
    }
    //Allocate task_t structure
    task_t *task = (task_t*)isix_alloc(sizeof(task_t));
    isix_printk("Alloc task struct %08x",task);
    //No free memory
    if(task==NULL) return NULL;
    //Zero task structure
    memset(task,0,sizeof(task_t));
    //Try Allocate stack for task
    task->init_stack = isix_alloc(stack_depth);
    isix_printk("Alloc stack mem %08x",task->init_stack);
    if(task->init_stack==NULL)
    {
        //Free allocated stack memory
        isix_free(task);
        return NULL;
    }
#ifdef ISIX_CONFIG_STACK_GROWTH
     task->top_stack = (unsigned long*)(((char*)task->init_stack) + stack_depth - sizeof(long));
#else
     task->top_stack = task->init_stack;
#endif
#if ISIX_CONFIG_TASK_STACK_CHECK==ISIX_ON
    memset(task->init_stack,MAGIC_FILL_VALUE,stack_depth);
#endif
    isix_printk("Top stack SP=%08x",task->top_stack);
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
        isix_printk("Add task to ready list failed.");
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
	    isix_printk("Call scheduler new prio %d > old prio %d",task->prio,isix_current_task->prio);
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
	if(isix_get_min_priority()< new_prio )
	{
	   return ISIX_ENOPRIO;
	}
	isixp_enter_critical();
    task_t *taskc = task?task:isix_current_task;
    //Save task prio
    const prio_t prio = taskc->prio;
    if(prio==new_prio)
    {
        isixp_exit_critical();
        return ISIX_EOK;
    }
    bool yield_req = false;
    if(taskc->state & TASK_READY)
    {
        isix_printk("Change prio of ready task");
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
        isix_printk("Change prio of task waiting on sem");
        list_delete(&taskc->inode_sem);
        //Assign new prio
        taskc->prio = new_prio;
        isixp_add_task_to_sem_list(&taskc->sem->sem_task,taskc);
    }
    isixp_exit_critical();
    //Yield processor
    if(yield_req && yield)
    {
        isix_printk("CPUYield request");
        isix_yield();
    }
    isix_printk("New prio %d\n",new_prio);
    return prio;
}
/*-----------------------------------------------------------------------*/
/* Get isix structure private data */
void* isix_get_task_private_data( task_t *task )
{
	if( !task )
		return NULL;
	isixp_enter_critical();
	void* d = task->prv;
	isixp_exit_critical();
	return d;
}
/*-----------------------------------------------------------------------*/
/* Isix set private data task */
int isix_set_task_private_data( task_t *task, void *data )
{
	if( !task )
	{
		return ISIX_EINVARG;
	}
	isixp_enter_critical();
	if( task->prv )
	{
		isixp_exit_critical();
		return ISIX_EINVARG;
	}
	task->prv = data;
	isixp_exit_critical();
	return ISIX_EOK;
}
/*-----------------------------------------------------------------------*/
//Delete task pointed by struct task
int isix_task_delete(task_t *task)
 {
    isixp_enter_critical();
    task_t *taskd = task?task:isix_current_task;
    isix_printk("Task: %08x(SP %08x) to delete",task,taskd->init_stack);
    if(taskd->state & TASK_READY)
    {
       //Task is ready remove from read
        isixp_delete_task_from_ready_list(taskd);
        isix_printk("Remove from ready list\n");
    }
    else if(taskd->state & TASK_SLEEPING)
    {
        //Task sleeping remove from sleeping
        list_delete(&taskd->inode);
        isix_printk("Remove from sleeping list");
    }
    //Task waiting for sem remove from waiting list
    if(taskd->state & TASK_WAITING)
    {
       list_delete(&taskd->inode_sem);
       taskd->sem = NULL;
       isix_printk("Remove from sem list");
    }
#if ISIX_CONFIG_USE_MULTIOBJECTS
    if( taskd->state & TASK_WAITING_MULTIPLE )
    {
    	isixp_delete_from_multiple_wait_list( taskd );
    	isix_printk("Remove item from multiple list");
    }
#endif
    //Add task to delete list
    taskd->state = TASK_DEAD;
    isixp_add_task_to_delete_list(taskd);
    if(task==NULL || task==isix_current_task)
    {
        isixp_exit_critical();
        isix_printk("Current task yield req");
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
#if ISIX_CONFIG_TASK_STACK_CHECK == ISIX_ON

#ifndef ISIX_CONFIG_STACK_GROWTH
#error isix_free_stack_space() for grown stack not implemented yet
#endif

size_t isix_free_stack_space(const task_t *task)
{
	size_t usage=0;
	const task_t *taskd = task?task:isix_current_task;
	unsigned char *b_stack = (unsigned char*)taskd->init_stack;
	while(*b_stack==MAGIC_FILL_VALUE)
	{
		b_stack++;
		usage++;
	}
	return usage;
}
#endif
/*-----------------------------------------------------------------------*/
/**
 *	Isix get task priority utility function
 *	@return none 
 */
/*-----------------------------------------------------------------------*/
prio_t isix_get_task_priority( const task_t* task )
{
	const task_t *taskd = task?task:isix_current_task;
	return taskd->prio;
}
/*-----------------------------------------------------------------------*/
