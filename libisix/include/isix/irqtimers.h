/*
 * irq_timers.h
 *
 *  Created on: 05-03-2011
 *      Author: lucck
 */
/*-----------------------------------------------------------------------*/
#pragma once

#include <isix/config.h>

/*-----------------------------------------------------------------------*/
#ifdef ISIX_CONFIG_USE_TIMERS
/*-----------------------------------------------------------------------*/
#include <isix/types.h>
#include <isix/semaphore.h>
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
/* Structure definition for hidding the type */
struct isix_vtimer;
typedef struct isix_vtimer* osvtimer_t;
typedef void (*osvtimer_callback)(void*);
/*-----------------------------------------------------------------------*/
//Private function for handling internal timer
osvtimer_t _isix_vtimer_create_internal_(osvtimer_callback func,void *arg, bool one_shoot );
/*-----------------------------------------------------------------------*/
/** Create virtual timer called from interrupt context for light events
 * consume much power cyclic tasks
 * @param[in] func Function to be called in context - cyclic timeout
 * @param[in] arg Argument passed to the function
 * @return NULL if timer create was unsucess, else return vtimer pointer
 */
static inline osvtimer_t isix_vtimer_create(osvtimer_callback func,void *arg ) {
	return _isix_vtimer_create_internal_( func, arg, false );
}
/*-----------------------------------------------------------------------*/
/**
 * Create virtual timer from interrupt context without init fn
 * @return
 */
static inline osvtimer_t isix_vtimer_create_oneshoot( void ) {
	return _isix_vtimer_create_internal_( NULL, NULL,true );
}
/*-----------------------------------------------------------------------*/
/**
 * Start one shoot timer execution
 * @param timer	Pointer to the timer structure
 * @param fun Function to be called in interrupt context
 * @param arg Argument passed to the function
 * @param timeout Timeout
 * @return success if ISIX_EOK else isix error
 */
int isix_vtimer_one_shoot( osvtimer_t timer, osvtimer_callback func, void *arg, ostick_t timeout );
/*-----------------------------------------------------------------------*/
/** Start the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @param[in] timeout Timeout for the next start
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_start(osvtimer_t timer, ostick_t timeout);
/*-----------------------------------------------------------------------*/
/** Stop the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
static inline int isix_vtimer_stop(osvtimer_t timer)
{
	return isix_vtimer_start( timer, 0 );
}
/*-----------------------------------------------------------------------*/
/** Destroy the vtimer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @return success if ISIX_EOK , otherwise error
 */
int isix_vtimer_destroy(osvtimer_t timer);
/*-----------------------------------------------------------------------*/
/** Start the timer on the selected period
 * @param[in] timer Pointer to the timer structure
 * @param[in] timeout in milisec Timeout for the next start
 * @return success if ISIX_EOK , otherwise error
 */
static inline int isix_vtimer_start_ms(osvtimer_t timer, ostick_t timeout)
{
	return isix_vtimer_start( timer, timeout>0?isix_ms2tick(timeout):0 );
}
/*-----------------------------------------------------------------------*/
/**
 * Start one shoot timer execution ms period
 * @param timer	Pointer to the timer structure
 * @param fun Function to be called in interrupt context
 * @param arg Argument passed to the function
 * @param timeout Timeout
 * @return success if ISIX_EOK else isix error
 */
static inline int isix_vtimer_one_shoot_ms( osvtimer_t timer, osvtimer_callback func, void *arg, ostick_t timeout ) 
{
	return isix_vtimer_one_shoot( timer, func, arg, timeout>0?isix_ms2tick(timeout):0 );
}
/* ----------------------------------------------------------------------- */
#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */



#endif /*ISIX_CONFIG_USE_TIMERS */
/*-----------------------------------------------------------------------*/
