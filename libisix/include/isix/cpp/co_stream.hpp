/*
 * Coroutine-native byte stream buffer. One producer, a thread or an ISR,
 * writes bytes with write()/write_isr(); one coroutine of the owning scheduler
 * awaits them with read(). Bytes that do not fit are dropped and counted.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <algorithm>
#include <array>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <span>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/error.h>
#include <isix/ostime.h>

namespace isix::co {

template<std::size_t N>
class stream_buffer final {
	static_assert(N > 0);
public:
	explicit stream_buffer(scheduler& sched) noexcept
		: m_sched(&sched), m_signal(&stream_buffer::dispatch_cb, this) {}

	stream_buffer(const stream_buffer&) = delete;
	stream_buffer& operator=(const stream_buffer&) = delete;

	//! The waiting reader is woken with ISIX_EDESTROY
	~stream_buffer()
	{
		m_sched->unsignal(m_signal);
		while (auto* l = m_waiters.front()) {
			m_sched->wake(*detail::from_wait_link(l), ISIX_EDESTROY);
		}
	}

	[[nodiscard]] bool is_valid() const noexcept { return m_sched->is_valid(); }

	//! Add bytes from a thread, returns how many were stored
	std::size_t write(std::span<const std::byte> data) noexcept
	{
		const auto n = push(data);
		if (n > 0) {
			m_sched->signal(m_signal);
		}
		return n;
	}

	//! Add bytes from an ISR, returns how many were stored
	std::size_t write_isr(std::span<const std::byte> data) noexcept
	{
		const auto n = push(data);
		if (n > 0) {
			m_sched->signal_isr(m_signal);
		}
		return n;
	}

	//! Bytes waiting to be read
	[[nodiscard]] std::size_t available() const noexcept
	{
		detail::critical_guard g;
		return m_count;
	}

	//! Bytes dropped because the buffer was full
	[[nodiscard]] std::size_t dropped() const noexcept
	{
		detail::critical_guard g;
		return m_dropped;
	}

	class read_awaiter final {
	public:
		read_awaiter(stream_buffer* sb, std::span<std::byte> buf, std::size_t min_bytes,
			ostick_t timeout) noexcept
			: m_sb(sb), m_buf(buf)
		{
			m_node.deadline = timeout;
			m_min = std::clamp<std::size_t>(min_bytes, 1U, std::min(N, buf.size()));
		}

		read_awaiter(const read_awaiter&) = delete;
		read_awaiter& operator=(const read_awaiter&) = delete;

		[[nodiscard]] bool await_ready() noexcept
		{
			if (m_buf.empty() || !m_sb->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (!m_sb->m_waiters.empty()) {
				m_node.result = ISIX_EBUSY;
				return true;
			}
			if (m_sb->available() >= m_min) {
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
			if (sched != m_sb->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_sb->m_want = m_min;
			m_sb->m_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_node.deadline);
			return true;
		}

		//! Number of bytes read or ISIX_ETIMEOUT, ISIX_EDESTROY, ISIX_ECANCELED, ISIX_EBUSY
		isix::co::result<std::size_t> await_resume() noexcept
		{
			if (m_node.result != ISIX_EOK) {
				return std::unexpected(static_cast<int>(m_node.result));
			}
			return m_sb->pop(m_buf);
		}

	private:
		stream_buffer* m_sb {};
		std::span<std::byte> m_buf {};
		std::size_t m_min {};
		detail::wait_node m_node {};
	};

	/**
	 * Read between min_bytes and buf.size() bytes, waiting until at least min_bytes
	 * are available. Only one reader may wait at a time.
	 */
	read_awaiter read(std::span<std::byte> buf, std::size_t min_bytes = 1,
		ostick_t timeout = ISIX_TIME_INFINITE) noexcept
	{
		return read_awaiter{ this, buf, min_bytes, timeout };
	}

	read_awaiter read(std::span<std::uint8_t> buf, std::size_t min_bytes = 1,
		ostick_t timeout = ISIX_TIME_INFINITE) noexcept
	{
		return read_awaiter{ this, std::as_writable_bytes(buf), min_bytes, timeout };
	}

private:
	static void dispatch_cb(void* ctx) { static_cast<stream_buffer*>(ctx)->dispatch(); }

	std::size_t push(std::span<const std::byte> data) noexcept
	{
		detail::critical_guard g;
		const auto n = std::min(data.size(), N - m_count);
		for (std::size_t i = 0; i < n; ++i) {
			m_buf[m_tail] = data[i];
			m_tail = m_tail + 1U == N ? 0U : m_tail + 1U;
		}
		m_count += n;
		m_dropped += data.size() - n;
		return n;
	}

	std::size_t pop(std::span<std::byte> out) noexcept
	{
		detail::critical_guard g;
		const auto n = std::min(out.size(), m_count);
		for (std::size_t i = 0; i < n; ++i) {
			out[i] = m_buf[m_head];
			m_head = m_head + 1U == N ? 0U : m_head + 1U;
		}
		m_count -= n;
		return n;
	}

	void dispatch() noexcept
	{
		if (auto* l = m_waiters.front()) {
			if (available() >= m_want) {
				m_sched->wake(*detail::from_wait_link(l), ISIX_EOK);
			}
		}
	}

	scheduler* m_sched {};
	detail::signal_link m_signal;
	std::array<std::byte, N> m_buf {};
	std::size_t m_head {};
	std::size_t m_tail {};
	std::size_t m_count {};
	std::size_t m_dropped {};
	std::size_t m_want {};
	detail::dlist m_waiters {};
};

} // namespace isix::co

#endif
