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
		: m_sched(&sched), m_bit(sched.alloc_bit(&channel::dispatch_cb, this)) {}

	channel(const channel&) = delete;
	channel& operator=(const channel&) = delete;

	//! Waiters are woken with ISIX_EDESTROY
	~channel()
	{
		while (auto* l = m_rx_waiters.front()) {
			m_sched->wake(*detail::node_of(l), ISIX_EDESTROY);
		}
		while (auto* l = m_tx_waiters.front()) {
			m_sched->wake(*detail::node_of(l), ISIX_EDESTROY);
		}
		m_sched->free_bit(m_bit);
	}

	[[nodiscard]] bool is_valid() const noexcept { return m_bit != 0U; }

	//! Send without waiting (thread context)
	bool try_send(T v) noexcept
	{
		if (!m_bit || !push(std::move(v))) {
			return false;
		}
		m_sched->signal(m_bit);
		return true;
	}

	//! Send without waiting (ISR context)
	bool try_send_isr(T v) noexcept requires std::is_trivially_copyable_v<T>
	{
		if (!m_bit || !push(std::move(v))) {
			return false;
		}
		m_sched->signal_isr(m_bit);
		return true;
	}

	//! Receive without waiting (thread context)
	std::optional<T> try_recv() noexcept
	{
		if (!m_bit) {
			return std::nullopt;
		}
		auto v = pop();
		if (v) {
			m_sched->signal(m_bit);
		}
		return v;
	}

	//! Receive without waiting (ISR context)
	std::optional<T> try_recv_isr() noexcept requires std::is_trivially_copyable_v<T>
	{
		if (!m_bit) {
			return std::nullopt;
		}
		auto v = pop();
		if (v) {
			m_sched->signal_isr(m_bit);
		}
		return v;
	}

	class send_awaiter final {
	public:
		send_awaiter(channel* ch, T v, ostick_t timeout) noexcept
			: m_ch(ch), m_timeout(timeout) { m_node.value.emplace(std::move(v)); }

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_ch || !m_ch->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_ch->m_tx_waiters.empty() && m_ch->push(std::move(*m_node.value))) {
				m_ch->m_sched->signal(m_ch->m_bit);
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
			if (sched != m_ch->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_ch->m_tx_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_timeout);
			return true;
		}

		int await_resume() const noexcept { return m_node.result; }

	private:
		struct node final : detail::wait_node {
			std::optional<T> value {};
		};
		friend class channel;

		channel* m_ch {};
		ostick_t m_timeout {};
		node m_node {};
	};

	class recv_awaiter final {
	public:
		recv_awaiter(channel* ch, ostick_t timeout) noexcept : m_ch(ch), m_timeout(timeout) {}

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_ch || !m_ch->is_valid()) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_ch->m_rx_waiters.empty()) {
				m_node.out = m_ch->pop();
				if (m_node.out) {
					m_ch->m_sched->signal(m_ch->m_bit);
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
			if (sched != m_ch->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_ch->m_rx_waiters.push_back(m_node.wait_link);
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
		friend class channel;

		channel* m_ch {};
		ostick_t m_timeout {};
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

	//! Receive with a timeout, std::nullopt on timeout
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
				auto* const n = static_cast<typename recv_awaiter::node*>(detail::node_of(l));
				n->out = std::move(v);
				m_sched->wake(*n, ISIX_EOK);
				progress = true;
			}
			while (auto* l = m_tx_waiters.front()) {
				auto* const n = static_cast<typename send_awaiter::node*>(detail::node_of(l));
				if (!push(std::move(*n->value))) {
					break;
				}
				m_sched->wake(*n, ISIX_EOK);
				progress = true;
			}
		}
	}

	scheduler* m_sched {};
	osbitset_t m_bit {};
	std::array<T, N> m_buf {};
	std::size_t m_head {};
	std::size_t m_tail {};
	std::size_t m_count {};
	detail::dlist m_rx_waiters {};
	detail::dlist m_tx_waiters {};
};

} // namespace isix::co

#endif
