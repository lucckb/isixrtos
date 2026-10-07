/*
 * Coroutine adapter for a regular isix semaphore. Tokens signalled by threads,
 * ISRs or drivers are awaited in a coroutine of the owning scheduler. A task
 * blocked in isix_sem_wait() has priority over the adapter. Requires
 * CONFIG_ISIX_SEM_EVENT_NOTIFY.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus) && CONFIG_ISIX_SEM_EVENT_NOTIFY

#include <bit>
#include <coroutine>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/error.h>
#include <isix/ostime.h>
#include <isix/semaphore.h>

namespace isix::co {

class sem_adapter final {
public:
	sem_adapter(scheduler& sched, ossem_t sem) noexcept
		: m_sched(&sched), m_sem(sem), m_bit(sched.alloc_bit(&sem_adapter::dispatch_cb, this))
	{
		if (m_bit && m_sem) {
			const auto rc = isix_sem_event_connect(m_sem, sched.event_handle(), std::countr_zero(m_bit));
			m_connected = rc == ISIX_EOK;
		}
	}

	sem_adapter(const sem_adapter&) = delete;
	sem_adapter& operator=(const sem_adapter&) = delete;

	//! Waiters are woken with ISIX_EDESTROY
	~sem_adapter()
	{
		if (m_connected) {
			isix_sem_event_disconnect(m_sem, m_sched->event_handle());
		}
		while (auto* l = m_waiters.front()) {
			m_sched->wake(*detail::node_of(l), ISIX_EDESTROY);
		}
		m_sched->free_bit(m_bit);
	}

	[[nodiscard]] bool is_valid() const noexcept { return m_connected; }

	//! Take a token without waiting
	[[nodiscard]] bool try_take() noexcept
	{
		return m_connected && isix_sem_trywait(m_sem) == ISIX_EOK;
	}

	class take_awaiter final {
	public:
		take_awaiter(sem_adapter* sem, ostick_t timeout) noexcept : m_sem(sem), m_timeout(timeout) {}

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_sem || !m_sem->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_sem->m_waiters.empty() && m_sem->try_take()) {
				return true;
			}
			if (m_timeout == ISIX_TIME_DONTWAIT) {
				m_node.result = ISIX_ETIMEOUT;
				return true;
			}
			return false;
		}

		template<typename P>
		bool await_suspend(std::coroutine_handle<P> h) noexcept
		{
			auto* const sched = h.promise().sched;
			if (sched != m_sem->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_sem->m_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_timeout);
			return true;
		}

		int await_resume() const noexcept { return m_node.result; }

	private:
		sem_adapter* m_sem {};
		ostick_t m_timeout {};
		detail::wait_node m_node {};
	};

	//! Wait for a token without a timeout
	take_awaiter take() noexcept { return take_awaiter{ this, ISIX_TIME_INFINITE }; }

	//! Wait for a token with a timeout (ISIX_TIME_DONTWAIT = try only)
	take_awaiter take(ostick_t timeout) noexcept { return take_awaiter{ this, timeout }; }

private:
	static void dispatch_cb(void* ctx) { static_cast<sem_adapter*>(ctx)->dispatch(); }

	void dispatch() noexcept
	{
		while (auto* l = m_waiters.front()) {
			if (!try_take()) {
				break;
			}
			m_sched->wake(*detail::node_of(l), ISIX_EOK);
		}
	}

	scheduler* m_sched {};
	ossem_t m_sem {};
	osbitset_t m_bit {};
	bool m_connected { false };
	detail::dlist m_waiters {};
};

} // namespace isix::co

#endif
