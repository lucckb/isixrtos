/*
 * irq_timers.h
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */

#pragma once
#include <isix/config.h>
#if CONFIG_ISIX_USE_TIMERS
#include <isix/types.h>
#include <isix/ostime.h>
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/* Structure definition for hidding the type */
struct isix_vtimer;
typedef struct isix_vtimer* osvtimer_t;

/** Callback functions  */
typedef void (*osvtimer_callback)(void*);
typedef osvtimer_callback osworkfunc_t;


/** Create global isixvtimer worker thread
 *  It must be called only when shedule work API is used
 *  @return ISIX_EOK if success
 */
int isix_vtimer_initialize( void );

/** Create virtual timer  object
 * @return Virtual timer or null ptr if cannot be created
 */
osvtimer_t isix_vtimer_create( void );

/**
 * Start one shoot timer execution ISR and non ISR ver
 * @param timer	Pointer to the timer structure
 * @param fun Function to be called
 * @param arg Argument passed to the function
 * @param timeout Timeout
 * @param[in] Cyclic is cyclic timer
 * @return success if ISIX_EOK else isix error
 */
int _isixp_vtimer_start( osvtimer_t timer, osvtimer_callback func,
		void* arg, ostick_t timeout, bool cyclic, bool isr );
static inline __attribute__((always_inline))
int isix_vtimer_start( osvtimer_t timer, osvtimer_callback func,
		void* arg, ostick_t timeout, bool cyclic )
{
	return _isixp_vtimer_start( timer, func, arg, timeout, cyclic, false );
}
static inline __attribute__((always_inline))
int isix_vtimer_start_isr( osvtimer_t timer, osvtimer_callback func,
		void* arg, ostick_t timeout, bool cyclic )
{
	return _isixp_vtimer_start( timer, func, arg, timeout, cyclic, true );
}

/** Stop the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int _isixp_vtimer_cancel( osvtimer_t timer, bool isr );
static inline __attribute__((always_inline))
int isix_vtimer_cancel( osvtimer_t timer )
{
	return _isixp_vtimer_cancel( timer, false );
}

static inline __attribute__((always_inline))
int isix_vtimer_cancel_isr( osvtimer_t timer )
{
	return _isixp_vtimer_cancel( timer, true );
}

/** Check if timer is still active and it waits for an event
 * @param[in] timer TImer handler
 * @return Timer state true if active false if not or error
 * code if returned value is negative */
int isix_vtimer_is_active( osvtimer_t timer );



/** Destroy the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_destroy( osvtimer_t timer );


/** Only one function schedule task from an interrupt or
 * other task context for delayed execution
 * @param[in] func Function to call
 * @param[in] arg Functon argument
 */
int isix_schedule_work_isr( osworkfunc_t func, void* arg );


/** Virtual timer next cancel from callback */
enum { OSVTIMER_CB_CANCEL = 0 };
/** Function modify the next timer timeout
 * it should be called only from from the timer
 * callback not from normal context function
 * @param[in] timer to modify
 * @param[in] New timeout or cancel
 */
int isix_vtimer_mod( osvtimer_t timer, ostick_t new_timeout );


#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */

#ifdef __cplusplus
namespace isix {
namespace {
	using vtimer_t = osvtimer_t;
	inline osvtimer_t vtimer_create() {
		return ::isix_vtimer_create();
	}
	inline int vtimer_start( osvtimer_t timer, osvtimer_callback func,
			void* arg, ostick_t timeout, bool cyclic ) {
		return ::isix_vtimer_start( timer, func, arg, timeout, cyclic );
	}
	inline int vtimer_start_isr( osvtimer_t timer, osvtimer_callback func,
			void* arg, ostick_t timeout, bool cyclic ) {
		return ::isix_vtimer_start_isr( timer, func, arg, timeout, cyclic );
	}
	inline int vtimer_cancel( osvtimer_t timer ) {
		return ::isix_vtimer_cancel( timer );
	}
	inline int vtimer_cancel_isr( osvtimer_t timer ) {
		return ::isix_vtimer_cancel_isr( timer );
	}
	inline int vtimer_is_active( osvtimer_t timer ) {
		return ::isix_vtimer_is_active( timer );
	}
	inline int vtimer_destroy( osvtimer_t timer ) {
		return ::isix_vtimer_destroy( timer );
	}
	inline int schedule_work_isr( osworkfunc_t func, void* arg ) {
		return ::isix_schedule_work_isr( func, arg );
	}
	inline int vtimer_mod( osvtimer_t timer, ostick_t new_timeout ) {
		return ::isix_vtimer_mod( timer, new_timeout );
	}
}}
#endif /* __cplusplus */

#endif /*CONFIG_ISIX_USE_TIMERS */
