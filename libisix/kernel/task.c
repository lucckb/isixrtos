#include <isix/config.h>
#include <isix/types.h>
#include <isix/prv/scheduler.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <isix/prv/semaphore.h>
#include <isix/prv/common.h>
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
//Magic value for stack checking
enum { MAGIC_FILL_VALUE = 0x55 };
/*-----------------------------------------------------------------------*/
/* Create task function */
task_t* isix_task_create(task_func_ptr_t task_func, void *func_param, 
		unsigned long  stack_depth, prio_t priority, unsigned long flags )
{
	(void)task_func; (void)task_func; (void)func_param;
	(void)stack_depth; (void)priority; (void)flags;
#if 0
	isix_printk("TaskCreate: Create task with prio %d",priority);
    if(isix_get_min_priority()< priority )
    {
    	return NULL;
    }
	//If stack length is small error
    if(stack_depth<ISIX_PORT_SCHED_MIN_STACK_DEPTH) return NULL;
    //Alignement
	stack_depth = _isixp_align_size( stack_depth );
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
	//! Try to allocate impure data if needed
	if( flags & isix_task_flag_newlib )
	{
		task->impure_data =  isix_alloc( sizeof(struct _reent) );
		if( !task->impure_data )
		{
			isix_free(task->init_stack);
			isix_free( task );
			return NULL;
		}
		memset( task->impure_data, 0, sizeof(struct _reent) );
		task->impure_data->_current_locale = "C";
	}
#ifdef ISIX_CONFIG_STACK_GROWTH
     task->top_stack = (unsigned long*)(((char*)task->init_stack) + stack_depth - sizeof(long));
#else
     task->top_stack = task->init_stack;
#endif
#ifdef ISIX_CONFIG_TASK_STACK_CHECK
    memset(task->init_stack,MAGIC_FILL_VALUE,stack_depth);
#endif
    isix_printk("Top stack SP=%08x",task->top_stack);
    //Assign task priority
    task->prio = priority;
    //Task is ready
    task->state = THR_STATE_READY;
    //Create initial task stack context
    task->top_stack = _isixp_task_init_stack(task->top_stack,task_func,func_param);
    //Lock scheduler
    _isixp_enter_critical();
    //Add task to ready list
    if(_isixp_add_task_to_ready_list(task)<0)
    {
        //Free allocated innode
        isix_printk("Add task to ready list failed.");
        isix_free(task->top_stack);
        isix_free(task);
	    _isixp_exit_critical();
	    return NULL;
    }
	_isixp_do_reschedule();
    _isixp_exit_critical();
    return task;
#endif
}

/*-----------------------------------------------------------------------*/
/*Change task priority function
 * task - task pointer structure if NULL current prio change
 * new_prio - new priority                                  */
int _isixp_task_change_prio(task_t *task,prio_t new_prio,bool yield)
{
	(void)task; (void)new_prio; (void)yield; 
	return -1;
#if 0
	if(isix_get_min_priority()< new_prio )
	{
	   return ISIX_ENOPRIO;
	}
	_isixp_enter_critical();
    task_t *taskc = task?task:_isix_current_task;
    //Save task prio
    const prio_t prio = taskc->prio;
    if(prio==new_prio)
    {
        _isixp_exit_critical();
        return ISIX_EOK;
    }
    bool yield_req = false;
    if(taskc->state & TASK_READY)
    {
        isix_printk("Change prio of ready task");
        _isixp_delete_task_from_ready_list(taskc);
        //Assign new prio
        taskc->prio = new_prio;
        //Add task to ready list
        if(_isixp_add_task_to_ready_list(taskc)<0)
        {
            _isixp_exit_critical();
            return ISIX_ENOMEM;
        }
        if(new_prio<prio && !(_isix_current_task->state&TASK_RUNNING) ) yield_req = true;
    }
    else if(taskc->state & TASK_WAITING)
    {
        isix_printk("Change prio of task waiting on sem");
        list_delete( &taskc->inode );
        //Assign new prio
        taskc->prio = new_prio;
        _isixp_add_task_to_sem_list(&taskc->sem->sem_task,taskc);
    }
    _isixp_exit_critical();
    //Yield processor
    if(yield_req && yield)
    {
        isix_printk("CPUYield request");
        isix_yield();
    }
    isix_printk("New prio %d\n",new_prio);
    return prio;
#endif
}
/*-----------------------------------------------------------------------*/
/* Get isix structure private data */
void* isix_get_task_private_data( task_t *task )
{
	if( !task )
		return NULL;
	_isixp_enter_critical();
	void* d = task->prv;
	_isixp_exit_critical();
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
	_isixp_enter_critical();
	if( task->prv )
	{
		_isixp_exit_critical();
		return ISIX_EINVARG;
	}
	task->prv = data;
	_isixp_exit_critical();
	return ISIX_EOK;
}
/*-----------------------------------------------------------------------*/
//Delete task pointed by struct task
int isix_task_delete(task_t *task)
 {
	 (void)task;
	 return 0;
#if 0
    _isixp_enter_critical();
    task_t *taskd = task?task:_isix_current_task;
    isix_printk("Task: %08x(SP %08x) to delete",task,taskd->init_stack);
    if(taskd->state & TASK_READY)
    {
       //Task is ready remove from read
        _isixp_delete_task_from_ready_list(taskd);
        isix_printk("Remove from ready list\n");
    }
    else if(taskd->state & TASK_SLEEPING)
    {
        //Task sleeping remove from sleeping
        list_delete(&taskd->inode_time);
        isix_printk("Remove from sleeping list");
    }
    //Task waiting for sem remove from waiting list
    if(taskd->state & TASK_WAITING)
    {
       list_delete(&taskd->inode );
       taskd->sem = NULL;
       isix_printk("Remove from sem list");
    }
    //Add task to delete list
    taskd->state = TASK_DEAD;
    _isixp_add_task_to_delete_list(taskd);
    if(task==NULL || task==_isix_current_task)
    {
        _isixp_exit_critical();
        isix_printk("Current task yield req");
        isix_yield();
        return ISIX_EOK;
    }
    else
    {
        _isixp_exit_critical();
        return ISIX_EOK;
    }
#endif
}

/*-----------------------------------------------------------------------*/
//Get current thread handler
task_t * isix_task_self(void)
 {
    task_t *t = _isix_current_task;
    return t;
}

/*-----------------------------------------------------------------------*/
//Stack check for fill value
#ifdef ISIX_CONFIG_TASK_STACK_CHECK

#ifndef ISIX_CONFIG_STACK_GROWTH
#error isix_free_stack_space() for grown stack not implemented yet
#endif

size_t isix_free_stack_space(const task_t *task)
{
	size_t usage=0;
	const task_t *taskd = task?task:_isix_current_task;
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
	const task_t *taskd = task?task:_isix_current_task;
	return taskd->prio;
}
/*-----------------------------------------------------------------------*/
