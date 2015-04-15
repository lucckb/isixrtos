/*
 * irq_timers.h
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */

#pragma once
#include <isix/config.h>
#ifdef ISIX_CONFIG_USE_TIMERS
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


/** Create virtual timer  object
 * @return Virtual timer or null ptr if cannot be created
 */
osvtimer_t isix_vtimer_create( void );

/**
 * Start one shoot timer execution
 * @param timer	Pointer to the timer structure
 * @param fun Function to be called 
 * @param arg Argument passed to the function
 * @param timeout Timeout
 * @param[in] Cyclic is cyclic timer
 * @return success if ISIX_EOK else isix error
 */
int isix_vtimer_start( osvtimer_t timer, osvtimer_callback func, 
		void* arg, ostick_t timeout, bool cyclic );

/** Stop the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_cancel( osvtimer_t timer );


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
 * @param[in[ arg Functon argument
 */
int isix_schedule_work_isr( osworkfunc_t func, void* arg );



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
	inline int vtimer_cancel( osvtimer_t timer ) {
		return ::isix_vtimer_cancel( timer );
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
}}
#endif /* __cplusplus */

#endif /*ISIX_CONFIG_USE_TIMERS */
