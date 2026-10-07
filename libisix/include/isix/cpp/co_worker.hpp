/*
 * Bridge from coroutines to blocking code: a worker thread runs a callable
 * while the awaiting coroutine is suspended and the scheduler keeps serving
 * the other coroutines. Use it for drivers and libraries that block.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <coroutine>
#include <cstddef>
#include <type_traits>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/cpp/thread_base.hpp>
#include <isix/semaphore.h>
#include <isix/cpp/semaphore.hpp>

namespace isix::co {

/**
 * One job at a time. A job is not interruptible: cancelling the awaiting coroutine
 * lets the job finish and then completes the wait with ISIX_ECANCELED, and
 * destroying the coroutine blocks until the job is done.
 */
class worker_thread final : public isix::detail::thread_base {
public:
	explicit worker_thread(scheduler& sched) noexcept
		: m_sched(&sched), m_signal(&worker_thread::dispatch_cb, this), m_job(0, 1), m_end(0, 1) {}

	~worker_thread() override
	{
		stop_thread();
		m_sched->unsignal(m_signal);
	}

	//! Start the worker thread, returns ISIX_ENOMEM when it cannot be created
	int start(std::size_t stack_depth, osprio_t priority, unsigned flags = 0) noexcept
	{
		start_thread(stack_depth, priority, flags);
		return tid() ? ISIX_EOK : ISIX_ENOMEM;
	}

	//! Wait record of a coroutine waiting for a job
	struct run_node final : detail::wait_node {
		bool cancelled { false };
	};

	template<typename F>
	class run_awaiter final {
	public:
		run_awaiter(worker_thread& w, F& f) noexcept : m_w(&w), m_f(&f) { m_node.on_abort = &abort_cb; }
		run_awaiter(const run_awaiter&) = delete;
		run_awaiter& operator=(const run_awaiter&) = delete;

		~run_awaiter()
		{
			if (m_started) {
				m_w->m_end.wait(ISIX_TIME_INFINITE);
			}
			if (m_claimed) {
				m_w->release();
			}
		}

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_w->tid()) {
				m_node.result = ISIX_ESTATE;
				return true;
			}
			if (!m_w->claim()) {
				m_node.result = ISIX_EBUSY;
				return true;
			}
			m_claimed = true;
			return false;
		}

		template<typename P>
		bool await_suspend(std::coroutine_handle<P> h) noexcept
		{
			if (detail::abort_if_cancelled(h, m_node)) {
				return false;
			}
			auto* const sched = h.promise().sched;
			if (sched != m_w->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_w->m_waiter = &m_node;
			sched->suspend(m_node, h, ISIX_TIME_INFINITE);
			m_started = true;
			m_w->m_fn = &thunk;
			m_w->m_ctx = m_f;
			m_w->m_rc = ISIX_EOK;
			m_w->m_job.signal();
			return true;
		}

		//! Result of the callable (void callables give ISIX_EOK) or ISIX_ECANCELED
		int await_resume() const noexcept { return m_node.result; }

	private:
		static void abort_cb(detail::wait_node& n, int) noexcept
		{
			static_cast<run_node&>(n).cancelled = true;
		}

		static int thunk(void* ctx) noexcept
		{
			if constexpr (std::is_void_v<std::invoke_result_t<F&>>) {
				(*static_cast<F*>(ctx))();
				return ISIX_EOK;
			} else {
				return static_cast<int>((*static_cast<F*>(ctx))());
			}
		}

		worker_thread* m_w {};
		F* m_f {};
		bool m_claimed { false };
		bool m_started { false };
		run_node m_node {};
	};

	/**
	 * Run the callable on the worker thread and wait for it without blocking the
	 * scheduler. The callable must stay valid until the co_await completes.
	 */
	template<typename F>
	[[nodiscard]] run_awaiter<F> run(F& f) noexcept { return run_awaiter<F>{ *this, f }; }

private:
	static void dispatch_cb(void* ctx) { static_cast<worker_thread*>(ctx)->dispatch(); }

	void dispatch() noexcept
	{
		bool done {};
		{
			detail::critical_guard g;
			done = m_done;
		}
		auto* const n = m_waiter;
		if (done && n && n->rdy.state == detail::node_state::waiting) {
			m_waiter = nullptr;
			const auto cancelled = static_cast<run_node*>(n)->cancelled;
			m_sched->wake(*n, cancelled ? ISIX_ECANCELED : m_rc);
		}
	}

	bool claim() noexcept
	{
		detail::critical_guard g;
		if (m_busy) {
			return false;
		}
		m_busy = true;
		m_done = false;
		return true;
	}

	void release() noexcept
	{
		detail::critical_guard g;
		m_busy = false;
		m_waiter = nullptr;
	}

	void runner() override
	{
		for (;;) {
			m_job.wait(ISIX_TIME_INFINITE);
			m_rc = m_fn(m_ctx);
			{
				detail::critical_guard g;
				m_done = true;
			}
			m_sched->signal(m_signal);
			m_end.signal();
		}
	}

	scheduler* m_sched {};
	detail::signal_link m_signal;
	isix::semaphore m_job;
	isix::semaphore m_end;
	run_node* m_waiter {};
	int (*m_fn)(void*) noexcept {};
	void* m_ctx {};
	int m_rc { ISIX_EOK };
	bool m_busy { false };
	bool m_done { false };
};

} // namespace isix::co

#endif
