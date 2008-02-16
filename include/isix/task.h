#ifndef __ISIX_TASK_H
#define __ISIX_TASK_H

#include <isix/types.h>
#include <isix/scheduler.h>


#define TASK_CURRENT NULL

/*-----------------------------------------------------------------------*/
//Definition of task function
#define TASK_FUNC(FUNC, ARG)							\
	void FUNC(void *ARG) __attribute__ ((noreturn));	\
	void FUNC(void *ARG)


/*-----------------------------------------------------------------------*/
/* Create task function */
task_t* task_create(task_func_ptr_t task_func, void *func_param, reg_t stack_depth, prio_t priority);

/*-----------------------------------------------------------------------*/
/*Change task priority function
 * task - task pointer structure if NULL current prio change
 * new_prio - new priority                                  */
int __task_change_prio(task_t *task,prio_t new_prio,bool yield);

#define task_change_prio(task,new_prio) __task_change_prio(task,new_prio,true)
/*-----------------------------------------------------------------------*/

//Delete task pointed by struct task
int task_delete(task_t *task);

/*-----------------------------------------------------------------------*/



#endif

