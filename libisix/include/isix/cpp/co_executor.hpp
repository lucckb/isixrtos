/*
 * Scheduler running on its own RTOS thread. Other threads hand work over with
 * post(); several executors at different priorities form the usual
 * high priority hardware layer and low priority application layer.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <cstddef>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/cpp/thread_base.hpp>

namespace isix::co {

class scheduler_thread final : public isix::detail::thread_base {
public:
	scheduler_thread() = default;

	~scheduler_thread() override
	{
		if (tid()) {
			m_sched.stop();
			wait_for();
		}
		stop_thread();
	}

	//! Start the thread, returns ISIX_ENOMEM when it cannot be created
	int start(std::size_t stack_depth, osprio_t priority, unsigned flags = 0) noexcept
	{
		start_thread(stack_depth, priority, flags);
		return tid() ? ISIX_EOK : ISIX_ENOMEM;
	}

	//! Hand a task over to the executor (any thread, not from an ISR)
	template<typename Task>
	int post(Task&& t) noexcept { return m_sched.post(std::forward<Task>(t)); }

	//! Cancel the running coroutines and let the thread finish
	void stop() noexcept { m_sched.stop(); }

	[[nodiscard]] scheduler& sched() noexcept { return m_sched; }

private:
	void runner() override { m_sched.run_forever(); }

	scheduler m_sched {};
};

} // namespace isix::co

#endif
