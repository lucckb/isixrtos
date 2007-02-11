#ifndef __ISIX_TASK_H
#define __ISIX_TASK_H

#include <isix/types.h>
#include <isix/scheduler.h>

/*-----------------------------------------------------------------------*/
//Definition of task function
#define TASK_FUNC(FUNC, ARG)							\
	void FUNC(void *ARG) __attribute__ ((noreturn));	\
	void FUNC(void *ARG)

/*-----------------------------------------------------------------------*/
//Pointer to task function
typedef void (*task_func_ptr_t)(void*);

/*-----------------------------------------------------------------------*/
//Priority type
typedef u8 prio_t;

/*-----------------------------------------------------------------------*/
/* Create task function */
//task_t* task_create(task_func_ptr_t task_func, void *func_param, reg_t stack_depth, prio_t priority);
struct task_struct* task_create(task_func_ptr_t task_func, void *func_param, reg_t stack_depth, prio_t priority);
/*-----------------------------------------------------------------------*/

#endif

