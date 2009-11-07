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
typedef uint8_t prio_t;
/*-----------------------------------------------------------------------*/

//Yield processor
#define sched_yield() port_yield()

/*-----------------------------------------------------------------------*/

//HZ value used as system ticks
#define HZ CONFIG_HZ

/*-----------------------------------------------------------------------*/

//Definition of task operations
struct task_struct;
typedef struct task_struct task_t;

/*-----------------------------------------------------------------------*/
//Halt system when critical error
void bug(void);

/*-----------------------------------------------------------------------*/
//Start scheduler
void start_scheduler(void) __attribute__((noreturn));

/*-----------------------------------------------------------------------*/
#endif
