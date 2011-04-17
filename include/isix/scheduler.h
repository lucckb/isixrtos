#ifndef _ISIX_SCHEDULER_H
#define _ISIX_SCHEDULER_H
/*-----------------------------------------------------------------------*/
#include <isix/config.h>
#include <isix/types.h>
#include <isix/port_scheduler.h>
/*-----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
//! Pointer to task function
typedef void (*task_func_ptr_t)(void*);

/*-----------------------------------------------------------------------*/
//! Priority type
typedef uint8_t prio_t;
/*-----------------------------------------------------------------------*/

//! Yield the current process
static inline void isix_yield() { port_yield(); }

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
static const unsigned ISIX_HZ = ISIX_CONFIG_HZ;
#else
//! HZ value used as system ticks
#define ISIX_HZ ISIX_CONFIG_HZ
#endif

/*-----------------------------------------------------------------------*/

//! Definition of task operations
struct task_struct;
typedef struct task_struct task_t;

/*-----------------------------------------------------------------------*/
//! Halt system when critical error is found
void isix_bug(void);

/*-----------------------------------------------------------*/
/** Get current sytem ticks
 * @return Number of system tick from system startup
 */
tick_t isix_get_jiffies(void);


/*-----------------------------------------------------------------------*/
//!Start the scheduler
void isix_start_scheduler(void) __attribute__((noreturn));

/*-----------------------------------------------------------------------*/

/** Initialize base OS structure before call main
 * @param[in] num_priorities Number of available tasks priorities*/
void isix_init(prio_t num_priorities);

/*-----------------------------------------------------------------------*/
/** Function return the minimal available priority
 * @return Number of minimal available priority
 */
prio_t isix_get_min_priority(void);
/*-----------------------------------------------------------------------*/
/** Functtion return scheduling state 
 @return True if scheduler is running
 */
bool isix_is_scheduler_active(void);

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */
/*-----------------------------------------------------------------------*/
#endif
