/*
 * Exception policy of isix::co: the only place that depends on whether the
 * build has C++ exceptions. Without them every helper is empty or constexpr.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <exception>
#include <new>
#include <type_traits>
#include <utility>

#include <isix/error.h>

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
#define ISIX_CO_HAS_EXCEPTIONS 1
#endif

namespace isix::co {

#ifdef ISIX_CO_HAS_EXCEPTIONS

//! Called for an exception of a fire and forget coroutine that nobody can receive
using unhandled_exception_handler = void (*)(std::exception_ptr) noexcept;

namespace detail {
inline unhandled_exception_handler g_unhandled_handler {};
} // namespace detail

/**
 * Set the handler for exceptions leaving fire and forget coroutines. It runs in the
 * scheduler thread, must not throw and must not destroy the scheduler or the scope.
 * nullptr restores the default, std::terminate. Returns the previous handler.
 */
inline unhandled_exception_handler set_unhandled_exception_handler(unhandled_exception_handler h) noexcept
{
	return std::exchange(detail::g_unhandled_handler, h);
}

#endif

namespace detail {

#ifdef ISIX_CO_HAS_EXCEPTIONS

//! Awaiters that rethrow are not noexcept when exceptions are enabled
inline constexpr bool c_co_nothrow = false;

using stored_exception = std::exception_ptr;

inline void rethrow(stored_exception ex) { std::rethrow_exception(std::move(ex)); }

//! Small state that exists only when exceptions are enabled
template<typename T, T Init>
class eh_state {
public:
	[[nodiscard]] T get() const noexcept { return m_v; }
	void set(T v) noexcept { m_v = v; }

private:
	T m_v { Init };
};

//! Holds an exception raised in a coroutine until somebody can receive it
class exception_slot {
public:
	void capture() noexcept { m_ex = std::current_exception(); }
	[[nodiscard]] bool has() const noexcept { return static_cast<bool>(m_ex); }

	//! Rethrow the stored exception and keep it stored
	void rethrow_if() const
	{
		if (m_ex) {
			std::rethrow_exception(m_ex);
		}
	}

	//! Rethrow the stored exception and clear the slot
	void take_and_rethrow()
	{
		if (m_ex) {
			std::rethrow_exception(std::exchange(m_ex, {}));
		}
	}

	[[nodiscard]] stored_exception take() noexcept { return std::exchange(m_ex, {}); }
	void put(stored_exception ex) noexcept { m_ex = std::move(ex); }
	void clear() noexcept { m_ex = nullptr; }

private:
	std::exception_ptr m_ex {};
};

//! Hand an exception nobody can receive to the handler, terminate by default
inline void report_unhandled(std::exception_ptr ex) noexcept
{
	if (g_unhandled_handler) {
		g_unhandled_handler(std::move(ex));
	} else {
		std::terminate();
	}
}

//! Receives exceptions of fire and forget coroutines: the first one is kept for the owner
class exception_sink {
public:
	void collect() noexcept { m_collect = true; }

	//! Take the exception from the finished coroutine
	void deliver(exception_slot& src) noexcept
	{
		if (!src.has()) {
			return;
		}
		auto ex = src.take();
		if (m_collect && !m_first.has()) {
			m_first.put(std::move(ex));
		} else {
			report_unhandled(std::move(ex));
		}
	}

	void rethrow_pending() { m_first.take_and_rethrow(); }

	//! Pass an exception nobody received to the handler
	void report_pending() noexcept
	{
		if (m_first.has()) {
			report_unhandled(m_first.take());
		}
	}

private:
	exception_slot m_first {};
	bool m_collect { false };
};

//! Empty task or generator was used as if it had a result
[[noreturn]] inline void fail_empty_task() { throw std::bad_alloc(); }

//! Call the callable and keep an exception thrown by it in the slot
template<typename F>
int invoke_capturing(F& f, exception_slot& s) noexcept
{
	try {
		if constexpr (std::is_void_v<std::invoke_result_t<F&>>) {
			f();
			return ISIX_EOK;
		} else {
			return static_cast<int>(f());
		}
	} catch (...) {
		s.capture();
		return ISIX_EOK;
	}
}

#else

inline constexpr bool c_co_nothrow = true;

struct stored_exception {};

inline void rethrow(stored_exception) noexcept {}

template<typename T, T Init>
class eh_state {
public:
	[[nodiscard]] static constexpr T get() noexcept { return Init; }
	static constexpr void set(T) noexcept {}
};

//! Without exceptions nothing is ever stored
class exception_slot {
public:
	[[noreturn]] void capture() noexcept { std::terminate(); }
	[[nodiscard]] static constexpr bool has() noexcept { return false; }
	static constexpr void rethrow_if() noexcept {}
	static constexpr void take_and_rethrow() noexcept {}
	[[nodiscard]] static constexpr stored_exception take() noexcept { return {}; }
	static constexpr void clear() noexcept {}
};

//! Nothing to deliver without exceptions
class exception_sink {
public:
	static constexpr void collect() noexcept {}
	static constexpr void deliver(exception_slot&) noexcept {}
	static constexpr void rethrow_pending() noexcept {}
	static constexpr void report_pending() noexcept {}
};

[[noreturn]] inline void fail_empty_task() noexcept { std::terminate(); }

template<typename F>
[[gnu::always_inline]] inline int invoke_capturing(F& f, exception_slot&) noexcept
{
	if constexpr (std::is_void_v<std::invoke_result_t<F&>>) {
		f();
		return ISIX_EOK;
	} else {
		return static_cast<int>(f());
	}
}

#endif

} // namespace detail

} // namespace isix::co

#endif
