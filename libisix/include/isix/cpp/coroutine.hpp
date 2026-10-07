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

#include <isix/cpp/co_channel.hpp>
#include <isix/cpp/co_event.hpp>
#include <isix/cpp/co_fifo.hpp>
#include <isix/cpp/co_mutex.hpp>
#include <isix/cpp/co_sem_adapter.hpp>
#include <isix/cpp/co_semaphore.hpp>
#include <isix/cpp/co_timer.hpp>
#include <isix/cpp/coroutine_scheduler.hpp>

namespace isix::co {

template<typename T>
class task;

template<typename T>
struct task_promise final : detail::promise_common {
	std::optional<T> result_value {};

	task<T> get_return_object();

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
struct task_promise<void> final : detail::promise_common {
	task<void> get_return_object();

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

	void destroy() noexcept
	{
		if (coro_) {
			coro_.destroy();
			coro_ = {};
		}
	}

	//! Awaiter running the task as a child of the awaiting coroutine
	class awaiter final {
	public:
		explicit awaiter(task&& t) noexcept : m_task(std::move(t)) {}

		[[nodiscard]] bool await_ready() const noexcept { return !m_task.valid(); }

		template<typename P>
		bool await_suspend(std::coroutine_handle<P> parent) noexcept
		{
			auto* const sched = parent.promise().sched;
			if (!sched) {
				return false;
			}
			auto& child = m_task.coro_.promise();
			child.sched = sched;
			child.start_node.h = m_task.coro_;
			m_node.h = parent;
			child.continuation = &m_node;
			sched->enqueue(child.start_node);
			return true;
		}

		auto await_resume()
		{
			return m_task.get();
		}

	private:
		task m_task;
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

inline task<void> task_promise<void>::get_return_object()
{
	return task<void>{ std::coroutine_handle<task_promise<void>>::from_promise(*this) };
}

template<typename T>
void spawn(scheduler& sched, task<T>& t) noexcept
{
	if (!t.valid()) {
		return;
	}
	auto h = t.handle();
	h.promise().sched = &sched;
	h.promise().start_node.h = h;
	sched.enqueue(h.promise().start_node);
}

template<typename T>
auto run(scheduler& sched, task<T>&& t)
{
	auto h = t.release();
	if (!h) {
		if constexpr (std::is_void_v<T>) {
			return;
		} else {
			std::terminate();
		}
	}
	h.promise().sched = &sched;
	h.promise().start_node.h = h;
	sched.enqueue(h.promise().start_node);
	while (!h.promise().completed) {
		sched.step();
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

#else

#error "C++20 coroutines are required when CONFIG_ISIX_CPP_COROUTINES is defined"

#endif

#endif // CONFIG_ISIX_CPP_COROUTINES && __cplusplus
