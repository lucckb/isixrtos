#ifndef __ISIX_SCHEDULER_H__
#define __ISIX_SCHEDULER_H__

#include <isix/config.h>
#include <isix/types.h>
#include <asm/scheduler.h>

/*-----------------------------------------------------------------------*/
//Define idle priority value
#define SCHED_IDLE_PRIORITY 255

/*-----------------------------------------------------------------------*/
//Pointer to task function
typedef void (*task_func_ptr_t)(void*);

/*-----------------------------------------------------------------------*/
//Priority type
typedef u8 prio_t;
/*-----------------------------------------------------------------------*/

//Yield processor
#define sched_yield() cpu_yield()

/*-----------------------------------------------------------------------*/

//HZ value used as system ticks
#define HZ CONFIG_HZ

/*-----------------------------------------------------------------------*/

//Definition of task operations
struct task_struct;
typedef struct task_struct task_t;

/*-----------------------------------------------------------------------*/
#endif
