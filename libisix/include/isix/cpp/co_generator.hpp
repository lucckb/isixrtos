/*
 * Synchronous generator: a coroutine producing a sequence with co_yield,
 * consumed with a range-for loop. It needs no scheduler.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <coroutine>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include <isix/cpp/co_frame.hpp>

namespace isix::co {

template<typename T>
class generator final {
public:
	struct promise_type {
		std::add_pointer_t<std::remove_reference_t<T>> current {};
		bool completed { false };

		static void* operator new(std::size_t size) noexcept { return detail::frame_acquire(size); }
		static void operator delete(void* frame, std::size_t size) noexcept
		{
			detail::frame_release(frame, size);
		}

		generator get_return_object() noexcept
		{
			return generator{ std::coroutine_handle<promise_type>::from_promise(*this) };
		}
		static generator get_return_object_on_allocation_failure() noexcept { return generator{}; }

		std::suspend_always initial_suspend() noexcept { return {}; }
		std::suspend_always final_suspend() noexcept
		{
			completed = true;
			return {};
		}

		std::suspend_always yield_value(std::remove_reference_t<T>& v) noexcept
		{
			current = std::addressof(v);
			return {};
		}
		std::suspend_always yield_value(std::remove_reference_t<T>&& v) noexcept
		{
			current = std::addressof(v);
			return {};
		}

		void return_void() noexcept {}
		void unhandled_exception() noexcept { std::terminate(); }

		//! co_await is not allowed in a generator
		template<typename U>
		std::suspend_never await_transform(U&&) = delete;
	};

	class sentinel final {};

	class iterator final {
	public:
		using value_type = std::remove_cvref_t<T>;
		using difference_type = std::ptrdiff_t;

		iterator() noexcept = default;
		explicit iterator(std::coroutine_handle<promise_type> h) noexcept : m_h(h) {}

		iterator& operator++() noexcept
		{
			m_h.resume();
			return *this;
		}
		void operator++(int) noexcept { ++*this; }

		[[nodiscard]] std::add_lvalue_reference_t<T> operator*() const noexcept { return *m_h.promise().current; }
		[[nodiscard]] bool operator==(sentinel) const noexcept { return !m_h || m_h.promise().completed; }

	private:
		std::coroutine_handle<promise_type> m_h {};
	};

	generator() noexcept = default;
	generator(generator&& o) noexcept : m_h(std::exchange(o.m_h, {})) {}
	generator& operator=(generator&& o) noexcept
	{
		if (this != &o) {
			destroy();
			m_h = std::exchange(o.m_h, {});
		}
		return *this;
	}
	generator(const generator&) = delete;
	generator& operator=(const generator&) = delete;
	~generator() { destroy(); }

	//! False when the frame could not be allocated
	[[nodiscard]] bool valid() const noexcept { return static_cast<bool>(m_h); }

	//! Runs the generator to its first value; an empty generator yields an empty range
	[[nodiscard]] iterator begin() noexcept
	{
		if (m_h) {
			m_h.resume();
		}
		return iterator{ m_h };
	}

	[[nodiscard]] sentinel end() const noexcept { return {}; }

private:
	explicit generator(std::coroutine_handle<promise_type> h) noexcept : m_h(h) {}

	void destroy() noexcept
	{
		if (m_h) {
			m_h.destroy();
			m_h = {};
		}
	}

	std::coroutine_handle<promise_type> m_h {};
};

} // namespace isix::co

#endif
