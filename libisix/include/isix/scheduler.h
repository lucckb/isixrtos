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
//! Fatal panic function callback
typedef void (*isix_panic_func_callback_t)(const char*, int, const char*);

/*-----------------------------------------------------------------------*/
//! Priority type
typedef uint8_t prio_t;
/*-----------------------------------------------------------------------*/

//! Yield the current process
static inline void isix_yield()
{
	extern volatile bool isix_scheduler_running;
	if(isix_scheduler_running)
		port_yield();
}

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
void isix_kernel_panic( const char *file, int line, const char *msg );

/*-----------------------------------------------------------*/
//ISIX BUG macro
#define isix_bug( msg ) isix_kernel_panic(__FILE__,__LINE__, (msg) )

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
 * @param[in] num_priorities Number of available tasks priorities
 */
void isix_init( prio_t num_priorities );

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
/** Get current sytem ticks
 * @return Number of system tick from system startup in usec resolution
 */
static inline utick_t isix_get_ujiffies(void)
{
	utick_t t = (utick_t)isix_get_jiffies() * ((utick_t)1000000/((utick_t)ISIX_HZ));
    t += (((utick_t)port_get_hres_jiffies_timer_value()) * ((utick_t)1000000/((utick_t)ISIX_HZ)))
    	/  (utick_t)port_get_hres_jiffies_timer_max_value();
    return t;
}
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */
/*-----------------------------------------------------------------------*/
#endif
