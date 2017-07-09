/*
 * =====================================================================================
 *
 *       Filename:  virtual_timer.hpp
 *
 *    Description:  Virtual timer reimpl
 *
 *        Version:  1.0
 *        Created:  02.04.2015 22:58:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#if CONFIG_ISIX_USE_TIMERS
#ifdef __cplusplus


#include <cstddef>
#include <functional>
#include <isix/types.h>
#include <isix/ostime.h>
#include <isix/softtimers.h>
namespace isix {

	//! C++ wrapper for the vtimer
	class virtual_timer {
		public:
			//! Create virtual timer object
			virtual_timer() {
				timer = vtimer_create( );
			}
			//! Destroy the virtual timer object
			virtual ~virtual_timer() {
				vtimer_destroy( timer );
			}
			virtual_timer(virtual_timer&) = delete;
			virtual_timer(virtual_timer&&) = default;
			virtual_timer& operator=(virtual_timer&) = delete;
			virtual_timer& operator=(virtual_timer&&) = delete;
			//! Check that object is valid
			bool is_valid() const noexcept {
				return timer!=0;
			}
			//! Start the timer on selected period
			int start( ostick_t timeout, bool cyclic=true ) noexcept {
				return vtimer_start( timer, callback, this, timeout, cyclic );
			}
			//! Start the timer on selected period
			int start_ms( ostick_t timeout, bool cyclic=true ) noexcept {
				return vtimer_start( timer, callback, this, ms2tick(timeout), cyclic );
			}
			//! Stop the timer
			int stop() noexcept {
				return isix_vtimer_cancel( timer );
			}
			//! Start the timer on selected period
			int start_isr( ostick_t timeout, bool cyclic=true ) noexcept {
				return vtimer_start_isr( timer, callback, this, timeout, cyclic );
			}
			//! Start the timer on selected period
			int start_ms_isr( ostick_t timeout, bool cyclic=true ) noexcept {
				return vtimer_start_isr( timer, callback, this, ms2tick(timeout), cyclic );
			}
			//! Stop the timer
			int stop_isr() noexcept {
				return isix_vtimer_cancel_isr( timer );
			}

		protected:
			//! Virtual function called on time
			virtual void handle_timer() noexcept = 0;
		private:
			static void callback(void *ptr) {
				static_cast<virtual_timer*>(ptr)->handle_timer();
			}
		private:
			//Noncopyable
			virtual_timer(const virtual_timer&);
			virtual_timer& operator=(const virtual_timer&);
		private:
			osvtimer_t timer;
	};


	//! C++11/14 virtual timer wrapper
	class soft_timer final : public virtual_timer {
	public:
		/** @brief thread constructor
			*  @param[in] fn Function executed in separate thread
			*  @param[in] args Arguments passed to the function
			*/
		template <typename FN, typename ... ARGS>
			soft_timer( FN&& function, ARGS&&... args ) noexcept
			: m_bound_fn( std::bind(std::forward<FN>(function), std::forward<ARGS>(args)... ) )
			{
			}
		soft_timer( soft_timer& ) = delete;
		soft_timer( soft_timer&& ) = default;
		soft_timer& operator=(soft_timer&) = delete;
		soft_timer& operator=(soft_timer&&) = delete;
		//! Destructor
		virtual ~soft_timer() {

		}
	private:
		void handle_timer() noexcept override {
			m_bound_fn();
		}
		std::function<void()> m_bound_fn;
	};

	/** Helper factory function for virtual timer creation */
	template <typename FN, typename ... ARGS>
		soft_timer vtimer_create( FN&& fn, ARGS&&... args ) noexcept
	{
		return { std::forward<FN>(fn), std::forward<ARGS>(args)... };
	}

}

#endif /*  __cplusplus */
#endif /* CONFIG_ISIX_USE_TIMERS */
