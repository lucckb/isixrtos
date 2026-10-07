/*
 * Coroutine-native sleep built on scheduler timers.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <coroutine>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/ostime.h>

namespace isix::co {

class sleep_awaiter final {
public:
	explicit sleep_awaiter(ostick_t ticks) noexcept : m_ticks(ticks) {}

	[[nodiscard]] bool await_ready() const noexcept
	{
		return m_ticks == 0 || m_ticks == ISIX_TIME_DONTWAIT;
	}

	template<typename P>
	bool await_suspend(std::coroutine_handle<P> h) noexcept
	{
		auto* const sched = h.promise().sched;
		if (!sched) {
			return false;
		}
		sched->suspend(m_node, h, m_ticks, ISIX_EOK);
		return true;
	}

	void await_resume() const noexcept {}

private:
	ostick_t m_ticks {};
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

} // namespace isix::co

#endif
