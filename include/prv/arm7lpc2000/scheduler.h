/* Scheduler CPU specific data */

#ifndef __ASM_PRV_SCHEDULER_H
#define __ASM_PRV_SCHEDULER_H

#include <asm/scheduler.h>

/*-----------------------------------------------------------------------*/
//Process base stack initialization
reg_t* task_init_stack(reg_t *sp,task_func_ptr_t pfun,void *param);

/*-----------------------------------------------------------------------*/
/* Initialize system timer */
void sys_time_init(void);

/*-----------------------------------------------------------------------*/
#endif

