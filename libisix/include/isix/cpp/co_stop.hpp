/*
 * Stop source: lets a thread or an ISR request cooperative cancellation of a
 * task running on a scheduler.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <isix/cpp/coroutine_scheduler.hpp>

namespace isix::co {

class stop_source final {
public:
	explicit stop_source(scheduler& sched) noexcept
		: m_sched(&sched), m_signal(&stop_source::dispatch_cb, this) {}

	stop_source(const stop_source&) = delete;
	stop_source& operator=(const stop_source&) = delete;

	~stop_source() { m_sched->unsignal(m_signal); }

	/**
	 * Attach the task to cancel. Call from the scheduler thread. The task must
	 * outlive the binding, call unbind() before destroying it earlier.
	 */
	template<typename Task>
	void bind(Task& t) noexcept
	{
		if (!t.valid()) {
			return;
		}
		m_bound = &t.handle().promise();
		if (stop_requested()) {
			detail::cancel_promise(*m_bound);
		}
	}

	//! Detach the bound task (scheduler thread)
	void unbind() noexcept { m_bound = nullptr; }

	//! Clear a stop request so the source can be bound again (scheduler thread)
	void reset() noexcept
	{
		m_bound = nullptr;
		detail::critical_guard g;
		m_stop = false;
	}

	[[nodiscard]] bool stop_requested() const noexcept
	{
		detail::critical_guard g;
		return m_stop;
	}

	//! Request cancellation from a thread
	void request_stop() noexcept
	{
		latch();
		m_sched->signal(m_signal);
	}

	//! Request cancellation from an ISR
	void request_stop_isr() noexcept
	{
		latch();
		m_sched->signal_isr(m_signal);
	}

private:
	static void dispatch_cb(void* ctx) { static_cast<stop_source*>(ctx)->dispatch(); }

	void latch() noexcept
	{
		detail::critical_guard g;
		m_stop = true;
	}

	void dispatch() noexcept
	{
		if (m_bound && stop_requested()) {
			detail::cancel_promise(*m_bound);
		}
	}

	scheduler* m_sched {};
	detail::signal_link m_signal;
	detail::promise_common* m_bound {};
	bool m_stop { false };
};

} // namespace isix::co

#endif
