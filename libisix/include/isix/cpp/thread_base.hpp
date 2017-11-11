/*
 * =====================================================================================
 *
 *       Filename:  thread_base.hpp
 *
 *    Description:  thread base
 *
 *        Version:  1.0
 *        Created:  02.07.2017 15:19:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <cstddef>
#include <isix/task.h>

namespace isix {
namespace detail {
	//! Thread class base
	class thread_base
	{
		public:
			//! Default constructor
			thread_base() {}
			//! Start thread
			void start_thread(std::size_t stack_depth, osprio_t priority, unsigned flags=0) noexcept
			{
				m_task = isix_task_create( construct_thread,
						this, stack_depth, priority, flags|isix_task_flag_ref
				);
			}
			//! Destructor
			virtual ~thread_base()
			{
				if( m_task ) {
					isix_task_kill(m_task);
					isix_task_unref(m_task);
					m_task = nullptr;
				}
			}
			//! Noncopyable
			thread_base(const thread_base&) = delete;
			thread_base(thread_base&&) = default;
			const thread_base& operator=(const thread_base&) = delete;
			thread_base& operator=(thread_base&&) = delete;
			//! Bool valid operator
			operator bool() const noexcept
			{
				return m_task!=nullptr;
			}
			//! Get task handler
			ostask_t tid() const noexcept
			{
				return m_task;
			}
			//! Kill the selected thread
			void kill() noexcept {
				if( m_task ) {
					isix::task_kill(m_task);
				}
			}
			//! Change priority
			int change_prio( osprio_t new_prio ) noexcept {
				return m_task?::isix_task_change_prio( m_task, new_prio ):int(ISIX_EINVARG);
			}
			//! Get isix task priority
			int get_prio() const noexcept {
				return (m_task)?(::isix_get_task_priority(m_task)):int(ISIX_EINVARG);
			}
			//! Get task inherited priority
			int get_inherited_prio() const noexcept {
				return m_task?::isix_get_task_inherited_priority( m_task ):int(ISIX_EINVARG);
			}
			//! Free stack space
			ssize_t free_stack_space() const noexcept {
				return m_task?::isix_free_stack_space( m_task ):int(ISIX_EINVARG);
			}
			//! Suspend
			int suspend() noexcept {
				if( m_task ) {
					::isix_task_suspend(m_task);
					return ISIX_EOK;
				} else {
					return ISIX_EINVARG;
				}
			}
			//! Resume
			int resume() const noexcept {
				return m_task?::isix_task_resume( m_task ):int(ISIX_EINVARG);
			}
			//! Get state
			int get_state() const noexcept {
				return m_task?::isix_get_task_state( m_task ):int(ISIX_EINVARG);
			}
			//! Wait for task
			int wait_for() const noexcept {
				return m_task?::isix_task_wait_for( m_task ):int(ISIX_EINVARG);
			}
		private:
			//! Thread runner
			virtual void runner() = 0;
			//! Static function for construct thread
			static void construct_thread(void *ptr) noexcept
			{
#			if __EXCEPTIONS
				try {
#			endif
				static_cast<thread_base*>(ptr)->runner();
#			if __EXCEPTIONS
				} catch(...) {
					// Unhandled exception
				}
#			endif
			}
		private:
			ostask_t m_task { nullptr };
			/* data */
	};
}
}
