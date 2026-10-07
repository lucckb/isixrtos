/*
 * Coroutine adapter for isix::fifo. Items written by threads or ISRs with the
 * regular fifo API are awaited in a coroutine of the owning scheduler. The
 * adapter must be the only consumer of the fifo. Requires
 * CONFIG_ISIX_FIFO_EVENT_NOTIFY.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus) && CONFIG_ISIX_FIFO_EVENT_NOTIFY

#include <bit>
#include <coroutine>
#include <optional>
#include <type_traits>
#include <utility>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/error.h>
#include <isix/fifo.h>
#include <isix/fifo_event.h>
#include <isix/ostime.h>

namespace isix::co {

template<typename T>
class fifo_reader final {
	static_assert(std::is_trivially_copyable_v<T>);
public:
	fifo_reader(scheduler& sched, osfifo_t fifo) noexcept
		: m_sched(&sched), m_fifo(fifo), m_bit(sched.alloc_bit(&fifo_reader::dispatch_cb, this))
	{
		if (m_bit && m_fifo) {
			const auto rc = isix_fifo_event_connect(m_fifo, sched.event_handle(), std::countr_zero(m_bit));
			m_connected = rc == ISIX_EOK;
		}
	}

	template<typename F>
	fifo_reader(scheduler& sched, F& fifo) noexcept : fifo_reader(sched, fifo.native_handle()) {}

	fifo_reader(const fifo_reader&) = delete;
	fifo_reader& operator=(const fifo_reader&) = delete;

	//! Waiters are woken with ISIX_EDESTROY
	~fifo_reader()
	{
		if (m_connected) {
			isix_fifo_event_disconnect(m_fifo, m_sched->event_handle());
		}
		while (auto* l = m_waiters.front()) {
			m_sched->wake(*detail::node_of(l), ISIX_EDESTROY);
		}
		m_sched->free_bit(m_bit);
	}

	[[nodiscard]] bool is_valid() const noexcept { return m_connected; }

	//! Read without waiting
	[[nodiscard]] std::optional<T> try_read() noexcept
	{
		if (!m_connected || isix_fifo_count(m_fifo) <= 0) {
			return std::nullopt;
		}
		// Single consumer: the element is present, so the read does not block
		T v {};
		if (isix_fifo_read(m_fifo, &v, ISIX_TIME_INFINITE) != ISIX_EOK) {
			return std::nullopt;
		}
		return v;
	}

	class read_awaiter final {
	public:
		read_awaiter(fifo_reader* rd, ostick_t timeout) noexcept : m_rd(rd), m_timeout(timeout) {}

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_rd || !m_rd->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_rd->m_waiters.empty()) {
				m_node.out = m_rd->try_read();
				if (m_node.out) {
					return true;
				}
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
			if (sched != m_rd->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_rd->m_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_timeout);
			return true;
		}

		std::optional<T> await_resume() noexcept
		{
			if (m_node.result != ISIX_EOK) {
				return std::nullopt;
			}
			return std::move(m_node.out);
		}

	private:
		struct node final : detail::wait_node {
			std::optional<T> out {};
		};
		friend class fifo_reader;

		fifo_reader* m_rd {};
		ostick_t m_timeout {};
		node m_node {};
	};

	//! Read, waiting for data
	read_awaiter read() noexcept { return read_awaiter{ this, ISIX_TIME_INFINITE }; }

	//! Read with a timeout, std::nullopt on timeout (ISIX_TIME_DONTWAIT = try only)
	read_awaiter read(ostick_t timeout) noexcept { return read_awaiter{ this, timeout }; }

private:
	static void dispatch_cb(void* ctx) { static_cast<fifo_reader*>(ctx)->dispatch(); }

	void dispatch() noexcept
	{
		while (auto* l = m_waiters.front()) {
			auto v = try_read();
			if (!v) {
				break;
			}
			auto* const n = static_cast<typename read_awaiter::node*>(detail::node_of(l));
			n->out = std::move(v);
			m_sched->wake(*n, ISIX_EOK);
		}
	}

	scheduler* m_sched {};
	osfifo_t m_fifo {};
	osbitset_t m_bit {};
	bool m_connected { false };
	detail::dlist m_waiters {};
};

} // namespace isix::co

#endif
