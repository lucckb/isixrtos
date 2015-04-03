/*
 * =====================================================================================
 *
 *       Filename:  semaphore.hpp
 *
 *    Description:  Semaphore C++ wrapper class for isix
 *
 *        Version:  1.0
 *        Created:  02.04.2015 23:06:18
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

namespace isix 
{

	//! Semaphore C++ class wrapper
	class semaphore
	{
	public:
		/** Construct semaphore object
		* @param[in] val Initial value of the semaphore
		*/
		explicit semaphore( int val, int limit_val=ISIX_SEM_ULIMITED )
		{
			sem = isix_sem_create_limited( NULL, val, limit_val );
		}
		//! Destruct semaphore object
		~semaphore()
		{
			isix_sem_destroy(sem);
		}
		/** Check the fifo object is in valid state
		* @return true if object is in valid state otherwise return false
		*/
		bool is_valid() { return sem!=0; }
		/** Wait for the semaphore for selected time
		* @param[in] timeout Max waiting time
		*/
		int wait(tick_t timeout)
		{
			return isix_sem_wait( sem, timeout );
		}
		/** Get the semaphore from the ISR context
		* @return ISIX_EOK if the operation is completed successfully otherwise return an error code
		*/
		int get_isr()
		{
			return isix_sem_get_isr(sem);
		}
		/** Signaling the semaphore
		* @return ISIX_EOK if the operation is completed successfully otherwise return an error code
		*/
		int signal()
		{
			return isix_sem_signal(sem);
		}
		/** Signal the semaphore from the ISR context
		* @return ISIX_EOK if the operation is completed successfully otherwise return an error code
		*/
		int signal_isr()
		{
			return isix_sem_signal_isr(sem);
		}
		/** Set value of the semaphore
		* @param[in] val Value of the semaphore
		* @return ISIX_EOK if the operation is completed successfully otherwise return an error code
		*/
		int setval(int val)
		{
			return isix_sem_setval( sem, val );
		}
		/** Get the semaphore value
		* @return the semaphore value otherwise an error
		*/
		int getval()
		{
			return isix_sem_getval( sem );
		}
	private:
		semaphore(const semaphore&);
		semaphore& operator=(const semaphore&);
	private:
		ossem_t sem;
	};
	
	//! Semaphore locker class
	class sem_lock {
	public:
		sem_lock( semaphore& _sem )
			: m_sem( _sem ) {
			_sem.wait( ISIX_TIME_INFINITE );
		}
		~sem_lock() {
			m_sem.signal();
		}
	private:
		sem_lock(const sem_lock&);
		sem_lock& operator=(const sem_lock&);
		semaphore& m_sem;
	};
}

#endif /* __cplusplus */

