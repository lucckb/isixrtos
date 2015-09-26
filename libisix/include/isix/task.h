#pragma once
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/memory.h>
#include <isix/osthr_state.h>

#ifndef __cplusplus
//!Definition of task function in C mode
#define ISIX_TASK_FUNC(FUNC, ARG)							\
	void FUNC(void *ARG)

#endif
//! Special flags for task create
enum isix_task_flags {
	isix_task_flag_newlib = 1U,		//! Uses per thread new lib data for ex errno
	isix_task_flag_suspended = 2U	//! Create task suspended
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


/* Isix set private data task
 * This function assign private data to the current task control block
 * The data can be assigned only once. The memory pointer should be
 * allocated on the heap with @see isix_alloc. Memory will be deleted automaticaly
 * when task will be deleted.
 * @param [in] task Task control object
 * @param [in] data Private data pointer assigned to the task
 * @return ISIX_EOK if success, ISIX_EINVAL when pointer is already assigned */
int isix_set_task_private_data( ostask_t task, void *data );

/* Get isix structure private data
 * Get the isix private data pointer asigned to the task
 * @param [in] task Task control object
 * @return private data pointer
 * */
void* isix_get_task_private_data( ostask_t task );

/**
 *	Isix get task priority utility function
 *	@param[in] task Task control object
 *	@return current task priority
 */
 osprio_t isix_get_task_priority( const ostask_t task );

/** Check of the available stack space
 * @param[in] task Task control block
 * @return Number of bytes in stack space
 */
#ifdef ISIX_CONFIG_TASK_STACK_CHECK
size_t isix_free_stack_space( const ostask_t task );
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


#ifdef WITH_ISIX_TCPIP_LIB
ostask_t sys_thread_new(const char *name, task_func_ptr_t thread,  void *arg, int stacksize, int prio);
/* Isix task create TCPIP version for usage with the TCPIP stack */
static inline ostask_t isix_task_create_tcpip(task_func_ptr_t task_func, 
		void *func_param, unsigned long stack_depth, osprio_t priority )
{
	return sys_thread_new( NULL, task_func, func_param, stack_depth, priority );
}

/* Isix task delete TCPIP version */
static inline void isix_task_delete_tcpip(ostask_t task)
{
	isix_task_kill( task );
}
#endif /* WITH_ISIX_TCPIP_LIB */


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

#ifdef WITH_ISIX_TCPIP_LIB
	/* Isix task create TCPIP version for usage with the TCPIP stack */
	static inline ostask_t task_create_tcpip(task_func_ptr_t task_func, 
			void *func_param, unsigned long stack_depth, osprio_t priority )
	{
		return ::isix_task_create_tcpip(task_func,func_param, stack_depth, priority );
	}
#endif
	inline int task_change_prio( ostask_t task, osprio_t new_prio ) {
		return ::isix_task_change_prio( task, new_prio );
	}
	inline void task_kill( ostask_t task ) {
		::isix_task_kill( task );
	}
	inline ostask_t task_self() {
		return ::isix_task_self();
	}
	inline int set_task_private_data( ostask_t task, void *data ) {
		return ::isix_set_task_private_data( task, data );
	}
	inline void* get_task_private_data( ostask_t task ) {
		return ::isix_get_task_private_data( task );
	}
	inline osprio_t get_task_priority( const ostask_t task=nullptr ) {
		return ::isix_get_task_priority( task );
	}
#ifdef ISIX_CONFIG_TASK_STACK_CHECK
	inline size_t free_stack_space( const ostask_t task=nullptr ) {
		return ::isix_free_stack_space( task );
	}
#endif
	inline void task_suspend( ostask_t task ) {
		::isix_task_suspend( task );
	}
	inline int task_resume( ostask_t task ) {
		return ::isix_task_resume( task );
	}
	inline thr_state get_task_state( const ostask_t task=nullptr ) {
		return ::isix_get_task_state( task );
	}
}}
#endif /* __cplusplus */
