#ifndef _ISIX_TASK_H
#define _ISIX_TASK_H
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/memory.h>
/*-----------------------------------------------------------------------*/
#ifndef __cplusplus
//!Definition of task function in C mode
#define ISIX_TASK_FUNC(FUNC, ARG)							\
	void FUNC(void *ARG) __attribute__ ((noreturn));	\
	void FUNC(void *ARG)

#endif
/*-----------------------------------------------------------------------*/
/** Create the task function (System thread)
 * @param[in] task_func Pointer to the thread function
 * @param[in] func_param Function parameter
 * @param[in] stack_depth Thread/Task stack depth
 * @param[in] priority The task priority
 * @return Task control object, or NULL when task can't be created */
task_t* isix_task_create(task_func_ptr_t task_func, void *func_param, unsigned long stack_depth, prio_t priority);

/*-----------------------------------------------------------------------*/
//! Private version of change prio function
int isixp_task_change_prio(task_t *task,prio_t new_prio,bool yield);

/*-----------------------------------------------------------------------*/
/** Change the task/thread priority
 * @param[in] task Task pointer structure if NULL change the current prio
 * @param[in] new_prio New task priority
 * @return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
static inline int isix_task_change_prio( task_t* task, prio_t new_prio )
{
	return isixp_task_change_prio(task,new_prio,true);
}

/*-----------------------------------------------------------------------*/

/** Delete the task pointed by the task control object
 *	@param[in] task Task control object
 *	@return ISIX_EOK if the operation is completed successfully otherwise return an error code
 */
int isix_task_delete(task_t *task);

/*-----------------------------------------------------------------------*/

/** Get current thread handler
 *  @return Get the task control block of the current task/thread
 */
task_t* isix_task_self(void);


/*-----------------------------------------------------------------------*/
/* Isix set private data task
 * This function assign private data to the current task control block
 * The data can be assigned only once. The memory is managed manualy
 * and it should be deletede before task deletion
 * @param [in] task Task control object
 * @param [in] data Private data pointer assigned to the task
 * @return ISIX_EOK if success, ISIX_EINVAL when pointer is already assigned */
int isix_set_task_private_data( task_t *task, void *data );

/*-----------------------------------------------------------------------*/
/* Get isix structure private data
 * Get the isix private data pointer asigned to the task
 * @param [in] task Task control object
 * @return private data pointer
 * */
void* isix_get_task_private_data( task_t *task );

/*-----------------------------------------------------------------------*/
/** Check of the available stack space
 * @param[in] task Task control block
 * @return Size of the number of bytes used by the task/thread
 */
#if ISIX_CONFIG_TASK_STACK_CHECK == ISIX_ON
size_t isix_free_stack_space(const task_t *task);
#endif

/*-----------------------------------------------------------------------*/

#ifdef WITH_ISIX_TCPIP_LIB
task_t* sys_thread_new(const char *name, task_func_ptr_t thread,  void *arg, int stacksize, int prio);
/* Isix task create TCPIP version for usage with the TCPIP stack */
static inline task_t* isix_task_create_tcpip(task_func_ptr_t task_func, void *func_param, unsigned long stack_depth, prio_t priority)
{
	return sys_thread_new( NULL, task_func, func_param, stack_depth, priority );
}

/* Isix task delete TCPIP version */
static inline int isix_task_delete_tcpip(task_t *task)
{
	void *prv = isix_get_task_private_data( task );
	int ret = isix_task_delete( task );
	isix_free( prv );
	return ret;
}
#endif

#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */

/****************************** C++ API ***********************************/
#ifdef __cplusplus
#include <cstddef>

namespace isix {
/*-----------------------------------------------------------------------*/
//! C++ wrapper for the task/thread
class task_base
{
public:
	/** Construct the task
	 * @param[in] stack_depth Stack depth of the thread/task
	 * @param[in] priority Thread/task priority
	 */
#ifdef WITH_ISIX_TCPIP_LIB
	explicit task_base(std::size_t stack_depth, prio_t priority, bool tcpip=false)
	{
		if(tcpip)
			task_id = isix_task_create_tcpip( start_task, this, stack_depth, priority );
		else
			task_id = isix_task_create( start_task, this, stack_depth, priority );
	}
#else
	explicit task_base(std::size_t stack_depth, prio_t priority)
	{
		task_id = isix_task_create( start_task, this, stack_depth, priority );
	}
#endif
	//! Destruct the task/thread object
	virtual ~task_base()
	{
		isix_task_delete(task_id);
	}
	/** Get thread task id
	 * @return Task control object
	 */
	task_t* get_taskid() { return task_id; }
	/** Check the fifo object is in valid state
	 * @return True when the object is in valid state
	 */
	bool is_valid() { return task_id!=0; }
protected:
	/** Pure virtual method for the object main thread */
	virtual void main() = 0;

private:
	__attribute__ ((noreturn)) static void start_task(void *ptr)
	{
		static_cast<task_base*>(ptr)->main();
		while(1) isix_wait(-1);
	}
private:
	task_base(const task_base&);
	task_base& operator=(const task_base&);
private:
	task_t *task_id;
};

/*-----------------------------------------------------------------------*/
}
#endif /* __cplusplus */
/*-----------------------------------------------------------------------*/
#endif /* __ISIX_TASK_H */

/*-----------------------------------------------------------------------*/
