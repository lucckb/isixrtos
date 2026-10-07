/*
 * Coroutine-native auto-reset event. set() is safe from any thread,
 * set_isr() from an ISR. A signal sent with no waiter stays latched.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <coroutine>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/error.h>
#include <isix/ostime.h>

namespace isix::co {

class event final {
public:
	explicit event(scheduler& sched) noexcept
		: m_sched(&sched), m_bit(sched.alloc_bit(&event::dispatch_cb, this)) {}

	event(const event&) = delete;
	event& operator=(const event&) = delete;

	//! Waiters are woken with ISIX_EDESTROY
	~event()
	{
		while (auto* l = m_waiters.front()) {
			m_sched->wake(*detail::node_of(l), ISIX_EDESTROY);
			l->unlink();
		}
		m_sched->free_bit(m_bit);
	}

	[[nodiscard]] bool is_valid() const noexcept { return m_bit != 0U; }

	//! Signal from a thread
	void set() noexcept
	{
		if (m_bit) {
			latch();
			m_sched->signal(m_bit);
		}
	}

	//! Signal from an ISR
	void set_isr() noexcept
	{
		if (m_bit) {
			latch();
			m_sched->signal_isr(m_bit);
		}
	}

	class awaiter final {
	public:
		awaiter(event* ev, ostick_t timeout) noexcept : m_ev(ev), m_timeout(timeout) {}

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_ev || !m_ev->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_ev->consume()) {
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
			if (sched != m_ev->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_ev->m_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_timeout);
			return true;
		}

		int await_resume() const noexcept { return m_node.result; }

	private:
		event* m_ev {};
		ostick_t m_timeout {};
		detail::wait_node m_node {};
	};

	//! Wait without a timeout
	awaiter operator co_await() noexcept { return awaiter{ this, ISIX_TIME_INFINITE }; }

	//! Wait with a timeout (ISIX_TIME_DONTWAIT = test only)
	awaiter wait_for(ostick_t t) noexcept { return awaiter{ this, t }; }

private:
	static void dispatch_cb(void* ctx) { static_cast<event*>(ctx)->dispatch(); }

	void latch() noexcept
	{
		detail::critical_guard g;
		m_signaled = true;
	}

	bool consume() noexcept
	{
		detail::critical_guard g;
		return std::exchange(m_signaled, false);
	}

	void dispatch() noexcept
	{
		if (m_waiters.empty() || !consume()) {
			return;
		}
		while (auto* l = m_waiters.front()) {
			m_sched->wake(*detail::node_of(l), ISIX_EOK);
			l->unlink();
		}
	}

	scheduler* m_sched {};
	osbitset_t m_bit {};
	bool m_signaled { false };
	detail::dlist m_waiters {};
};

} // namespace isix::co

#endif
