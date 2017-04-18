#pragma once
#include <isix/config.h>
#include <isix/types.h>
#include <isix/arch/scheduler.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//! Definition of task operations
struct isix_task;
typedef struct isix_task* ostask_t;
//! Pointer to task function
typedef void (*task_func_ptr_t)(void*);
//! Fatal panic function callback
typedef void (*isix_panic_func_callback_t)(const char*, int, const char*);

//! Yield the current process
static inline void isix_yield()
{
	extern volatile bool _isix_scheduler_running;
	if(_isix_scheduler_running)
		_isix_port_yield();
}

//! Halt system when critical error is found
void isix_kernel_panic( const char *file, int line, const char *msg );

//ISIX BUG macro
#define isix_bug( msg ) isix_kernel_panic(__FILE__,__LINE__, (msg) )

/** Get current sytem ticks
 * @return Number of system tick from system startup
 */
ostick_t isix_get_jiffies(void);

//!Start the scheduler
#if CONFIG_ISIX_SHUTDOWN_API
void isix_start_scheduler(void);
#else
void isix_start_scheduler(void) __attribute__((noreturn));
#endif

#if CONFIG_ISIX_SHUTDOWN_API
/**
 * Shutdown scheduler and return to main
 * @note It can be called only a once just before
 * the system shutdown for battery power save
 */
void isix_shutdown_scheduler(void);
#endif


/** Reboot the target system */
static inline __attribute__((noreturn,always_inline))
void isix_reboot(void)
{
	_isix_port_system_reset();
}


/** Initialize base OS structure before call main
 * @param[in] core_freq  AHB core frequency
 */
void isix_init( unsigned long core_freq );

/** Function return the minimal available priority
 * @return Number of minimal available priority
 */
osprio_t isix_get_min_priority(void);

/** Functtion return scheduling state
 @return True if scheduler is running
 */
static inline bool isix_is_scheduler_active(void)
{
	extern volatile bool _isix_scheduler_running;
	return _isix_scheduler_running;
}


//! Lock scheduler and disable selected interrupt
void isix_enter_critical(void);

//! Lock scheduler and reenable selected interrupt
void isix_exit_critical(void);

#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */


#ifdef __cplusplus
namespace isix {
namespace {
	inline void yield() {
		::isix_yield();
	}
	inline ostick_t get_jiffies() {
		return ::isix_get_jiffies();
	}
	inline void start_scheduler() {
		isix_start_scheduler();
	}
#if CONFIG_ISIX_SHUTDOWN_API
	inline void shutdown_scheduler() {
		::isix_shutdown_scheduler();
	}
#endif
	inline void init( unsigned long core_freq ) {
		::isix_init( core_freq );
	}
	inline osprio_t get_min_priority() {
		return ::isix_get_min_priority();
	}
	inline bool is_scheduler_active() {
		return ::isix_is_scheduler_active();
	}
	inline void enter_critical(void) {
		return ::isix_enter_critical();
	}
	inline void exit_critical(void) {
		return ::isix_exit_critical();
	}

}}
#endif /* __cplusplus */
