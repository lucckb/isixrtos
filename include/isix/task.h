#ifndef __ISIX_TASK_H
#define __ISIX_TASK_H

#include <isix/types.h>
#include <isix/scheduler.h>

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

#define isix_task_change_prio(task,new_prio) isixp_task_change_prio(task,new_prio,true)
/*-----------------------------------------------------------------------*/

//Delete task pointed by struct task
int isix_task_delete(task_t *task);

/*-----------------------------------------------------------------------*/

//Get current thread handler
task_t* isix_task_self(void);

/*-----------------------------------------------------------------------*/
//Stack check for fill value
#if CONFIG_TASK_STACK_CHECK == ISIX_ON
size_t isix_free_stack_space(const task_t *task);
#endif
/*-----------------------------------------------------------------------*/
#endif

