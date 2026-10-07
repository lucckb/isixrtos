/*
 * Task composition: when_all and when_any run tasks concurrently on the
 * scheduler of the awaiting coroutine. Cancellation is cooperative: losers
 * are asked to stop and the awaiting coroutine resumes after all of them
 * have finished.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <array>
#include <coroutine>
#include <cstddef>
#include <cstdint>

#include <isix/cpp/coroutine.hpp>

namespace isix::co {

namespace detail {

//! Wait record of a coroutine suspended until a group of child tasks finishes
struct join_node : wait_node {
	static constexpr std::uint8_t c_none = 0xFFU;

	promise_common* const* kids {};
	std::uint8_t count {};
	std::uint8_t pending {};
	std::uint8_t winner { c_none };
	bool any { false };
	bool aborted { false };
	std::int8_t abort_reason { ISIX_EOK };

	join_node() noexcept { on_abort = &join_node::abort_cb; }

	//! Attach to the children and suspend the parent until they all finish
	void start(scheduler& sched, promise_common& parent, std::coroutine_handle<> h,
		ostick_t timeout) noexcept
	{
		sched.suspend(*this, h, timeout);
		for (std::uint8_t n = 0; n < count; ++n) {
			if (!running(n)) {
				continue;
			}
			auto& kid = *kids[n];
			kid.cancel_requested = kid.cancel_requested || parent.cancel_requested;
			kid.continuation = this;
			kid.on_finish = &join_node::finish_cb;
			sched.start(kid, kid.h);
		}
	}

	//! Detach from children still running when the awaiting coroutine goes away
	void release() noexcept
	{
		for (std::uint8_t n = 0; n < count; ++n) {
			if (running(n) && kids[n]->continuation == this) {
				kids[n]->continuation = nullptr;
				kids[n]->on_finish = nullptr;
			}
		}
	}

	[[nodiscard]] bool running(std::uint8_t n) const noexcept { return kids[n] && !kids[n]->completed; }

	[[nodiscard]] static std::uint8_t pending_of(promise_common* const* kids, std::uint8_t count) noexcept
	{
		std::uint8_t n = 0;
		for (std::uint8_t i = 0; i < count; ++i) {
			n += kids[i] && !kids[i]->completed ? 1U : 0U;
		}
		return n;
	}

	void cancel_others(const promise_common* except) noexcept
	{
		for (std::uint8_t n = 0; n < count; ++n) {
			if (kids[n] != except && running(n)) {
				cancel_promise(*kids[n]);
			}
		}
	}

private:
	static void abort_cb(wait_node& node, int reason) noexcept
	{
		auto& j = static_cast<join_node&>(node);
		j.aborted = true;
		j.abort_reason = static_cast<std::int8_t>(reason);
		j.cancel_others(nullptr);
	}

	static void finish_cb(promise_common& p) noexcept
	{
		auto& j = static_cast<join_node&>(*p.continuation);
		if (j.winner == c_none) {
			for (std::uint8_t n = 0; n < j.count; ++n) {
				if (j.kids[n] == &p) {
					j.winner = n;
				}
			}
			if (j.any) {
				j.cancel_others(&p);
			}
		}
		if (--j.pending == 0U) {
			p.sched->wake(j, ISIX_EOK);
		}
	}
};

//! Awaiter running the given tasks concurrently as children of the awaiting coroutine
template<bool Any, std::size_t N>
class join_awaiter final {
public:
	template<typename... T>
	explicit join_awaiter(task<T>&... tasks) noexcept
	{
		std::size_t n = 0;
		((m_kids[n++] = bind(tasks)), ...);
		m_node.kids = m_kids.data();
		m_node.count = static_cast<std::uint8_t>(N);
		m_node.any = Any;
		m_node.pending = join_node::pending_of(m_kids.data(), m_node.count);
		for (std::uint8_t i = 0; Any && i < N; ++i) {
			if (m_kids[i] && m_kids[i]->completed && m_node.winner == join_node::c_none) {
				m_node.winner = i;
			}
		}
	}

	join_awaiter(const join_awaiter&) = delete;
	join_awaiter& operator=(const join_awaiter&) = delete;
	~join_awaiter() { m_node.release(); }

	[[nodiscard]] bool await_ready() const noexcept
	{
		return m_node.pending == 0U || (Any && m_node.winner != join_node::c_none);
	}

	template<typename P>
	bool await_suspend(std::coroutine_handle<P> h) noexcept
	{
		auto& parent = static_cast<promise_common&>(h.promise());
		if (!parent.sched) {
			return false;
		}
		m_node.start(*parent.sched, parent, h, ISIX_TIME_INFINITE);
		return true;
	}

	auto await_resume() const noexcept
	{
		if constexpr (Any) {
			return m_node.winner == join_node::c_none ? N : static_cast<std::size_t>(m_node.winner);
		}
	}

private:
	template<typename T>
	static promise_common* bind(task<T>& t) noexcept
	{
		if (!t.valid()) {
			return nullptr;
		}
		auto& p = t.handle().promise();
		p.h = t.handle();
		return &p;
	}

	std::array<promise_common*, N> m_kids {};
	join_node m_node {};
};

//! Awaiter running one task with a deadline; the task is canceled when time runs out
template<typename T>
class timeout_awaiter final {
public:
	timeout_awaiter(task<T>& t, ostick_t timeout) noexcept : m_task(&t), m_timeout(timeout) { init(); }

	timeout_awaiter(task<T>&& t, ostick_t timeout) noexcept
		: m_owned(std::move(t)), m_task(&m_owned), m_timeout(timeout) { init(); }

	timeout_awaiter(const timeout_awaiter&) = delete;
	timeout_awaiter& operator=(const timeout_awaiter&) = delete;
	~timeout_awaiter() { m_node.release(); }

	[[nodiscard]] bool await_ready() const noexcept { return m_node.pending == 0U; }

	template<typename P>
	bool await_suspend(std::coroutine_handle<P> h) noexcept
	{
		auto& parent = static_cast<promise_common&>(h.promise());
		if (!parent.sched) {
			return false;
		}
		m_node.start(*parent.sched, parent, h, m_timeout);
		return true;
	}

	//! ISIX_ETIMEOUT when time ran out, ISIX_ECANCELED when the caller was canceled
	isix::co::result<T> await_resume() noexcept
	{
		if (!m_task->valid()) {
			return std::unexpected(ISIX_ENOMEM);
		}
		if (m_node.aborted) {
			return std::unexpected(m_node.abort_reason == ISIX_ETIMEOUT ? ISIX_ETIMEOUT : ISIX_ECANCELED);
		}
		return m_task->result();
	}

private:
	void init() noexcept
	{
		if (m_task->valid()) {
			auto& p = m_task->handle().promise();
			p.h = m_task->handle();
			m_kid = &p;
		}
		m_node.kids = &m_kid;
		m_node.count = 1;
		m_node.pending = join_node::pending_of(&m_kid, 1);
	}

	task<T> m_owned {};
	task<T>* m_task {};
	ostick_t m_timeout {};
	promise_common* m_kid {};
	join_node m_node {};
};

} // namespace detail

/**
 * Await a task for at most the given number of ticks (0 waits without a limit).
 * When time runs out the task is canceled and the result is ISIX_ETIMEOUT; the
 * caller resumes after the task has finished.
 */
template<typename T>
[[nodiscard]] auto with_timeout(task<T>& t, ostick_t ticks) noexcept
{
	return detail::timeout_awaiter<T>{ t, ticks };
}

//! with_timeout() owning the task
template<typename T>
[[nodiscard]] auto with_timeout(task<T>&& t, ostick_t ticks) noexcept
{
	return detail::timeout_awaiter<T>{ std::move(t), ticks };
}

//! Await tasks running concurrently until all of them have finished
template<typename... T>
[[nodiscard]] auto when_all(task<T>&... tasks) noexcept
{
	return detail::join_awaiter<false, sizeof...(T)>{ tasks... };
}

/**
 * Await tasks running concurrently until one finishes. The others are asked to
 * stop and finish before the caller resumes. Returns the index of the first
 * task that finished.
 */
template<typename... T>
[[nodiscard]] auto when_any(task<T>&... tasks) noexcept
{
	return detail::join_awaiter<true, sizeof...(T)>{ tasks... };
}

} // namespace isix::co

#endif
