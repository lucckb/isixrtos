#include <isix/config.h>
#include <isix/types.h>
#include <isix/task.h>
#include <isix/memory.h>
#include <isix/prv/semaphore.h>
#include <isix/prv/mutex.h>
#include <isix/prv/common.h>
#include <isix/prv/bitops.h>
#include <string.h>
#include <isix/prv/scheduler.h>
#include <isix/arch/core.h>


#ifdef CONFIG_ISIX_LOGLEVEL_TASK
#undef CONFIG_ISIX_LOGLEVEL
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_TASK
#endif
#include <isix/prv/printk.h>

//Magic value for stack checking
enum { MAGIC_FILL_VALUE = 0x55 };



/* Create task function */
ostask_t isix_task_create(task_func_ptr_t task_func, void *func_param,
		unsigned long  stack_depth, osprio_t priority, unsigned long flags )
{
	pr_info("tskcreate: Create task with prio %i",priority);
    if( (isix_get_min_priority()<priority) && !_isixp_is_idle_prio(priority) )
    {
		return NULL;
    }
	//If stack length is small error
    if(stack_depth<ISIX_PORT_SCHED_MIN_STACK_DEPTH) return NULL;
    //Alignement
	stack_depth = _isixp_align_size( stack_depth );
    //Allocate task_t structure
    ostask_t task = (ostask_t)isix_alloc(sizeof(struct isix_task));
    pr_debug("Alloc task struct %p",task);
    //No free memory
    if(task==NULL) return NULL;
    //Zero task structure
    memset( task, 0, sizeof(*task) );
    //Try Allocate stack for task
    task->init_stack = isix_alloc(stack_depth+ISIX_MEMORY_PROTECTION_EFENCE_SIZE);
    pr_debug("Alloc stack mem %p",task->init_stack);
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
	}
#ifndef CONFIG_ISIX_STACK_ASCENDING
     task->top_stack = (unsigned long*)(((uintptr_t)task->init_stack)
			 + stack_depth + ISIX_MEMORY_PROTECTION_EFENCE_SIZE );
#	if CONFIG_ISIX_MEMORY_PROTECTION_MODEL > 0
	 task->fence_estack = (uintptr_t)task->init_stack;
#	endif /* CONFIG_ISIX_MEMORY_PROTECTION_MODEL  */
#else /*  CONFIG_ISIX_STACK_ASCENDING */
     task->top_stack = task->init_stack;
#	if CONFIG_ISIX_MEMORY_PROTECTION_MODEL > 0
	task->fence_estack = (uintptr_t)task->init_stack+stack_depth+
		ISIX_MEMORY_PROTECTION_EFENCE_SIZE;
#	endif
#endif /*  CONFIG_ISIX_STACK_ASCENDING */
#if CONFIG_ISIX_TASK_STACK_CHECK
    memset( task->init_stack, MAGIC_FILL_VALUE, stack_depth );
#endif	/*  CONFIG_ISIX_TASK_STACK_CHECK */
    pr_debug("Top stack SP=%p",task->top_stack);
    //Assign task priority
    task->prio = priority;
	//Assing real priority
	task->real_prio = priority;
	//Mutex list init
	list_init( &task->owned_mutexes );
	//Waiting tasks list
	list_init( &task->waiting_tasks );
    //Task is ready
    task->state = (flags&isix_task_flag_suspended)?OSTHR_STATE_SUSPEND:OSTHR_STATE_CREATED;
    //Create initial task stack context
    task->top_stack = _isixp_task_init_stack(task->top_stack,task_func,func_param);
	//Reference counter initialization
	//If extra reference flag is passed task is refered twice will not be freed when fin.
	task->refcnt = (flags&isix_task_flag_ref)?(1):(0);
    //Lock scheduler
	if( !(flags&isix_task_flag_suspended) ) {
		isix_enter_critical();
		_isixp_wakeup_task( task, ISIX_EOK );
	}
    return task;
}


/*Change task priority function
 * task - task pointer structure if NULL current prio change
 */
int isix_task_change_prio( ostask_t task, osprio_t new_prio )
{
	if( !schrun ) {
		return ISIX_ESTATE;
	}
	if(isix_get_min_priority()<new_prio ) {
	   return ISIX_ENOPRIO;
	}
	isix_enter_critical();
    ostask_t taskc = task?task:currp;
	pr_debug("Change prio curr task ptr %p %i", taskc, taskc->state );
    //Save task prio
    const osprio_t real_prio = taskc->real_prio;
    if(real_prio==new_prio)
    {
        isix_exit_critical();
        return ISIX_EOK;
    }
	taskc->real_prio = new_prio;
	if( taskc->prio==real_prio||_isixp_prio_gt(new_prio,taskc->prio) ) {
		_isixp_reallocate_priority( taskc, new_prio );
		_isix_port_yield();	//Unconditional port yield due to force reschedule all tasks
	}
	isix_exit_critical();
    return real_prio;
}



//Delete task pointed by struct task
void isix_task_kill( ostask_t task )
{
	//Release all waiting mutexes owned by task
    ostask_t taskd = task?task:currp;
	isix_enter_critical();
	if(    taskd->state == OSTHR_STATE_ZOMBIE
		|| taskd->state == OSTHR_STATE_EXITED ) {
		isix_exit_critical();
		return;
	}
	_isixp_mutex_unlock_all_in_task( taskd );
	_isixp_add_kill_or_set_suspend( taskd, false );
	if( taskd == currp ) {
		isix_exit_critical();
		isix_yield();
	}
	else {
		isix_exit_critical();
	}
}

//Get current thread handler
ostask_t isix_task_self(void)
{
    return currp;
}

//Stack check for fill value
#if CONFIG_ISIX_TASK_STACK_CHECK

#ifdef CONFIG_ISIX_STACK_ASCENDING
#error isix_free_stack_space() for ascending stack not implemented yet
#endif

ssize_t isix_free_stack_space(ostask_t task)
{
	if( task == NULL ) task = currp;
	isix_enter_critical();
	if( !task->init_stack ) {
		isix_exit_critical();
		return ISIX_EBADF;
	}
	size_t freespc=0;
	const ostask_t taskd = task?task:currp;
	volatile unsigned char *b_stack = (volatile unsigned char*)
		(_isix_port_memory_efence_aligna((uintptr_t)taskd->init_stack) +
		 ISIX_MEMORY_PROTECTION_EFENCE_SIZE );

	while(*b_stack==MAGIC_FILL_VALUE) {
		++b_stack;
		++freespc;
	}
	isix_exit_critical();
	return freespc;
}
#endif

/**
 *	Isix get task priority utility function
 *	@return none
 */
osprio_t isix_get_task_priority( const ostask_t task )
{
	const ostask_t taskd = task?task:currp;
	return taskd->real_prio;
}

//Get task origin priority
osprio_t isix_get_task_inherited_priority( const ostask_t task )
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
	isix_enter_critical();
    ostask_t taskd = task?task:currp;
	_isixp_add_kill_or_set_suspend( taskd, true );
	if( taskd==currp ) {
		isix_exit_critical();
		isix_yield();
	}
	else {
		isix_exit_critical();
	}
}

/** Resume the current task */
int isix_task_resume( ostask_t task )
{
	if( task == currp || !task ) {
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	if( task->state == OSTHR_STATE_SUSPEND ) {
		//Wakeup suspended task
		_isixp_wakeup_task( task, ISIX_EOK );
	} else {
		isix_exit_critical();
		return ISIX_ESTATE;
	}
	return ISIX_EOK;
}


// Add extra reference to the task pointer
int isix_task_ref( ostask_t task )
{
    ostask_t taskd = task?task:currp;
	isix_enter_critical();
	int ret = ISIX_EOK;
	//! Unable to reference already released task
	if( taskd->refcnt>=OSREF_T_MAX ) {
		ret = ISIX_EINVARG;
	} else {
		++taskd->refcnt;
	}
	isix_exit_critical();
	return ret;
}


// Remove task reference
int isix_task_unref( ostask_t task )
{
    ostask_t taskd = task?task:currp;
	isix_enter_critical();
	if( taskd->refcnt < 1  ) {
		isix_exit_critical();
		return ISIX_EINVARG;
	} else {
		--taskd->refcnt;
	}
	bool do_clean = ( taskd->refcnt==0 && taskd->state==OSTHR_STATE_EXITED );
	__sync_synchronize();
	isix_exit_critical();
	if( do_clean ) isix_free( taskd );
	return ISIX_EOK;
}

// Wait for selected task to finish
int isix_task_wait_for( ostask_t task )
{
	if( task == currp || !task ) {
		pr_err("Wait for self");
		return ISIX_EINVARG;
	}
	isix_enter_critical();
	if( task->refcnt == 0 ) {
		pr_err("No references");
		isix_exit_critical();
		return ISIX_ENOREF;
	}
	if( task->state==OSTHR_STATE_ZOMBIE || task->state==OSTHR_STATE_EXITED ) {
		isix_exit_critical();
		return ISIX_EOK;
	} else {
		_isixp_set_sleep( OSTHR_STATE_WTEXIT );
		list_insert_end( &task->waiting_tasks, &currp->inode );
		isix_exit_critical();
		isix_yield();
		return task->obj.dmsg;
	}
}


//! Terminate the process when task exits
void __attribute__((noreturn)) _isixp_task_terminator(void)
{
	ostask_t tsk, tmp;
	isix_enter_critical();
	list_for_each_entry_safe( &currp->waiting_tasks, tsk, tmp, inode ) {
		list_delete( &tsk->inode );
		_isixp_wakeup_task_l( tsk, ISIX_EOK );
	}
	isix_exit_critical();
	isix_task_kill(NULL);
	for(;;);
}

