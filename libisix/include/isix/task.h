#pragma once
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/memory.h>
/*-----------------------------------------------------------------------*/
#ifndef __cplusplus
//!Definition of task function in C mode
#define ISIX_TASK_FUNC(FUNC, ARG)							\
	void FUNC(void *ARG)

#endif
/* ----------------------------------------------------------------------*/
//! Special flags for task create
enum isix_task_flags {
	isix_task_flag_newlib = 1U	//! Uses per thread new lib data for ex errno
};
/*-----------------------------------------------------------------------*/
/** Create the task function (System thread)
 * @param[in] task_func Pointer to the thread function
 * @param[in] func_param Function parameter
 * @param[in] stack_depth Thread/Task stack depth
 * @param[in] priority The task priority
 * @param[in] flags extra flags for control task parameters
 * @return Task control object, or NULL when task can't be created */
ostask_t isix_task_create(task_func_ptr_t task_func, void *func_param, 
		unsigned long stack_depth, osprio_t priority, unsigned long flags );

/*-----------------------------------------------------------------------*/
/** Change the task/thread priority
 * @param[in] task Task pointer structure if NULL change the current prio
 * @param[in] new_prio New task priority
 * @return old priority if the operation is completed successfully otherwise return an error code
 */
int isix_task_change_prio( ostask_t task, osprio_t new_prio );

/*-----------------------------------------------------------------------*/

/** Delete the task pointed by the task control object
 *	@param[in] task Task control object
 */
void isix_task_kill( ostask_t task );

/*-----------------------------------------------------------------------*/

/** Get current thread handler
 *  @return Get the task control block of the current task/thread
 */
ostask_t isix_task_self(void);


/*-----------------------------------------------------------------------*/
/* Isix set private data task
 * This function assign private data to the current task control block
 * The data can be assigned only once. The memory is managed manualy
 * and it should be deletede before task deletion
 * @param [in] task Task control object
 * @param [in] data Private data pointer assigned to the task
 * @return ISIX_EOK if success, ISIX_EINVAL when pointer is already assigned */
int isix_set_task_private_data( ostask_t task, void *data );

/*-----------------------------------------------------------------------*/
/* Get isix structure private data
 * Get the isix private data pointer asigned to the task
 * @param [in] task Task control object
 * @return private data pointer
 * */
void* isix_get_task_private_data( ostask_t task );

/*-----------------------------------------------------------------------*/
/**
 *	Isix get task priority utility function
 *	@param[in] task Task control object
 *	@return current task priority
 */
 osprio_t isix_get_task_priority( const ostask_t task );

/*-----------------------------------------------------------------------*/
/** Check of the available stack space
 * @param[in] task Task control block
 * @return Size of the number of bytes used by the task/thread
 */
#ifdef ISIX_CONFIG_TASK_STACK_CHECK
size_t isix_free_stack_space(const ostask_t task);
#endif

/*-----------------------------------------------------------------------*/

#ifdef WITH_ISIX_TCPIP_LIB
task_t* sys_thread_new(const char *name, task_func_ptr_t thread,  void *arg, int stacksize, int prio);
/* Isix task create TCPIP version for usage with the TCPIP stack */
static inline ostask_t isix_task_create_tcpip(task_func_ptr_t task_func, 
		void *func_param, unsigned long stack_depth, osprio_t priority )
{
	return sys_thread_new( NULL, task_func, func_param, stack_depth, priority );
}

/* Isix task delete TCPIP version */
static inline int isix_task_delete_tcpip(ostask_t task)
{
	void *prv = isix_get_task_private_data( task );
	int ret = isix_task_delete( task );
	if( prv ) isix_free( prv );
	return ret;
}
#endif

#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */

