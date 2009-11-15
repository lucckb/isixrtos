#ifndef __ISIX_SCHEDULER_H__
#define __ISIX_SCHEDULER_H__

#include <isix/config.h>
#include <isix/types.h>
#include <isix/port_scheduler.h>


/*-----------------------------------------------------------------------*/
//Pointer to task function
typedef void (*task_func_ptr_t)(void*);

/*-----------------------------------------------------------------------*/
//Priority type
typedef uint8_t prio_t;
/*-----------------------------------------------------------------------*/

//Yield processor
#define isix_yield() port_yield()

/*-----------------------------------------------------------------------*/

//HZ value used as system ticks
#define HZ CONFIG_HZ

/*-----------------------------------------------------------------------*/

//Definition of task operations
struct task_struct;
typedef struct task_struct task_t;

/*-----------------------------------------------------------------------*/
//Halt system when critical error
void isix_bug(void);

/*-----------------------------------------------------------*/
//Get current jiffies
tick_t isix_get_jiffies(void);


/*-----------------------------------------------------------------------*/
//Start scheduler
void isix_start_scheduler(void) __attribute__((noreturn));

/*-----------------------------------------------------------------------*/

/* Initialize base OS structure before call main */
void isix_init(prio_t num_priorities);

/*-----------------------------------------------------------------------*/
//Get maxium available priority
prio_t isix_get_min_priority(void);

/*-----------------------------------------------------------------------*/
#endif
