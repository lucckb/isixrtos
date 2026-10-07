/*
 * Coroutine-native counting semaphore. signal() is safe from any thread,
 * signal_isr() from an ISR; take() is awaited in a coroutine of the owning
 * scheduler. Tokens are handed to waiters in FIFO order.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <climits>
#include <coroutine>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/error.h>
#include <isix/ostime.h>

namespace isix::co {

class semaphore final {
public:
	explicit semaphore(scheduler& sched, int initial = 0, int limit = INT_MAX) noexcept
		: m_sched(&sched), m_signal(&semaphore::dispatch_cb, this)
		, m_limit(limit), m_count(initial) {}

	semaphore(const semaphore&) = delete;
	semaphore& operator=(const semaphore&) = delete;

	//! Waiters are woken with ISIX_EDESTROY
	~semaphore()
	{
		m_sched->unsignal(m_signal);
		while (auto* l = m_waiters.front()) {
			m_sched->wake(*detail::from_wait_link(l), ISIX_EDESTROY);
		}
	}

	[[nodiscard]] bool is_valid() const noexcept { return m_sched->is_valid(); }

	//! Release a token from a thread
	int signal() noexcept
	{
		if (!is_valid()) {
			return ISIX_EINVARG;
		}
		m_sched->signal(m_signal, [this] { add_token(); });
		return ISIX_EOK;
	}

	//! Release a token from an ISR
	int signal_isr() noexcept
	{
		if (!is_valid()) {
			return ISIX_EINVARG;
		}
		m_sched->signal_isr(m_signal, [this] { add_token(); });
		return ISIX_EOK;
	}

	//! Take a token without waiting (any thread)
	[[nodiscard]] bool try_take() noexcept
	{
		detail::critical_guard g;
		if (m_count > 0) {
			--m_count;
			return true;
		}
		return false;
	}

	class take_awaiter final {
	public:
		take_awaiter(semaphore* sem, ostick_t timeout) noexcept : m_sem(sem) { m_node.deadline = timeout; }

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_sem || !m_sem->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_sem->m_waiters.empty() && m_sem->try_take()) {
				return true;
			}
			if (m_node.deadline == ISIX_TIME_DONTWAIT) {
				m_node.result = ISIX_ETIMEOUT;
				return true;
			}
			return false;
		}

		template<typename P>
		bool await_suspend(std::coroutine_handle<P> h) noexcept
		{
			if (detail::abort_if_cancelled(h, m_node)) {
				return false;
			}
			auto* const sched = h.promise().sched;
			if (sched != m_sem->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_sem->m_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_node.deadline);
			return true;
		}

		int await_resume() const noexcept { return m_node.result; }

	private:
		semaphore* m_sem {};
		detail::wait_node m_node {};
	};

	//! Wait for a token without a timeout
	take_awaiter take() noexcept { return take_awaiter{ this, ISIX_TIME_INFINITE }; }

	//! Wait for a token with a timeout (ISIX_TIME_DONTWAIT = try only)
	take_awaiter take(ostick_t timeout) noexcept { return take_awaiter{ this, timeout }; }

private:
	static void dispatch_cb(void* ctx) { static_cast<semaphore*>(ctx)->dispatch(); }

	// Called inside the critical section of the scheduler signal
	void add_token() noexcept
	{
		if (m_count < m_limit) {
			++m_count;
		}
	}

	void dispatch() noexcept
	{
		while (auto* l = m_waiters.front()) {
			if (!try_take()) {
				break;
			}
			m_sched->wake(*detail::from_wait_link(l), ISIX_EOK);
		}
	}

	scheduler* m_sched {};
	detail::signal_link m_signal;
	int m_limit {};
	int m_count {};
	detail::dlist m_waiters {};
};

} // namespace isix::co

#endif
