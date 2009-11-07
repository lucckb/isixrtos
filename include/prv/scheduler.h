/* Scheduler CPU specific data */

#ifndef __ASM_PRV_SCHEDULER_H
#define __ASM_PRV_SCHEDULER_H

#include "scheduler2.h"

/*-----------------------------------------------------------------------*/
//Process base stack initialization
unsigned long* task_init_stack(unsigned long *sp,task_func_ptr_t pfun,void *param);

/*-----------------------------------------------------------------------*/
#endif

