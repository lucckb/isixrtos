/*
 * Coroutine-native bounded channel. try_send()/try_recv() are safe from any
 * thread, the *_isr variants from an ISR; send()/recv() are awaited in
 * coroutines of the owning scheduler.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <array>
#include <coroutine>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/error.h>
#include <isix/ostime.h>

namespace isix::co {

template<typename T, std::size_t N>
class channel final {
	static_assert(N > 0);
public:
	explicit channel(scheduler& sched) noexcept
		: m_sched(&sched), m_signal(&channel::dispatch_cb, this) {}

	channel(const channel&) = delete;
	channel& operator=(const channel&) = delete;

	//! Waiters are woken with ISIX_EDESTROY
	~channel()
	{
		m_sched->unsignal(m_signal);
		while (auto* l = m_rx_waiters.front()) {
			m_sched->wake(*detail::from_wait_link(l), ISIX_EDESTROY);
		}
		while (auto* l = m_tx_waiters.front()) {
			m_sched->wake(*detail::from_wait_link(l), ISIX_EDESTROY);
		}
	}

	[[nodiscard]] bool is_valid() const noexcept { return m_sched->is_valid(); }

	//! Send without waiting (thread context)
	bool try_send(T v) noexcept
	{
		if (!is_valid() || !push(std::move(v))) {
			return false;
		}
		m_sched->signal(m_signal);
		return true;
	}

	//! Send without waiting (ISR context)
	bool try_send_isr(T v) noexcept requires std::is_trivially_copyable_v<T>
	{
		if (!is_valid() || !push(std::move(v))) {
			return false;
		}
		m_sched->signal_isr(m_signal);
		return true;
	}

	//! Receive without waiting (thread context)
	std::optional<T> try_recv() noexcept
	{
		if (!is_valid()) {
			return std::nullopt;
		}
		auto v = pop();
		if (v) {
			m_sched->signal(m_signal);
		}
		return v;
	}

	//! Receive without waiting (ISR context)
	std::optional<T> try_recv_isr() noexcept requires std::is_trivially_copyable_v<T>
	{
		if (!is_valid()) {
			return std::nullopt;
		}
		auto v = pop();
		if (v) {
			m_sched->signal_isr(m_signal);
		}
		return v;
	}

	class send_awaiter final {
	public:
		send_awaiter(channel* ch, T v, ostick_t timeout) noexcept
			: m_ch(ch)
		{
			m_node.deadline = timeout;
			m_node.value.emplace(std::move(v));
		}

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_ch || !m_ch->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_ch->m_tx_waiters.empty() && m_ch->push(std::move(*m_node.value))) {
				m_ch->m_sched->signal(m_ch->m_signal);
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
			if (sched != m_ch->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_ch->m_tx_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_node.deadline);
			return true;
		}

		int await_resume() const noexcept { return m_node.result; }

	private:
		struct node final : detail::wait_node {
			std::optional<T> value {};
		};
		friend class channel;

		channel* m_ch {};
		node m_node {};
	};

	class recv_awaiter final {
	public:
		recv_awaiter(channel* ch, ostick_t timeout) noexcept : m_ch(ch) { m_node.deadline = timeout; }

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_ch || !m_ch->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_ch->m_rx_waiters.empty()) {
				m_node.out = m_ch->pop();
				if (m_node.out) {
					m_ch->m_sched->signal(m_ch->m_signal);
					return true;
				}
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
			if (sched != m_ch->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_ch->m_rx_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_node.deadline);
			return true;
		}

		//! The value or ISIX_ETIMEOUT, ISIX_EDESTROY, ISIX_ECANCELED
		isix::co::result<T> await_resume() noexcept
		{
			if (m_node.result != ISIX_EOK || !m_node.out) {
				return std::unexpected(m_node.result != ISIX_EOK ? static_cast<int>(m_node.result) : ISIX_ESTATE);
			}
			return std::move(*m_node.out);
		}

	private:
		struct node final : detail::wait_node {
			std::optional<T> out {};
		};
		friend class channel;

		channel* m_ch {};
		node m_node {};
	};

	//! Send, waiting for free space
	send_awaiter send(T v) noexcept { return send_awaiter{ this, std::move(v), ISIX_TIME_INFINITE }; }

	//! Send with a timeout (ISIX_TIME_DONTWAIT = try only)
	send_awaiter send(T v, ostick_t timeout) noexcept
	{
		return send_awaiter{ this, std::move(v), timeout };
	}

	//! Receive, waiting for data
	recv_awaiter recv() noexcept { return recv_awaiter{ this, ISIX_TIME_INFINITE }; }

	//! Receive with a timeout
	recv_awaiter recv(ostick_t timeout) noexcept { return recv_awaiter{ this, timeout }; }

private:
	static void dispatch_cb(void* ctx) { static_cast<channel*>(ctx)->dispatch(); }

	bool push(T&& v) noexcept
	{
		detail::critical_guard g;
		if (m_count >= N) {
			return false;
		}
		m_buf[m_tail] = std::move(v);
		m_tail = (m_tail + 1) % N;
		++m_count;
		return true;
	}

	std::optional<T> pop() noexcept
	{
		detail::critical_guard g;
		if (m_count == 0) {
			return std::nullopt;
		}
		std::optional<T> v { std::move(m_buf[m_head]) };
		m_head = (m_head + 1) % N;
		--m_count;
		return v;
	}

	//! Serve both waiter queues until neither side can progress
	void dispatch() noexcept
	{
		bool progress = true;
		while (progress) {
			progress = false;
			while (auto* l = m_rx_waiters.front()) {
				auto v = pop();
				if (!v) {
					break;
				}
				auto* const n = static_cast<typename recv_awaiter::node*>(detail::from_wait_link(l));
				n->out = std::move(v);
				m_sched->wake(*n, ISIX_EOK);
				progress = true;
			}
			while (auto* l = m_tx_waiters.front()) {
				auto* const n = static_cast<typename send_awaiter::node*>(detail::from_wait_link(l));
				if (!push(std::move(*n->value))) {
					break;
				}
				m_sched->wake(*n, ISIX_EOK);
				progress = true;
			}
		}
	}

	scheduler* m_sched {};
	detail::signal_link m_signal;
	std::array<T, N> m_buf {};
	std::size_t m_head {};
	std::size_t m_tail {};
	std::size_t m_count {};
	detail::dlist m_rx_waiters {};
	detail::dlist m_tx_waiters {};
};

} // namespace isix::co

#endif
