#include <isix/config.h>
#include <isix/types.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <isix/prv/semaphore.h>
#include <isix/prv/common.h>
#include <string.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>

#ifndef ISIX_DEBUG_TASK
#define ISIX_DEBUG_TASK ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_TASK == ISIX_DBG_ON
#include <isix/printk.h>
#else
#undef printk
#define printk(...) do {} while(0)
#endif

//Magic value for stack checking
enum { MAGIC_FILL_VALUE = 0x55 };

/* Create task function */
ostask_t isix_task_create(task_func_ptr_t task_func, void *func_param, 
		unsigned long  stack_depth, osprio_t priority, unsigned long flags )
{
	printk("tskcreate: Create task with prio %i",priority);
    if(isix_get_min_priority()< priority )
    {
    	return NULL;
    }
	//If stack length is small error
    if(stack_depth<ISIX_PORT_SCHED_MIN_STACK_DEPTH) return NULL;
    //Alignement
	stack_depth = _isixp_align_size( stack_depth );
    //Allocate task_t structure
    ostask_t task = (ostask_t)isix_alloc(sizeof(struct isix_task));
    printk("Alloc task struct %p",task);
    //No free memory
    if(task==NULL) return NULL;
    //Zero task structure
    memset( task, 0, sizeof(*task) );
    //Try Allocate stack for task
    task->init_stack = isix_alloc(stack_depth);
    printk("Alloc stack mem %p",task->init_stack);
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
    printk("Top stack SP=%p",task->top_stack);
    //Assign task priority
    task->prio = priority;
    //Task is ready
    task->state = OSTHR_STATE_CREATED;
    //Create initial task stack context
    task->top_stack = _isixp_task_init_stack(task->top_stack,task_func,func_param);
    //Lock scheduler
    _isixp_enter_critical();
	_isixp_wakeup_task( task, ISIX_EOK );
	//_isixp_exit_critical();
    return task;
}


/*Change task priority function
 * task - task pointer structure if NULL current prio change
 */
int isix_task_change_prio( ostask_t task, osprio_t new_prio )
{
	if(isix_get_min_priority()<new_prio )
	{
	   return ISIX_ENOPRIO;
	}
	_isixp_enter_critical();
    ostask_t taskc = task?task:currp;
	printk("Change prio curr task ptr %p %i", taskc, taskc->state );
    //Save task prio
    const osprio_t prio = taskc->prio;
    if(prio==new_prio)
    {
        _isixp_exit_critical();
        return ISIX_EOK;
    }
	_isixp_reallocate_priority( taskc, new_prio );
	_isixp_do_reschedule();
    return prio;
}

/* Get isix structure private data */
void* isix_get_task_private_data( ostask_t task )
{
	if( !task ) {
		return NULL;
	}
	_isixp_enter_critical();
	void* d = task->prv;
	_isixp_exit_critical();
	return d;
}

/* Isix set private data task */
int isix_set_task_private_data( ostask_t task, void *data )
{
	if( !task ) {
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

//Delete task pointed by struct task
void isix_task_kill( ostask_t task )
{
	_isixp_enter_critical();
    ostask_t taskd = task?task:currp;
	_isixp_add_kill_or_set_suspend( taskd, false );
	if( !task ) {
		_isixp_exit_critical();
		isix_yield();
	} 
	else {
		_isixp_exit_critical();
	}
}

//Get current thread handler
ostask_t isix_task_self(void)
{
    return currp;
}

//Stack check for fill value
#ifdef ISIX_CONFIG_TASK_STACK_CHECK

#ifndef ISIX_CONFIG_STACK_GROWTH
#error isix_free_stack_space() for growth stack not implemented yet
#endif

size_t isix_free_stack_space(const ostask_t task)
{
	size_t usage=0;
	const ostask_t taskd = task?task:currp;
	unsigned char *b_stack = (unsigned char*)taskd->init_stack;
	while(*b_stack==MAGIC_FILL_VALUE)
	{
		b_stack++;
		usage++;
	}
	return usage;
}
#endif

/**
 *	Isix get task priority utility function
 *	@return none 
 */
osprio_t isix_get_task_priority( const ostask_t task )
{
	const ostask_t taskd = task?task:currp;
	return taskd->prio;
}

/** Get current task state 
 * @param[in] Task identifier
 * @return Task state
 */
enum osthr_state isix_get_task_state( const ostask_t task )
{
	const ostask_t taskd = task?task:currp;
	return taskd->state;
}

//! Set task to suspend state
void isix_task_suspend( ostask_t task )
{
	_isixp_enter_critical();
    ostask_t taskd = task?task:currp;
	_isixp_add_kill_or_set_suspend( taskd, true );
	if( !task ) {
		_isixp_exit_critical();
		isix_yield();
	} 
	else {
		_isixp_exit_critical();
	}
}

/** Resume the current task
 * @param[in] Task identifier 
 * @return Error code
 */
int isix_task_resume( ostask_t task )
{
	if( task == currp || !task ) {
		return ISIX_EINVARG;
	}
	_isixp_enter_critical();
	if( task->state == OSTHR_STATE_SUSPEND ) {
		//Wakeup suspended task
		_isixp_wakeup_task( task, ISIX_EOK );
	} else {
		_isixp_exit_critical();
		return ISIX_ESTATE;
	}
	return ISIX_EOK;
}
