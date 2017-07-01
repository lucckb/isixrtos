#pragma once
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/memory.h>
#include <isix/osthr_state.h>
#include <isix/arch/core.h>

#ifndef __cplusplus
//!Definition of task function in C mode
#define ISIX_TASK_FUNC(FUNC, ARG)							\
	void FUNC(void *ARG)
#endif

//! Special flags for task create
enum isix_task_flags {
	isix_task_flag_newlib = 1U,		//! Uses per thread new lib data for ex errno
	isix_task_flag_suspended = 2U,	//! Create task suspended
	isix_task_flag_ref = 8U			//! Extra reference non should be called task_unref();
};

//! Minimal stack DEPTH
enum {
	ISIX_MIN_STACK_SIZE = ISIX_PORT_SCHED_MIN_STACK_DEPTH
};

/** Create the task function (System thread)
 * @param[in] task_func Pointer to the thread function
 * @param[in] func_param Function parameter
 * @param[in] stack_depth Thread/Task stack depth
 * @param[in] priority The task priority
 * @param[in] flags extra flags for control task parameters
 * @return Task control object, or NULL when task can't be created */
ostask_t isix_task_create(task_func_ptr_t task_func, void *func_param,
		unsigned long stack_depth, osprio_t priority, unsigned long flags );


/** Change the task/thread priority
 * @param[in] task Task pointer structure if NULL change the current prio
 * @param[in] new_prio New task priority
 * @return old priority if the operation is completed successfully otherwise return an error code
 */
int isix_task_change_prio( ostask_t task, osprio_t new_prio );


/** Delete the task pointed by the task control object
 *	@param[in] task Task control object
 */
void isix_task_kill( ostask_t task );


/** Get current thread handler
 *  @return Get the task control block of the current task/thread
 */
ostask_t isix_task_self(void);


/**
 *	Isix get task priority utility function
 *	@param[in] task Task control object
 *	@return current task priority
 */
 osprio_t isix_get_task_priority( const ostask_t task );

/**
 *	Isix get task inherided priority (incl. prio inheritance )
 *	@param[in] task Task control object
 *	@return current task priority
 */
 osprio_t isix_get_task_inherited_priority( const ostask_t task );

/** Check of the available stack space
 * @param[in] task Task control block
 * @return Number of bytes in stack space or errno
 */
#if CONFIG_ISIX_TASK_STACK_CHECK
_ssize_t isix_free_stack_space( const ostask_t task );
#endif

/** Suspend the current task
 * @param[in] Task identifier
 * @return Error code
 */
void isix_task_suspend( ostask_t task );

/** Resume the current task
 * @param[in] Task identifier
 * @return Error code
 */
int isix_task_resume( ostask_t task );

/** Get current task state
 * @param[in] Task identifier
 * @return Task state
 */
enum osthr_state isix_get_task_state( const ostask_t task );


/** @brief Add extra reference to the task pointer
 * @task[in] task Input task to be referenced
 * @return reference status */
int isix_task_ref( ostask_t task );


/** @brief Remove task reference
 * @note release the task memory if count is zero
 * @param[in] Input task reference
 * @return reference status */
int isix_task_unref( ostask_t task );

/** @brief Wait for selected task to finish
 *  @param[in] task Input task to wait for
 *  @return Task waiting status
 */
int isix_task_wait_for( ostask_t task );


#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */


#ifdef __cplusplus
namespace isix {
	using task_t = ostask_t;
	using thr_state = osthr_state;
namespace {
	inline ostask_t task_create(task_func_ptr_t task_func, void *func_param, 
			unsigned long stack_depth, osprio_t priority, unsigned long flags=0 ) {
		return ::isix_task_create( task_func, func_param, stack_depth,
				priority, flags );
	}
	inline int task_change_prio( ostask_t task, osprio_t new_prio ) {
		return ::isix_task_change_prio( task, new_prio );
	}
	inline void task_kill( ostask_t task ) {
		::isix_task_kill( task );
	}
	inline ostask_t task_self() {
		return ::isix_task_self();
	}
	inline osprio_t get_task_priority( const ostask_t task=nullptr ) {
		return ::isix_get_task_priority( task );
	}
	inline osprio_t get_task_inherited_priority( const ostask_t task=nullptr ) {
		return ::isix_get_task_inherited_priority( task );
	}
#if CONFIG_ISIX_TASK_STACK_CHECK
	inline ssize_t free_stack_space( const ostask_t task=nullptr ) {
		return ::isix_free_stack_space( task );
	}
#endif
	inline void task_suspend( ostask_t task = nullptr ) {
		::isix_task_suspend( task );
	}
	inline int task_resume( ostask_t task ) {
		return ::isix_task_resume( task );
	}
	inline thr_state get_task_state( const ostask_t task=nullptr ) {
		return ::isix_get_task_state( task );
	}
	inline int task_ref( ostask_t task ) {
		return ::isix_task_ref( task );
	}
	inline int task_unref( ostask_t task ) {
		return ::isix_task_unref( task );
	}
	inline int task_wait_for( ostask_t task ) {
		return ::isix_task_wait_for( task );
	}
}}
#endif /* __cplusplus */
