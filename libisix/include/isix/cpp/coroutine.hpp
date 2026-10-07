/*
 * C++20 stackless coroutines for isix: many coroutines scheduled on one RTOS task,
 * woken by events and timers (see coroutine_scheduler.hpp).
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#if (defined(__cpp_impl_coroutine) && __cpp_impl_coroutine >= 201902L) \
	|| (defined(__cpp_coroutines) && __cpp_coroutines >= 201902L)

#include <coroutine>
#include <cstdint>
#include <new>
#include <optional>
#include <type_traits>
#include <utility>

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#include <exception>
#define ISIX_CO_HAS_EXCEPTIONS 1
#endif

#include <isix/assert.h>
#include <isix/cpp/co_channel.hpp>
#include <isix/cpp/co_event.hpp>
#include <isix/cpp/co_executor.hpp>
#include <isix/cpp/co_fifo.hpp>
#include <isix/cpp/co_frame.hpp>
#include <isix/cpp/co_generator.hpp>
#include <isix/cpp/co_mutex.hpp>
#include <isix/cpp/co_sem_adapter.hpp>
#include <isix/cpp/co_semaphore.hpp>
#include <isix/cpp/co_scope.hpp>
#include <isix/cpp/co_stop.hpp>
#include <isix/cpp/co_stream.hpp>
#include <isix/cpp/co_timer.hpp>
#include <isix/cpp/co_worker.hpp>
#include <isix/cpp/coroutine_scheduler.hpp>

namespace isix::co {

template<typename T>
class task;

//! Awaiter returned by cancelled(): reports a cancellation request without suspending
class cancelled_awaiter final {
public:
	[[nodiscard]] bool await_ready() const noexcept { return false; }

	template<typename P>
	bool await_suspend(std::coroutine_handle<P> h) noexcept
	{
		m_cancelled = h.promise().cancel_requested;
		return false;
	}

	[[nodiscard]] bool await_resume() const noexcept { return m_cancelled; }

private:
	bool m_cancelled { false };
};

//! co_await cancelled() tells whether the running task was asked to stop
[[nodiscard]] inline cancelled_awaiter cancelled() noexcept { return {}; }

namespace detail {

//! Frame allocation shared by all task promises; null results are handled by the promise
struct frame_alloc_base : promise_common {
	static void* operator new(std::size_t size) noexcept { return counted(frame_acquire(size), size); }

	template<std::size_t N, typename... A>
	static void* operator new(std::size_t size, frame_storage<N>& s, A&&...) noexcept
	{
		return counted(s.acquire(size), size);
	}

	template<std::size_t B, std::size_t C, typename... A>
	static void* operator new(std::size_t size, frame_pool<B, C>& p, A&&...) noexcept
	{
		return counted(p.acquire(size), size);
	}

	//! Member coroutines and lambdas pass the object first
	template<typename Self, std::size_t N, typename... A>
	static void* operator new(std::size_t size, Self&, frame_storage<N>& s, A&&...) noexcept
	{
		return counted(s.acquire(size), size);
	}

	template<typename Self, std::size_t B, std::size_t C, typename... A>
	static void* operator new(std::size_t size, Self&, frame_pool<B, C>& p, A&&...) noexcept
	{
		return counted(p.acquire(size), size);
	}

	static void operator delete(void* frame, std::size_t size) noexcept
	{
#if CONFIG_ISIX_CO_DEBUG
		critical_guard g;
		--g_frame_counters.live;
		g_frame_counters.bytes -= static_cast<std::uint32_t>(size);
#endif
		frame_release(frame, size);
	}

private:
	static void* counted(void* frame, [[maybe_unused]] std::size_t size) noexcept
	{
#if CONFIG_ISIX_CO_DEBUG
		if (frame) {
			critical_guard g;
			++g_frame_counters.live;
			g_frame_counters.bytes += static_cast<std::uint32_t>(size);
			g_frame_counters.peak_bytes = g_frame_counters.bytes > g_frame_counters.peak_bytes
				? g_frame_counters.bytes : g_frame_counters.peak_bytes;
		}
#endif
		return frame;
	}
};

} // namespace detail

template<typename T>
struct task_promise final : detail::frame_alloc_base {
	std::optional<T> result_value {};

	task<T> get_return_object();
	static task<T> get_return_object_on_allocation_failure() noexcept;

	std::suspend_always initial_suspend() noexcept { return {}; }
	auto final_suspend() noexcept
	{
		struct final_awaiter {
			bool await_ready() const noexcept { return false; }
			void await_suspend(std::coroutine_handle<task_promise> h) const noexcept
			{
				static_cast<detail::promise_common&>(h.promise()).finish();
			}
			void await_resume() const noexcept {}
		};
		return final_awaiter{};
	}

	template<typename U>
		requires std::convertible_to<U&&, T>
	void return_value(U&& v)
	{
		result_value.emplace(std::forward<U>(v));
	}

#ifdef ISIX_CO_HAS_EXCEPTIONS
	std::exception_ptr exception {};

	void unhandled_exception() noexcept { exception = std::current_exception(); }
#else
	void unhandled_exception() noexcept { std::terminate(); }
#endif
};

template<>
struct task_promise<void> final : detail::frame_alloc_base {
	task<void> get_return_object();
	static task<void> get_return_object_on_allocation_failure() noexcept;

	std::suspend_always initial_suspend() noexcept { return {}; }
	auto final_suspend() noexcept
	{
		struct final_awaiter {
			bool await_ready() const noexcept { return false; }
			void await_suspend(std::coroutine_handle<task_promise> h) const noexcept
			{
				static_cast<detail::promise_common&>(h.promise()).finish();
			}
			void await_resume() const noexcept {}
		};
		return final_awaiter{};
	}

	void return_void() noexcept {}

#ifdef ISIX_CO_HAS_EXCEPTIONS
	std::exception_ptr exception {};

	void unhandled_exception() noexcept { exception = std::current_exception(); }
#else
	void unhandled_exception() noexcept { std::terminate(); }
#endif
};

template<typename T>
class task {
public:
	using promise_type = task_promise<T>;

	task() noexcept = default;

	explicit task(std::coroutine_handle<promise_type> h) noexcept : coro_(h) {}

	task(task&& o) noexcept : coro_(std::exchange(o.coro_, {})) {}

	task& operator=(task&& o) noexcept
	{
		if (this != &o) {
			destroy();
			coro_ = std::exchange(o.coro_, {});
		}
		return *this;
	}

	task(const task&) = delete;
	task& operator=(const task&) = delete;

	~task() { destroy(); }

	bool valid() const noexcept { return static_cast<bool>(coro_); }

	std::coroutine_handle<promise_type> release() noexcept
	{
		return std::exchange(coro_, {});
	}

	std::coroutine_handle<promise_type> handle() const noexcept { return coro_; }

	bool done() const noexcept { return !coro_ || coro_.promise().completed; }

	//! ISIX_EOK when finished normally, ISIX_ECANCELED when canceled, ISIX_ESTATE when not finished
	[[nodiscard]] int error() const noexcept
	{
		if (!coro_ || !coro_.promise().completed) {
			return ISIX_ESTATE;
		}
		return coro_.promise().cancel_requested ? ISIX_ECANCELED : ISIX_EOK;
	}

	//! Value produced by a finished task
	[[nodiscard]] auto& value() requires (!std::is_void_v<T>)
	{
		if (!coro_ || !coro_.promise().result_value.has_value()) {
			isix_bug("co::task::value: no value");
		}
		return *coro_.promise().result_value;
	}

	//! Value of a finished task or the reason it has none
	[[nodiscard]] isix::co::result<T> result()
	{
		if (!coro_ || !coro_.promise().completed) {
			return std::unexpected(ISIX_ESTATE);
		}
		if constexpr (std::is_void_v<T>) {
			if (coro_.promise().cancel_requested) {
				return std::unexpected(ISIX_ECANCELED);
			}
			return {};
		} else {
			auto& v = coro_.promise().result_value;
			if (!v.has_value()) {
				return std::unexpected(coro_.promise().cancel_requested ? ISIX_ECANCELED : ISIX_ESTATE);
			}
			return std::move(*v);
		}
	}

	void destroy() noexcept
	{
		if (coro_) {
			coro_.destroy();
			coro_ = {};
		}
	}

	/**
	 * Request cooperative cancellation: the innermost awaited operation completes
	 * with ISIX_ECANCELED and every later suspension returns immediately.
	 * Call from the scheduler thread only.
	 */
	void cancel() noexcept
	{
		if (!coro_) {
			return;
		}
		detail::cancel_promise(coro_.promise());
	}

	//! Awaiter running the task as a child of the awaiting coroutine
	class awaiter final {
	public:
		explicit awaiter(task&& t) noexcept : m_task(std::move(t)) {}
		~awaiter() { release_parent(); }

		[[nodiscard]] bool await_ready() const noexcept { return !m_task.valid(); }

		template<typename P>
		bool await_suspend(std::coroutine_handle<P> parent) noexcept
		{
			auto* const sched = parent.promise().sched;
			if (!sched) {
				return false;
			}
			auto& child = m_task.coro_.promise();
			auto& pp = static_cast<detail::promise_common&>(parent.promise());
			child.cancel_requested = child.cancel_requested || pp.cancel_requested;
			pp.child = &child;
			m_parent = &pp;
			m_node.rdy.h = parent;
			child.continuation = &m_node;
			sched->start(child, m_task.coro_);
			return true;
		}

		auto await_resume()
		{
			release_parent();
			return m_task.get();
		}

	private:
		void release_parent() noexcept
		{
			if (m_parent) {
				m_parent->child = nullptr;
				m_parent = nullptr;
			}
		}

		task m_task;
		detail::promise_common* m_parent {};
		detail::wait_node m_node {};
	};

	[[nodiscard]] awaiter operator co_await() && noexcept { return awaiter{ std::move(*this) }; }

	auto get()
	{
		if (!coro_) {
			std::terminate();
		}
#ifdef ISIX_CO_HAS_EXCEPTIONS
		if (coro_.promise().exception) {
			std::rethrow_exception(coro_.promise().exception);
		}
#endif
		if constexpr (std::is_void_v<T>) {
			return;
		} else {
			if (!coro_.promise().result_value.has_value()) {
				std::terminate();
			}
			return std::move(*coro_.promise().result_value);
		}
	}

private:
	std::coroutine_handle<promise_type> coro_ {};
};

template<typename T>
task<T> task_promise<T>::get_return_object()
{
	return task<T>{ std::coroutine_handle<task_promise<T>>::from_promise(*this) };
}

template<typename T>
task<T> task_promise<T>::get_return_object_on_allocation_failure() noexcept
{
	return task<T>{};
}

inline task<void> task_promise<void>::get_return_object_on_allocation_failure() noexcept
{
	return task<void>{};
}

inline task<void> task_promise<void>::get_return_object()
{
	return task<void>{ std::coroutine_handle<task_promise<void>>::from_promise(*this) };
}

//! Start a task on the scheduler. Returns ISIX_ENOMEM when its frame could not be allocated.
template<typename T>
int spawn(scheduler& sched, task<T>& t, [[maybe_unused]] const char* name = nullptr) noexcept
{
	if (!t.valid()) {
		return ISIX_ENOMEM;
	}
	auto h = t.handle();
#if CONFIG_ISIX_CO_DEBUG
	h.promise().name = name;
#endif
	sched.start(h.promise(), h);
	return ISIX_EOK;
}

template<typename T>
auto run(scheduler& sched, task<T>&& t)
{
	auto h = t.release();
	if (!h) {
		isix_bug("co::run: task has no frame");
	}
	sched.start(h.promise(), h);
	while (!h.promise().completed) {
		if (!sched.step() && !h.promise().completed) {
			isix_bug("co::run: coroutine waits on something outside of isix::co");
		}
	}
#ifdef ISIX_CO_HAS_EXCEPTIONS
	if (h.promise().exception) {
		std::exception_ptr ex = std::move(h.promise().exception);
		h.destroy();
		std::rethrow_exception(ex);
	}
#endif
	if constexpr (std::is_void_v<T>) {
		h.destroy();
		return;
	} else {
		if (!h.promise().result_value.has_value()) {
			h.destroy();
			std::terminate();
		}
		auto v = std::move(*h.promise().result_value);
		h.destroy();
		return v;
	}
}

template<typename T>
auto run(task<T>&& t)
{
	scheduler sched;
	return run(sched, std::move(t));
}

} // namespace isix::co

#include <isix/cpp/co_compose.hpp>

#else

#error "C++20 coroutines are required when CONFIG_ISIX_CPP_COROUTINES is defined"

#endif

#endif // CONFIG_ISIX_CPP_COROUTINES && __cplusplus
