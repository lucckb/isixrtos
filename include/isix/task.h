#ifndef __ISIX_TASK_H
#define __ISIX_TASK_H
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
#include <isix/types.h>
#include <isix/scheduler.h>

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
/** Check of the available stack space
 * @param[in] task Task control block
 * @return Size of the number of bytes used by the task/thread
 */
#if ISIX_CONFIG_TASK_STACK_CHECK == ISIX_ON
size_t isix_free_stack_space(const task_t *task);
#endif

/*-----------------------------------------------------------------------*/

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
	explicit task_base(std::size_t stack_depth, prio_t priority)
	{
		task_id = isix_task_create( start_task, this, stack_depth, priority );
	}
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
	task_t *task_id;
};


/*-----------------------------------------------------------------------*/
}
#endif /* __cplusplus */
/*-----------------------------------------------------------------------*/
#endif /* __ISIX_TASK_H */

/*-----------------------------------------------------------------------*/
