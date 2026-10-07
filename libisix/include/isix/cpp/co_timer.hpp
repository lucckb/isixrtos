/*
 * Coroutine-native sleep built on scheduler timers.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <coroutine>
#include <cstdint>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/ostime.h>

namespace isix::co {

class sleep_awaiter final {
public:
	explicit sleep_awaiter(ostick_t ticks) noexcept { m_node.deadline = ticks; }

	[[nodiscard]] bool await_ready() const noexcept
	{
		return m_node.deadline == 0 || m_node.deadline == ISIX_TIME_DONTWAIT;
	}

	template<typename P>
	bool await_suspend(std::coroutine_handle<P> h) noexcept
	{
		if (detail::abort_if_cancelled(h, m_node)) {
			return false;
		}
		auto* const sched = h.promise().sched;
		if (!sched) {
			return false;
		}
		sched->suspend(m_node, h, m_node.deadline, ISIX_EOK);
		return true;
	}

	void await_resume() const noexcept {}

private:
	detail::wait_node m_node {};
};

inline sleep_awaiter sleep_ticks(ostick_t ticks) noexcept
{
	return sleep_awaiter{ ticks };
}

inline sleep_awaiter sleep_ms(unsigned ms) noexcept
{
	return sleep_ticks(isix::ms2tick(ms));
}

//! Sleep until the jiffies counter reaches the deadline; a deadline in the past returns at once
inline sleep_awaiter sleep_until(ostick_t deadline) noexcept
{
	const auto left = static_cast<std::int32_t>(deadline - isix::get_jiffies());
	return sleep_ticks(left > 0 ? static_cast<ostick_t>(left) : 0U);
}

//! Awaiter letting the other ready coroutines run first
class yield_awaiter final {
public:
	[[nodiscard]] bool await_ready() const noexcept { return false; }

	template<typename P>
	bool await_suspend(std::coroutine_handle<P> h) noexcept
	{
		if (detail::abort_if_cancelled(h, m_node)) {
			return false;
		}
		auto* const sched = h.promise().sched;
		if (!sched) {
			return false;
		}
		sched->suspend(m_node, h, ISIX_TIME_INFINITE);
		sched->wake(m_node, ISIX_EOK);
		return true;
	}

	void await_resume() const noexcept {}

private:
	detail::wait_node m_node {};
};

//! Let the other ready coroutines of the scheduler run before continuing
[[nodiscard]] inline yield_awaiter yield() noexcept { return {}; }

} // namespace isix::co

#endif
