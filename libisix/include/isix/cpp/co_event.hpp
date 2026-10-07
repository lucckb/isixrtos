/*
 * Coroutine-native auto-reset event. set() is safe from any thread,
 * set_isr() from an ISR. A signal sent with no waiter stays latched.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <coroutine>
#include <cstdint>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/error.h>
#include <isix/ostime.h>

namespace isix::co {

class event final {
public:
	explicit event(scheduler& sched) noexcept
		: m_sched(&sched), m_signal(&event::dispatch_cb, this) {}

	event(const event&) = delete;
	event& operator=(const event&) = delete;

	//! Waiters are woken with ISIX_EDESTROY
	~event()
	{
		m_sched->unsignal(m_signal);
		while (auto* l = m_waiters.front()) {
			m_sched->wake(*detail::from_wait_link(l), ISIX_EDESTROY);
			l->unlink();
		}
	}

	[[nodiscard]] bool is_valid() const noexcept { return m_sched->is_valid(); }

	//! Signal from a thread; status is what the woken waiters receive
	void set(int status = ISIX_EOK) noexcept
	{
		if (!is_valid()) {
			return;
		}
		// An unqueued signal can be applied in place: waiters are only touched by the scheduler thread
		if (detail::scheduler_access::in_driver_thread(*m_sched) && !m_signal.queued) {
			if (m_waiters.empty()) {
				detail::critical_guard g;
				latch(status);
			} else {
				wake_all(status);
			}
			return;
		}
		m_sched->signal(m_signal, [this, status] { latch(status); });
	}

	//! Signal from an ISR; the status of the last signal before the dispatch wins
	void set_isr(int status = ISIX_EOK) noexcept
	{
		if (is_valid()) {
			m_sched->signal_isr(m_signal, [this, status] { latch(status); });
		}
	}

	class awaiter final {
	public:
		awaiter(event* ev, ostick_t timeout) noexcept : m_ev(ev) { m_node.deadline = timeout; }

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_ev || !m_ev->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_ev->consume(m_node.result)) {
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
			if (sched != m_ev->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_ev->m_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_node.deadline);
			return true;
		}

		int await_resume() const noexcept { return m_node.result; }

	private:
		event* m_ev {};
		detail::wait_node m_node {};
	};

	//! Wait without a timeout
	awaiter operator co_await() noexcept { return awaiter{ this, ISIX_TIME_INFINITE }; }

	//! Wait with a timeout (ISIX_TIME_DONTWAIT = test only)
	awaiter wait_for(ostick_t t) noexcept { return awaiter{ this, t }; }

private:
	static void dispatch_cb(void* ctx) { static_cast<event*>(ctx)->dispatch(); }

	// Called inside the critical section of the scheduler signal
	void latch(int status) noexcept
	{
		m_signaled = true;
		m_status = static_cast<std::int16_t>(status);
	}

	bool consume(std::int16_t& status) noexcept
	{
		detail::critical_guard g;
		status = m_status;
		return std::exchange(m_signaled, false);
	}

	void dispatch() noexcept
	{
		std::int16_t status {};
		if (m_waiters.empty() || !consume(status)) {
			return;
		}
		wake_all(status);
	}

	void wake_all(int status) noexcept
	{
		while (auto* l = m_waiters.front()) {
			m_sched->wake(*detail::from_wait_link(l), status);
			l->unlink();
		}
	}

	scheduler* m_sched {};
	detail::signal_link m_signal;
	std::int16_t m_status { ISIX_EOK };
	bool m_signaled { false };
	detail::dlist m_waiters {};
};

} // namespace isix::co

#endif
