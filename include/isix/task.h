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
#define ISIX_TASK_CURRENT NULL


/*-----------------------------------------------------------------------*/
//Definition of task function
#define ISIX_TASK_FUNC(FUNC, ARG)							\
	void FUNC(void *ARG) __attribute__ ((noreturn));	\
	void FUNC(void *ARG)


/*-----------------------------------------------------------------------*/
/* Create task function */
task_t* isix_task_create(task_func_ptr_t task_func, void *func_param, unsigned long stack_depth, prio_t priority);

/*-----------------------------------------------------------------------*/
/*Change task priority function
 * task - task pointer structure if NULL current prio change
 * new_prio - new priority                                  */
int isixp_task_change_prio(task_t *task,prio_t new_prio,bool yield);

static inline int isix_task_change_prio( task_t* task, prio_t new_prio )
{
	return isixp_task_change_prio(task,new_prio,true);
}

/*-----------------------------------------------------------------------*/

//Delete task pointed by struct task
int isix_task_delete(task_t *task);

/*-----------------------------------------------------------------------*/

//Get current thread handler
task_t* isix_task_self(void);

/*-----------------------------------------------------------------------*/
//Stack check for fill value
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
//Base class for task creation
class task_base
{
public:
	//Default constructor
	explicit task_base(std::size_t stack_depth, prio_t priority)
	{
		task_id = isix_task_create( start_task, this, stack_depth, priority );
	}
	virtual ~task_base()
	{
		isix_task_delete(task_id);
	}
	//Get task id
	task_t* get_taskid() { return task_id; }
	//Check the fifo object is in valid state
	bool is_valid() { return task_id!=0; }
protected:
	virtual void main() = 0;

private:
	__attribute__ ((noreturn)) static void start_task(void *ptr)
	{
		static_cast<task_base*>(ptr)->main();
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
