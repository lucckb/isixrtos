/* Scheduler CPU specific data */

#ifndef __ASM_SCHEDULER_H
#define __ASM_SCHEDULER_H

#include <isix/task.h>

/*-----------------------------------------------------------------------*/
//Process base stack initialization
reg_t* task_init_stack(reg_t *sp,task_func_ptr_t pfun,void *param);

/*-----------------------------------------------------------------------*/
//Minimum stack depth
#define SCHED_MIN_STACK_DEPTH 80

/*-----------------------------------------------------------------------*/
#endif

