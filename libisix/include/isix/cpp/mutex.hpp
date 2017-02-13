#pragma once

#ifdef __cplusplus
#include <cstddef>
namespace isix {
	//! Mutex C++ class wrapper
	class mutex
	{
	public:
		/** Construct mutex object
		 */
		mutex()
			: m_mtx( isix_mutex_create(nullptr) )
		{ }
		//! Private assign/copy
		mutex( const mutex& ) = delete;
		mutex& operator=( const mutex& ) = delete;
		//! Destruct mutex object
		~mutex() {
			isix_mutex_destroy( m_mtx );
		}
		bool is_valid() const noexcept { return m_mtx!=nullptr; }
		/** Wait for the semaphore for selected time
		* @param[in] timeout Max waiting time
		*/

		/** Lock the mutex
		 * @return ISIX_EOK if the operation is completed succesfully */
		int lock() noexcept {
			return isix_mutex_lock( m_mtx );
		}

		/** Unlock the mutex
		 * @return ISIX_EOK if the operation is completed succesfully */
		int unlock() noexcept {
			return isix_mutex_unlock( m_mtx );
		}

		/** Check if mutex can be locked without task block
		 * @return ISIX_EOK if the operation is completed succesfully */
		int try_lock() noexcept {
			return isix_mutex_trylock( m_mtx );
		}

	private:
		osmtx_t m_mtx;
	};

	//! Semaphore locker class
	class mutex_locker {
	public:
		mutex_locker(const mutex_locker&) = delete;
		mutex_locker& operator=(const mutex_locker&) = delete;
		mutex_locker( mutex& _mtx )
			: m_mtx( _mtx ) {
			_mtx.lock();
		}
		~mutex_locker() {
			m_mtx.unlock();
		}
	private:
		mutex& m_mtx;
	};
}
#endif
