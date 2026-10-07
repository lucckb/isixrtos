/*
 * Async scope: owns coroutines started without a task object (fire and
 * forget) and lets the owner cancel them or wait until they have finished.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <coroutine>
#include <cstddef>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/error.h>

namespace isix::co {

/**
 * Scheduler thread only. Destroy the scope outside of the coroutines it runs;
 * coroutines still alive are destroyed with it.
 */
class async_scope final {
public:
	explicit async_scope(scheduler& sched) noexcept { m_core.sched = &sched; }

	async_scope(const async_scope&) = delete;
	async_scope& operator=(const async_scope&) = delete;

	~async_scope() { m_core.destroy_all(); }

	//! Start a task that nobody awaits. Returns ISIX_ENOMEM for a task without a frame.
	template<typename Task>
	int spawn(Task&& t, [[maybe_unused]] const char* name = nullptr) noexcept
	{
		if (!t.valid()) {
			return ISIX_ENOMEM;
		}
		const auto h = t.release();
		detail::promise_common& p = h.promise();
		p.h = h;
#if CONFIG_ISIX_CO_DEBUG
		p.name = name;
#endif
		m_core.adopt(p);
		return ISIX_EOK;
	}

	//! Ask every running task to stop
	void cancel_all() noexcept { m_core.cancel_all(); }

	//! Number of tasks still running
	[[nodiscard]] std::size_t size() const noexcept { return m_core.count; }

	class join_awaiter final {
	public:
		explicit join_awaiter(detail::scope_core& core) noexcept : m_core(&core) {}
		join_awaiter(const join_awaiter&) = delete;
		join_awaiter& operator=(const join_awaiter&) = delete;
		~join_awaiter()
		{
			if (m_core->joiner == &m_node) {
				m_core->joiner = nullptr;
			}
		}

		[[nodiscard]] bool await_ready() const noexcept { return m_core->count == 0U; }

		template<typename P>
		bool await_suspend(std::coroutine_handle<P> h) noexcept
		{
			if (detail::abort_if_cancelled(h, m_node)) {
				return false;
			}
			if (m_core->joiner) {
				m_node.result = ISIX_EBUSY;
				return false;
			}
			m_core->joiner = &m_node;
			m_core->sched->suspend(m_node, h, ISIX_TIME_INFINITE);
			return true;
		}

		//! ISIX_EOK, ISIX_ECANCELED when the waiter was canceled
		int await_resume() const noexcept { return m_node.result; }

	private:
		detail::scope_core* m_core {};
		detail::wait_node m_node {};
	};

	//! Wait until every task has finished
	[[nodiscard]] join_awaiter join() noexcept { return join_awaiter{ m_core }; }

private:
	detail::scope_core m_core {};
};

} // namespace isix::co

#endif
