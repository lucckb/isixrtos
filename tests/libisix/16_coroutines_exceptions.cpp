/*
 * Exceptions in isix::co: every construct either delivers the exception to the
 * awaiting coroutine or hands it to the unhandled exception handler.
 *
 * Author: Lucjan Bryndza
 */

/* Enable isix::co via isix.h only in this translation unit (keeps other tests lean). */
#define CONFIG_ISIX_CPP_COROUTINES 1

#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <array>
#include <atomic>
#include <cstddef>
#include <exception>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

#if CONFIG_ISIX_CPP_COROUTINES

namespace {

[[maybe_unused]] constexpr auto c_stack_size = 2048U;
[[maybe_unused]] constexpr auto c_task_prio = 3;

using isix::co::detail::c_co_nothrow;

//! Awaiters and accessors that rethrow are noexcept only when exceptions are off
using void_task = isix::co::task<void>;
using int_task = isix::co::task<int>;
using join_all_t = decltype(isix::co::when_all(std::declval<void_task&>(), std::declval<void_task&>()));
using join_any_t = decltype(isix::co::when_any(std::declval<void_task&>(), std::declval<int_task&>()));
using timeout_t = decltype(isix::co::with_timeout(std::declval<int_task&>(), 1));
using gen_t = isix::co::generator<int>;
using scope_join_t = decltype(std::declval<isix::co::async_scope&>().join());
using job_t = void (*)();
using run_t = decltype(std::declval<isix::co::worker_thread&>().run(std::declval<job_t&>()));

static_assert(noexcept(std::declval<void_task&>().result()) == c_co_nothrow);
static_assert(noexcept(std::declval<int_task&>().result()) == c_co_nothrow);
static_assert(noexcept(std::declval<join_all_t&>().await_resume()) == c_co_nothrow);
static_assert(noexcept(std::declval<join_any_t&>().await_resume()) == c_co_nothrow);
static_assert(noexcept(std::declval<timeout_t&>().await_resume()) == c_co_nothrow);
static_assert(noexcept(std::declval<scope_join_t&>().await_resume()) == c_co_nothrow);
static_assert(noexcept(std::declval<run_t&>().await_resume()) == c_co_nothrow);
static_assert(noexcept(std::declval<gen_t&>().begin()) == c_co_nothrow);
static_assert(noexcept(++std::declval<gen_t::iterator&>()) == c_co_nothrow);
static_assert(noexcept(std::declval<int_task&>().error()));
static_assert(noexcept(std::declval<int_task&>().has_exception()));
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
static_assert(!c_co_nothrow);
static_assert(!noexcept(std::declval<join_all_t&>().await_resume()));
#else
static_assert(c_co_nothrow);
static_assert(noexcept(std::declval<join_all_t&>().await_resume()));
static_assert(std::is_empty_v<isix::co::detail::exception_slot>);
#endif

//! Step the scheduler until pred() holds or the time limit elapses
template<typename F>
[[maybe_unused]] bool drive(isix::co::scheduler& sched, F&& pred, unsigned limit_ms = 2000U)
{
	const auto t0 = isix::get_jiffies();
	while (!pred()) {
		if (isix::timer_elapsed(t0, isix::ms2tick(limit_ms))) {
			return false;
		}
		sched.step();
	}
	return true;
}

} // namespace

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)

namespace {

struct boom {
	int code;
};

//! Counts live and destroyed objects to prove that frames and handlers are unwound
struct probe {
	static inline int alive {};
	static inline int destroyed {};
	probe() noexcept { ++alive; }
	probe(const probe&) = delete;
	probe& operator=(const probe&) = delete;
	~probe()
	{
		--alive;
		++destroyed;
	}
};

//! Exceptions delivered to the handler; no assertions are allowed in it
struct handler_log {
	static constexpr std::size_t c_max = 4;
	static inline std::array<std::exception_ptr, c_max> seen {};
	static inline int count {};

	static void reset() noexcept
	{
		for (auto& e : seen) {
			e = nullptr;
		}
		count = 0;
	}
	static void handle(std::exception_ptr ex) noexcept
	{
		if (static_cast<std::size_t>(count) < c_max) {
			seen[static_cast<std::size_t>(count)] = std::move(ex);
		}
		++count;
	}
};

int code_of(const std::exception_ptr& ep)
{
	try {
		std::rethrow_exception(ep);
	} catch (const boom& b) {
		return b.code;
	} catch (...) {
		return -1;
	}
}

//! Frame source that always fails
void* refuse_alloc(void*, std::size_t) noexcept { return nullptr; }
void refuse_free(void*, void*, std::size_t) noexcept {}

//! Task that throws after the given delay
isix::co::task<void> throw_after(unsigned ms, int code)
{
	co_await isix::co::sleep_ms(ms);
	throw boom { code };
}

//! Task that waits for the event and throws when it was canceled
isix::co::task<void> throw_when_canceled(isix::co::event& ev, int code, int& rc)
{
	rc = co_await ev.wait_for(isix::ms2tick(1000));
	if (rc == ISIX_ECANCELED) {
		throw boom { code };
	}
}

isix::co::task<void> sleep_task(unsigned ms, int& rc)
{
	co_await isix::co::sleep_ms(ms);
	rc = co_await isix::co::cancelled() ? ISIX_ECANCELED : ISIX_EOK;
}

} // namespace

TEST_GROUP(coroutines_exceptions);

TEST_SETUP(coroutines_exceptions)
{
	probe::alive = 0;
	probe::destroyed = 0;
	handler_log::reset();
	isix::co::set_unhandled_exception_handler(&handler_log::handle);
}

TEST_TEAR_DOWN(coroutines_exceptions)
{
	isix::co::set_unhandled_exception_handler(nullptr);
	isix::co::reset_frame_allocator();
	handler_log::reset();
}

TEST(coroutines_exceptions, frame_unwound_on_exception)
{
	bool caught { false };
	int code { 0 };
	auto thrower = []() -> int_task {
		probe p;
		co_await isix::co::sleep_ms(2);
		throw boom { 11 };
		co_return 0;
	};
	try {
		isix::co::run(thrower());
	} catch (const boom& b) {
		caught = true;
		code = b.code;
	}
	TEST_ASSERT_TRUE(caught);
	TEST_ASSERT_EQUAL(11, code);
	TEST_ASSERT_EQUAL(0, probe::alive);
	TEST_ASSERT_EQUAL(1, probe::destroyed);
}

TEST(coroutines_exceptions, exception_travels_through_await_chain)
{
	auto leaf = []() -> int_task {
		probe p;
		co_await isix::co::sleep_ms(2);
		throw boom { 3 };
		co_return 0;
	};
	auto mid = [&leaf]() -> int_task {
		probe p;
		co_return (co_await leaf()) + 1;
	};
	auto top = [&mid]() -> int_task {
		int r { 0 };
		try {
			r = co_await mid();
		} catch (const boom& b) {
			r = -b.code;
		}
		co_return r;
	};
	TEST_ASSERT_EQUAL(-3, isix::co::run(top()));
	TEST_ASSERT_EQUAL(0, probe::alive);
	TEST_ASSERT_EQUAL(2, probe::destroyed);
}

TEST(coroutines_exceptions, result_rethrows)
{
	isix::co::scheduler sched;
	auto tv = throw_after(2, 21);
	auto ti = []() -> int_task {
		co_await isix::co::sleep_ms(2);
		throw boom { 22 };
		co_return 0;
	}();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, tv));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, ti));
	TEST_ASSERT_TRUE(drive(sched, [&] { return tv.done() && ti.done(); }));
	int void_code { 0 };
	int int_code { 0 };
	try {
		std::ignore = tv.result();
	} catch (const boom& b) {
		void_code = b.code;
	}
	try {
		std::ignore = ti.result();
	} catch (const boom& b) {
		int_code = b.code;
	}
	TEST_ASSERT_EQUAL(21, void_code);
	TEST_ASSERT_EQUAL(22, int_code);
}

TEST(coroutines_exceptions, error_not_ok_on_exception)
{
	isix::co::scheduler sched;
	int rc { ISIX_ESTATE };
	auto bad = throw_after(2, 5);
	auto good = sleep_task(2, rc);
	TEST_ASSERT_FALSE(bad.has_exception());
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, bad));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, good));
	TEST_ASSERT_TRUE(drive(sched, [&] { return bad.done() && good.done(); }));
	TEST_ASSERT_TRUE(bad.has_exception());
	TEST_ASSERT_EQUAL(ISIX_EBADF, bad.error());
	TEST_ASSERT_FALSE(good.has_exception());
	TEST_ASSERT_EQUAL(ISIX_EOK, good.error());
	TEST_ASSERT_TRUE(good.result().has_value());
}

TEST(coroutines_exceptions, cancel_then_throw_exception_wins)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc { ISIX_ESTATE };
	auto t = throw_when_canceled(ev, 8, rc);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	t.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&t] { return t.done(); }));
	int code { 0 };
	try {
		std::ignore = t.result();
	} catch (const boom& b) {
		code = b.code;
	}
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc);
	TEST_ASSERT_EQUAL(8, code);
	TEST_ASSERT_EQUAL(ISIX_EBADF, t.error());
}

TEST(coroutines_exceptions, with_timeout_rethrows_before_deadline)
{
	isix::co::scheduler sched;
	int code { 0 };
	bool resumed_normally { false };
	auto parent = [&]() -> void_task {
		auto child = throw_after(5, 31);
		try {
			std::ignore = co_await isix::co::with_timeout(child, isix::ms2tick(500));
			resumed_normally = true;
		} catch (const boom& b) {
			code = b.code;
		}
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_FALSE(resumed_normally);
	TEST_ASSERT_EQUAL(31, code);
}

TEST(coroutines_exceptions, with_timeout_throw_on_cancel_wins_over_timeout)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int code { 0 };
	int child_rc { ISIX_ESTATE };
	bool resumed_normally { false };
	auto parent = [&]() -> void_task {
		auto child = throw_when_canceled(ev, 32, child_rc);
		try {
			std::ignore = co_await isix::co::with_timeout(child, isix::ms2tick(20));
			resumed_normally = true;
		} catch (const boom& b) {
			code = b.code;
		}
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_FALSE(resumed_normally);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, child_rc);
	TEST_ASSERT_EQUAL(32, code);
}

TEST(coroutines_exceptions, when_all_rethrows_first_and_cancels)
{
	isix::co::scheduler sched;
	int code { 0 };
	int slow_rc { ISIX_ESTATE };
	int a_err { ISIX_ESTATE };
	int b_err { ISIX_ESTATE };
	osutick_t elapsed {};
	auto parent = [&]() -> void_task {
		auto a = throw_after(5, 41);
		auto b = sleep_task(500, slow_rc);
		const auto u0 = isix::get_ujiffies();
		try {
			co_await isix::co::when_all(a, b);
		} catch (const boom& e) {
			code = e.code;
		}
		elapsed = isix::get_ujiffies() - u0;
		a_err = a.error();
		b_err = b.error();
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(41, code);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, slow_rc);
	TEST_ASSERT_EQUAL(ISIX_EBADF, a_err);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, b_err);
	TEST_ASSERT_TRUE(elapsed < 200ULL * 1000ULL);
}

TEST(coroutines_exceptions, when_all_second_exception_kept_in_task)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int code { 0 };
	int rc { ISIX_ESTATE };
	bool second_kept { false };
	int second_code { 0 };
	auto parent = [&]() -> void_task {
		auto a = throw_after(5, 42);
		auto b = throw_when_canceled(ev, 43, rc);
		try {
			co_await isix::co::when_all(a, b);
		} catch (const boom& e) {
			code = e.code;
		}
		second_kept = b.has_exception();
		try {
			std::ignore = b.result();
		} catch (const boom& e) {
			second_code = e.code;
		}
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(42, code);
	TEST_ASSERT_TRUE(second_kept);
	TEST_ASSERT_EQUAL(43, second_code);
}

TEST(coroutines_exceptions, when_all_no_exception_regression)
{
	isix::co::scheduler sched;
	int sum { 0 };
	bool threw { false };
	auto make = [](int v) -> int_task {
		co_await isix::co::sleep_ms(3);
		co_return v;
	};
	auto parent = [&]() -> void_task {
		auto a = make(3);
		auto b = make(4);
		try {
			co_await isix::co::when_all(a, b);
			sum = a.value() + b.value();
		} catch (...) {
			threw = true;
		}
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_FALSE(threw);
	TEST_ASSERT_EQUAL(7, sum);
}

TEST(coroutines_exceptions, when_all_precompleted_exception)
{
	isix::co::scheduler sched;
	auto a = []() -> void_task {
		throw boom { 44 };
		co_return;
	}();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, a));
	TEST_ASSERT_TRUE(drive(sched, [&a] { return a.done(); }));
	int code { 0 };
	int rc { ISIX_ESTATE };
	auto parent = [&]() -> void_task {
		auto b = sleep_task(5, rc);
		try {
			co_await isix::co::when_all(a, b);
		} catch (const boom& e) {
			code = e.code;
		}
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(44, code);
	TEST_ASSERT_EQUAL(ISIX_EOK, rc);
}

TEST(coroutines_exceptions, when_any_exception_winner_rethrows)
{
	isix::co::scheduler sched;
	int code { 0 };
	int slow_rc { ISIX_ESTATE };
	bool resumed_normally { false };
	auto parent = [&]() -> void_task {
		auto a = throw_after(3, 51);
		auto b = sleep_task(500, slow_rc);
		try {
			std::ignore = co_await isix::co::when_any(a, b);
			resumed_normally = true;
		} catch (const boom& e) {
			code = e.code;
		}
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_FALSE(resumed_normally);
	TEST_ASSERT_EQUAL(51, code);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, slow_rc);
}

TEST(coroutines_exceptions, when_any_loser_exception_kept_in_task)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	std::size_t winner { 99 };
	bool threw { false };
	bool loser_kept { false };
	int rc { ISIX_ESTATE };
	int winner_rc { ISIX_ESTATE };
	auto parent = [&]() -> void_task {
		auto a = sleep_task(3, winner_rc);
		auto b = throw_when_canceled(ev, 52, rc);
		try {
			winner = co_await isix::co::when_any(a, b);
		} catch (...) {
			threw = true;
		}
		loser_kept = b.has_exception();
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_FALSE(threw);
	TEST_ASSERT_EQUAL_UINT32(0, winner);
	TEST_ASSERT_TRUE(loser_kept);
}

TEST(coroutines_exceptions, scope_join_rethrows_first)
{
	isix::co::scheduler sched;
	isix::co::async_scope scope(sched);
	int code { 0 };
	bool second_join_threw { false };
	auto parent = [&]() -> void_task {
		try {
			co_await scope.join();
		} catch (const boom& e) {
			code = e.code;
		}
		try {
			co_await scope.join();
		} catch (...) {
			second_join_threw = true;
		}
	};
	TEST_ASSERT_EQUAL(ISIX_EOK, scope.spawn(throw_after(2, 61)));
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(61, code);
	TEST_ASSERT_FALSE(second_join_threw);
	TEST_ASSERT_EQUAL(0, handler_log::count);
}

TEST(coroutines_exceptions, scope_second_exception_to_handler)
{
	isix::co::scheduler sched;
	isix::co::async_scope scope(sched);
	int code { 0 };
	auto parent = [&]() -> void_task {
		try {
			co_await scope.join();
		} catch (const boom& e) {
			code = e.code;
		}
	};
	TEST_ASSERT_EQUAL(ISIX_EOK, scope.spawn(throw_after(2, 62)));
	TEST_ASSERT_EQUAL(ISIX_EOK, scope.spawn(throw_after(6, 63)));
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(62, code);
	TEST_ASSERT_EQUAL(1, handler_log::count);
	TEST_ASSERT_EQUAL(63, code_of(handler_log::seen[0]));
}

TEST(coroutines_exceptions, scope_destroy_unobserved_to_handler)
{
	isix::co::scheduler sched;
	{
		isix::co::async_scope scope(sched);
		TEST_ASSERT_EQUAL(ISIX_EOK, scope.spawn(throw_after(2, 64)));
		TEST_ASSERT_TRUE(drive(sched, [&scope] { return scope.size() == 0; }));
		TEST_ASSERT_EQUAL(0, handler_log::count);
	}
	TEST_ASSERT_EQUAL(1, handler_log::count);
	TEST_ASSERT_EQUAL(64, code_of(handler_log::seen[0]));
}

TEST(coroutines_exceptions, post_exception_to_handler)
{
	isix::co::scheduler sched;
	TEST_ASSERT_EQUAL(ISIX_EOK, sched.post(throw_after(2, 65)));
	TEST_ASSERT_TRUE(drive(sched, [&sched] { return handler_log::count > 0 && sched.detached_count() == 0; }));
	TEST_ASSERT_EQUAL(1, handler_log::count);
	TEST_ASSERT_EQUAL(65, code_of(handler_log::seen[0]));
}

TEST(coroutines_exceptions, generator_rethrows_to_consumer)
{
	auto gen = []() -> isix::co::generator<int> {
		probe p;
		co_yield 1;
		co_yield 2;
		throw boom { 71 };
	};
	int sum { 0 };
	int code { 0 };
	{
		try {
			for (const auto v : gen()) {
				sum += v;
			}
		} catch (const boom& b) {
			code = b.code;
		}
	}
	TEST_ASSERT_EQUAL(3, sum);
	TEST_ASSERT_EQUAL(71, code);
	TEST_ASSERT_EQUAL(0, probe::alive);
	TEST_ASSERT_EQUAL(1, probe::destroyed);
}

TEST(coroutines_exceptions, generator_throws_before_first_yield)
{
	auto gen = []() -> isix::co::generator<int> {
		probe p;
		throw boom { 72 };
		co_yield 1;
	};
	int items { 0 };
	int code { 0 };
	try {
		for (const auto v : gen()) {
			items += v;
		}
	} catch (const boom& b) {
		code = b.code;
	}
	TEST_ASSERT_EQUAL(0, items);
	TEST_ASSERT_EQUAL(72, code);
	TEST_ASSERT_EQUAL(0, probe::alive);
}

TEST(coroutines_exceptions, worker_rethrows_in_coroutine)
{
	isix::co::scheduler sched;
	isix::co::worker_thread worker(sched);
	TEST_ASSERT_EQUAL(ISIX_EOK, worker.start(c_stack_size * 2, c_task_prio));
	int code { 0 };
	bool resumed_normally { false };
	auto job = []() -> void {
		isix::wait_ms(5);
		throw boom { 81 };
	};
	auto parent = [&]() -> void_task {
		try {
			std::ignore = co_await worker.run(job);
			resumed_normally = true;
		} catch (const boom& e) {
			code = e.code;
		}
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_FALSE(resumed_normally);
	TEST_ASSERT_EQUAL(81, code);
}

TEST(coroutines_exceptions, worker_usable_after_exception)
{
	isix::co::scheduler sched;
	isix::co::worker_thread worker(sched);
	TEST_ASSERT_EQUAL(ISIX_EOK, worker.start(c_stack_size * 2, c_task_prio));
	int code { 0 };
	int rc { ISIX_ESTATE };
	bool second_threw { false };
	bool second_ran { false };
	auto bad = []() -> void { throw boom { 82 }; };
	auto good = [&second_ran]() -> void { second_ran = true; };
	auto parent = [&]() -> void_task {
		try {
			std::ignore = co_await worker.run(bad);
		} catch (const boom& e) {
			code = e.code;
		}
		try {
			rc = co_await worker.run(good);
		} catch (...) {
			second_threw = true;
		}
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(82, code);
	TEST_ASSERT_FALSE(second_threw);
	TEST_ASSERT_TRUE(second_ran);
	TEST_ASSERT_EQUAL(ISIX_EOK, rc);
}

TEST(coroutines_exceptions, worker_cancel_then_throw_exception_wins)
{
	isix::co::scheduler sched;
	isix::co::worker_thread worker(sched);
	TEST_ASSERT_EQUAL(ISIX_EOK, worker.start(c_stack_size * 2, c_task_prio));
	int code { 0 };
	bool resumed_normally { false };
	bool done { false };
	auto job = []() -> void {
		isix::wait_ms(40);
		throw boom { 83 };
	};
	auto parent = [&]() -> void_task {
		try {
			std::ignore = co_await worker.run(job);
			resumed_normally = true;
		} catch (const boom& e) {
			code = e.code;
		}
		done = true;
	};
	auto t = parent();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	isix::wait_ms(5);
	t.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_FALSE(resumed_normally);
	TEST_ASSERT_EQUAL(83, code);
}

TEST(coroutines_exceptions, worker_thread_and_scheduler_throw_concurrently)
{
	constexpr int c_rounds = 50;
	struct worker_ex {
		int v;
	};
	isix::co::scheduler sched;
	isix::co::worker_thread worker(sched);
	isix::co::async_scope scope(sched);
	TEST_ASSERT_EQUAL(ISIX_EOK, worker.start(c_stack_size * 2, c_task_prio));
	std::atomic<int> worker_ok { 0 };
	std::atomic<int> worker_bad { 0 };
	int sched_ok { 0 };
	int sched_bad { 0 };
	int final_code { 0 };
	auto job = [&]() -> void {
		for (int i = 0; i < c_rounds; ++i) {
			int got { -1 };
			try {
				throw worker_ex { i };
			} catch (const worker_ex& e) {
				isix::wait_ms(1);
				got = e.v;
			}
			(got == i ? worker_ok : worker_bad)++;
		}
		throw boom { 91 };
	};
	auto thrower = [&]() -> void_task {
		for (int i = 0; i < c_rounds; ++i) {
			int got { -1 };
			try {
				throw boom { i };
			} catch (const boom& b) {
				isix::wait_ms(1);
				got = b.code;
			}
			++(got == i ? sched_ok : sched_bad);
			co_await isix::co::yield();
		}
	};
	auto parent = [&]() -> void_task {
		try {
			std::ignore = co_await worker.run(job);
		} catch (const boom& e) {
			final_code = e.code;
		}
		co_await scope.join();
	};
	TEST_ASSERT_EQUAL(ISIX_EOK, scope.spawn(thrower()));
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(c_rounds, worker_ok.load());
	TEST_ASSERT_EQUAL(0, worker_bad.load());
	TEST_ASSERT_EQUAL(c_rounds, sched_ok);
	TEST_ASSERT_EQUAL(0, sched_bad);
	TEST_ASSERT_EQUAL(91, final_code);
}

TEST(coroutines_exceptions, empty_task_await_throws_bad_alloc)
{
	isix::co::set_frame_allocator({ &refuse_alloc, &refuse_free, nullptr });
	auto leaf = []() -> int_task { co_return 1; };
	auto empty = leaf();
	isix::co::reset_frame_allocator();
	TEST_ASSERT_FALSE(empty.valid());
	bool caught_alloc { false };
	bool get_threw { false };
	auto parent = [&]() -> void_task {
		try {
			std::ignore = co_await std::move(empty);
		} catch (const std::bad_alloc&) {
			caught_alloc = true;
		}
	};
	isix::co::run(parent());
	TEST_ASSERT_TRUE(caught_alloc);
	auto empty2 = []() -> int_task { co_return 2; };
	isix::co::set_frame_allocator({ &refuse_alloc, &refuse_free, nullptr });
	auto e2 = empty2();
	isix::co::reset_frame_allocator();
	try {
		std::ignore = e2.get();
	} catch (const std::bad_alloc&) {
		get_threw = true;
	}
	TEST_ASSERT_TRUE(get_threw);
}

TEST_GROUP_RUNNER(coroutines_exceptions)
{
	RUN_TEST_CASE(coroutines_exceptions, frame_unwound_on_exception);
	RUN_TEST_CASE(coroutines_exceptions, exception_travels_through_await_chain);
	RUN_TEST_CASE(coroutines_exceptions, result_rethrows);
	RUN_TEST_CASE(coroutines_exceptions, error_not_ok_on_exception);
	RUN_TEST_CASE(coroutines_exceptions, cancel_then_throw_exception_wins);
	RUN_TEST_CASE(coroutines_exceptions, with_timeout_rethrows_before_deadline);
	RUN_TEST_CASE(coroutines_exceptions, with_timeout_throw_on_cancel_wins_over_timeout);
	RUN_TEST_CASE(coroutines_exceptions, when_all_rethrows_first_and_cancels);
	RUN_TEST_CASE(coroutines_exceptions, when_all_second_exception_kept_in_task);
	RUN_TEST_CASE(coroutines_exceptions, when_all_no_exception_regression);
	RUN_TEST_CASE(coroutines_exceptions, when_all_precompleted_exception);
	RUN_TEST_CASE(coroutines_exceptions, when_any_exception_winner_rethrows);
	RUN_TEST_CASE(coroutines_exceptions, when_any_loser_exception_kept_in_task);
	RUN_TEST_CASE(coroutines_exceptions, scope_join_rethrows_first);
	RUN_TEST_CASE(coroutines_exceptions, scope_second_exception_to_handler);
	RUN_TEST_CASE(coroutines_exceptions, scope_destroy_unobserved_to_handler);
	RUN_TEST_CASE(coroutines_exceptions, post_exception_to_handler);
	RUN_TEST_CASE(coroutines_exceptions, generator_rethrows_to_consumer);
	RUN_TEST_CASE(coroutines_exceptions, generator_throws_before_first_yield);
	RUN_TEST_CASE(coroutines_exceptions, worker_rethrows_in_coroutine);
	RUN_TEST_CASE(coroutines_exceptions, worker_usable_after_exception);
	RUN_TEST_CASE(coroutines_exceptions, worker_cancel_then_throw_exception_wins);
	RUN_TEST_CASE(coroutines_exceptions, worker_thread_and_scheduler_throw_concurrently);
	RUN_TEST_CASE(coroutines_exceptions, empty_task_await_throws_bad_alloc);
}

#else

TEST_GROUP_RUNNER(coroutines_exceptions) {}

#endif // exceptions

#else // !CONFIG_ISIX_CPP_COROUTINES

TEST_GROUP_RUNNER(coroutines_exceptions) {}

#endif
