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
#ifdef __cplusplus


#include <cstddef>
#include <cstdlib>
#include <functional>
#include <isix/types.h>
#include <isix/ostime.h>
#include <isix/softtimers.h>
#include <isix/semaphore.h>
namespace isix {

	//! C++ wrapper for the vtimer
	class virtual_timer {
		public:
			//! Create virtual timer object
			virtual_timer() {
				timer = vtimer_create( );
			}
			/** Destroy the virtual timer object
			 *  Destruction is asynchronous. A class derived from virtual_timer must call
			 *  stop_sync() in its own destructor, otherwise the callback may reach the
			 *  already destroyed derived part. It must not be destroyed from the timer callback.
			 */
			virtual ~virtual_timer() {
				if( timer ) {
					vtimer_destroy( timer );
				}
			}
			virtual_timer(virtual_timer&) = delete;
			/** Move is possible only for a timer which is not active, the callback keeps the object address */
			virtual_timer(virtual_timer&& other) noexcept
				: timer( other.timer )
			{
				if( timer && ::isix_vtimer_is_active( timer ) > 0 ) {
					std::abort();
				}
				other.timer = nullptr;
			}
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
			/** Cancel the timer and wait until the worker thread stops using this object
			 *  Call it from the destructor of the most derived class. It cannot be called
			 *  from the timer callback.
			 */
			void stop_sync() noexcept {
				if( !timer ) {
					return;
				}
				isix_vtimer_cancel( timer );
				// The worker handles the commands in order, so after the fence timer
				// fired the cancel is done and no callback of this timer is running
				const auto sem = isix_sem_create_limited( nullptr, 0, 1 );
				const auto fence = isix_vtimer_create();
				if( sem && fence &&
					isix_vtimer_start( fence, fence_callback, sem, 1, false ) == ISIX_EOK ) {
					isix_sem_wait( sem, ISIX_TIME_INFINITE );
				}
				if( fence ) {
					isix_vtimer_destroy( fence );
				}
				if( sem ) {
					isix_sem_destroy( sem );
				}
			}
			//! Virtual function called on time
			virtual void handle_timer() noexcept = 0;
		private:
			static void fence_callback(void *ptr) {
				isix_sem_signal( static_cast<ossem_t>(ptr) );
			}
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
			stop_sync();
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
