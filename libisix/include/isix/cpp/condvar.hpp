#pragma once

#ifdef __cplusplus
#include <cstddef>
namespace isix {


	class condvar
	{
	public:
		/** Construct condvar object */
		condvar() {
			m_cv = isix_condvar_create();
		}
		/** Destruct condvar object  */
		~condvar() {
			isix_condvar_destroy( m_cv );
		}
		//! Private assign/copy
		condvar( const condvar& ) = delete;
		condvar& operator=( const condvar& ) = delete;
		/** Check the fifo object is in valid state
		* @return true if object is in valid state otherwise return false
		*/
		bool is_valid() const noexcept { return m_cv!=0; }

		int signal() noexcept {
			return isix_condvar_signal( m_cv );
		}
		int signal_isr() noexcept {
			return isix_condvar_signal_isr( m_cv );
		}
		int broadcast() noexcept {
			return isix_condvar_broadcast( m_cv );
		}
		int broadcast_isr() noexcept {
			return isix_condvar_broadcast_isr( m_cv );
		}
		int wait( ostick_t timeout = ISIX_TIME_INFINITE ) noexcept {
			return isix_condvar_wait( m_cv, timeout );
		}
	private:
		oscondvar_t m_cv;
	};

}

#endif
