/*
 * =====================================================================================
 *
 *       Filename:  events.hpp
 *
 *    Description:  Events implementation CPP wrapper
 *
 *        Version:  1.0
 *        Created:  09.04.2015 17:43:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *       Author:  Lucjan Bryndza (LB), lucck
 *
 * =====================================================================================
 */

#pragma once

#ifdef __cplusplus
namespace isix {

	//! Event class implementation
	class event
	{
	public:
		/** Construct base event object */
		event()
			: ev( ::isix_event_create() )
		{}
		/** Destroy the event object */
		~event()
		{
			::isix_event_destroy( ev );
		}
		//! Noncopyable
		event( const event& ) = delete;
		event& operator=( const event& ) = delete;
		/** Check if the object is in valid state
		 * @return true if object is properly initialized
		 */
		bool is_valid() const {
			return ev!=nullptr;
		}
		/** Get native handle
		 * @return Native handle
		 */
		osevent_t get() const {
			return ev;
		}
		/** Wait for event
		 * @param[in] bits_to_set Bits to set in event
		 * @param[in] bits_to_wait Bits to wait for
		 * @param[in] timeout Isix timeout
		 * @return Changed bitset or error code
		 */
		osbitset_ret_t sync( osbitset_t bits_to_set, osbitset_t bits_to_wait,
				ostick_t timeout = ISIX_TIME_INFINITE )
		{
			return ::isix_event_sync( ev, bits_to_set, bits_to_wait, timeout );
		}
		/** Wait for event bit set
		* @param[in]	bits_to_wait Bits to wait for
		* @param[in]	clear_on_exit Clear bits on exit
		* @param[in]	wait_for_all  Wait for all bits
		* @param[in]   timeout		 Timeout to wait for sync
		* @return Bits which are set
		*/
		osbitset_ret_t wait( osbitset_t bits_to_wait, bool clear_on_exit,
			bool wait_for_all, ostick_t timeout = ISIX_TIME_INFINITE )
		{
			return ::isix_event_wait( ev, bits_to_wait, clear_on_exit, wait_for_all, timeout );
		}
		/** Clear the selected bits from the event
		* @param[in]	bits_to_clear
		* @return Changed bits
		*/
		osbitset_ret_t clear( osbitset_t bits_to_clear )
		{
			return ::isix_event_clear( ev, bits_to_clear );
		}
		osbitset_t clear_isr( osbitset_t bits_to_clear )
		{
			return ::isix_event_clear_isr( ev, bits_to_clear );
		}
		/** Isix set bits
		* @param[in]	bits_to_clear
		* @return Changed bits
		*/
		osbitset_t set( osbitset_t bits_to_set )
		{
			return ::isix_event_set( ev, bits_to_set );
		}
		osbitset_t set_isr( osbitset_t bits_to_set )
		{
			return ::isix_event_set_isr( ev, bits_to_set );
		}
		/** Get the events from the interrupt context
		* @return Bit state
		*/
		osbitset_ret_t get_isr( )
		{
			return ::isix_event_get_isr( ev );
		}
		osbitset_ret_t get( )
		{
			return ::isix_event_get( ev );
		}
	private:
		osevent_t ev;
	};
}

#endif
