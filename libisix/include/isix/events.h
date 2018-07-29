/*
 * =====================================================================================
 *
 *       Filename:  events.h
 *
 *    Description:  Event object wait
 *
 *        Version:  1.0
 *        Created:  07.04.2015 11:42:58
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once

#include <isix/config.h>
#include <isix/types.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/
struct isix_event;
typedef struct isix_event* osevent_t;

/** Create event group
 * @return event group handle or null
 */
osevent_t isix_event_create( void );

/** Delete the event group
 *  @param[in] Osevent input group
 * */
int isix_event_destroy( osevent_t event );

/** Atomically set bits (flags) * within an event group,
 *  then wait for a combination of bits to be set within
 *  the same event group
 * @param[in]	evth Event handle
 * @param[in]	bits_to_set Bits to set
 * @param[in]	bits_to_wait Bits to wait for
 * @param[in]	timeout Timeout to wait for sync
 * @return Changed bits or error if negative
 */
osbitset_ret_t isix_event_sync( osevent_t evth, osbitset_t bits_to_set,
		osbitset_t bits_to_wait, ostick_t timeout );

/** Wait for event bit set
 * @param[in]	evth Event handle
 * @param[in]	bits_to_wait Bits to wait for
 * @param[in]	clear_on_exit Clear bits on exit
 * @param[in]	wait_for_all  Wait for all bits
 * @param[in]   timeout		 Timeout to wait for sync
 * @return Bits which are set
 */
osbitset_ret_t isix_event_wait( osevent_t evth, osbitset_t bits_to_wait, 
		bool clear_on_exit, bool wait_for_all, ostick_t timeout );

/** Clear the selected bits from the event
 * @parma[in]	evth Event handle
 * @param[in]	bits_to_clear
 * @return Changed bits
 */
osbitset_ret_t isix_event_clear( osevent_t evth, osbitset_t bits_to_clear );

static inline osbitset_t isix_event_clear_isr( osevent_t evth, osbitset_t bits_to_clear ) 
{
	return isix_event_clear( evth, bits_to_clear );
}

/** Isix set bits
 * @parma[in]	evth Event handle
 * @param[in]	bits_to_clear
 * @return Changed bits
 */
osbitset_ret_t _isixp_event_set( osevent_t evth, osbitset_t bits_to_set, bool isr );
static inline osbitset_t isix_event_set( osevent_t evth, osbitset_t bits_to_set ) {
	return _isixp_event_set( evth, bits_to_set, false );
}
static inline osbitset_t isix_event_set_isr( osevent_t evth, osbitset_t bits_to_set ) {
	return _isixp_event_set( evth, bits_to_set, true );
}

/** Get the events from the interrupt context
 * @param[in] evth Event handle
 * @return Bit state
 */
osbitset_ret_t isix_event_get_isr( osevent_t evth );

static inline osbitset_ret_t isix_event_get( osevent_t evth ) {
	return isix_event_clear( evth, 0 );
}

#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */


#ifdef __cplusplus
namespace isix {
namespace {
	using event_t = osevent_t;
	using bitset_ret_t = osbitset_ret_t;

	inline osevent_t event_create( ) {
		return ::isix_event_create();
	}
	inline int event_destroy( osevent_t event ) {
		return ::isix_event_destroy( event );
	}

	inline osbitset_ret_t event_sync( osevent_t evth, osbitset_t bits_to_set, 
			osbitset_t bits_to_wait, ostick_t timeout=ISIX_TIME_INFINITE ) {
		return ::isix_event_sync( evth, bits_to_set, bits_to_wait, timeout );
	}

	inline osbitset_ret_t event_wait( osevent_t evth, osbitset_t bits_to_wait, 
			bool clear_on_exit, bool wait_for_all, ostick_t timeout=ISIX_TIME_INFINITE ) {
		return ::isix_event_wait( evth, bits_to_wait, clear_on_exit, wait_for_all, timeout );
	}

	inline osbitset_ret_t event_clear( osevent_t evth, osbitset_t bits_to_clear ) {
		return ::isix_event_clear( evth, bits_to_clear );
	}

	inline osbitset_t event_clear_isr( osevent_t evth, osbitset_t bits_to_clear ) {
		return ::isix_event_clear_isr( evth, bits_to_clear );
	}

	inline osbitset_t event_set( osevent_t evth, osbitset_t bits_to_set ) {
		return ::isix_event_set( evth, bits_to_set );
	}

	inline osbitset_t event_set_isr( osevent_t evth, osbitset_t bits_to_set ) {
		return ::isix_event_set_isr( evth, bits_to_set );
	}

	inline osbitset_ret_t event_get_isr( osevent_t evth ) {
		return ::isix_event_get_isr( evth );
	}
	inline osbitset_ret_t event_get( osevent_t evth ) {
		return ::isix_event_get( evth );
	}

}}
#endif
