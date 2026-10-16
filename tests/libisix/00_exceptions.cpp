#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <atomic>
#include <cxxabi.h>
#include <exception>
#include <new>
#include <utils/test_prio.hpp>

TEST_SETUP(exceptions) {}
TEST_TEAR_DOWN(exceptions) {}

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
TEST_GROUP(exceptions);

TEST(exceptions, basic)
{
	try {
		throw std::bad_alloc();
		TEST_ASSERT(false);
	} catch (const std::bad_alloc& e) {
		TEST_ASSERT(true);
	} catch (const std::exception& e) {
		TEST_ASSERT(false);
	} catch (...) {
		TEST_ASSERT(false);
	}
}

TEST(exceptions, rethrow)
{
	auto rethrow = []() {
		try {
			throw std::bad_alloc();
			TEST_ASSERT(false);
		} catch (...) {
			throw;
		}
		TEST_ASSERT(false);
	};

	try {
		rethrow();
		TEST_ASSERT(false);
	} catch (const std::exception& e) {
		TEST_ASSERT(true);
	} catch (...) {
		TEST_ASSERT(false);
	}
}

namespace {

struct ex_a { int v; };
struct ex_b { int v; };

constexpr size_t c_ex_stack = 6144;
constexpr osprio_t c_prio_low = 3;
constexpr osprio_t c_prio_high = 2;

[[gnu::noinline]] void* current_eh_globals() noexcept
{
	return static_cast<void*>(abi::__cxa_get_globals());
}

ostick_t ex_wait() noexcept { return test_utils::ms_ticks(10000); }

struct globals_ctx {
	std::atomic<void*> first {};
	std::atomic<void*> second {};
	isix::semaphore done { 0, 1 };
};

void globals_task(void* arg)
{
	auto& c = *static_cast<globals_ctx*>(arg);
	c.first = current_eh_globals();
	isix::wait_ms(3);
	c.second = current_eh_globals();
	c.done.signal();
}

//! Two tasks hold an open catch block at the same time
struct preempt_ctx {
	std::atomic<int> value { -1 };
	std::atomic<bool> has_current { false };
	std::atomic<bool> wrong_type { false };
	std::atomic<int> uncaught { -1 };
	isix::semaphore in_catch { 0, 1 };
	isix::semaphore go { 0, 1 };
	isix::semaphore done { 0, 1 };
};

template<typename E>
void preempt_task(void* arg)
{
	auto& c = *static_cast<preempt_ctx*>(arg);
	try {
		throw E { static_cast<int>(std::is_same_v<E, ex_a> ? 111 : 222) };
	} catch (const E&) {
		c.in_catch.signal();
		c.go.wait(ISIX_TIME_INFINITE);
		c.has_current = static_cast<bool>(std::current_exception());
		try {
			throw;
		} catch (const E& e) {
			c.value = e.v;
		} catch (...) {
			c.wrong_type = true;
		}
		c.uncaught = std::uncaught_exceptions();
	}
	c.done.signal();
}

//! Task A rethrows twice inside nested handlers while B throws in between
struct nested_ctx {
	std::atomic<int> value { -1 };
	std::atomic<bool> wrong_type { false };
	std::atomic<int> b_value { -1 };
	isix::semaphore in_catch { 0, 1 };
	isix::semaphore go { 0, 1 };
	isix::semaphore done { 0, 1 };
	isix::semaphore b_done { 0, 1 };
};

void nested_task_a(void* arg)
{
	auto& c = *static_cast<nested_ctx*>(arg);
	try {
		throw ex_a { 31 };
	} catch (const ex_a&) {
		try {
			throw;
		} catch (const ex_a&) {
			c.in_catch.signal();
			c.go.wait(ISIX_TIME_INFINITE);
			try {
				throw;
			} catch (const ex_a& e) {
				c.value = e.v;
			} catch (...) {
				c.wrong_type = true;
			}
		}
	}
	c.done.signal();
}

void nested_task_b(void* arg)
{
	auto& c = *static_cast<nested_ctx*>(arg);
	try {
		throw ex_b { 77 };
	} catch (const ex_b& e) {
		isix::wait_ms(1);
		c.b_value = e.v;
	}
	c.b_done.signal();
}

constexpr int c_stress_tasks = 4;
constexpr int c_stress_iters = 100;

struct stress_ctx {
	std::atomic<int> ok { 0 };
	std::atomic<int> bad { 0 };
	isix::semaphore done { 0, c_stress_tasks };
};

struct stress_arg {
	stress_ctx* ctx;
	int id;
};

void stress_task(void* arg)
{
	const auto a = *static_cast<stress_arg*>(arg);
	for (int i = 0; i < c_stress_iters; ++i) {
		const int expected = a.id * 1000 + i;
		try {
			if (i & 1) {
				throw ex_a { expected };
			}
			throw ex_b { expected };
		} catch (const ex_a& e) {
			isix::wait_ms(static_cast<unsigned long>(i % 3) + 1);
			try {
				throw;
			} catch (const ex_a& e2) {
				(e.v == expected && e2.v == expected ? a.ctx->ok : a.ctx->bad)++;
			} catch (...) {
				a.ctx->bad++;
			}
		} catch (const ex_b& e) {
			isix::wait_ms(static_cast<unsigned long>(i % 3) + 1);
			(e.v == expected ? a.ctx->ok : a.ctx->bad)++;
		}
	}
	a.ctx->done.signal();
}

} // namespace

TEST(exceptions, eh_globals_differs_per_task)
{
	globals_ctx a, b;
	const auto main_first = current_eh_globals();
	TEST_ASSERT_NOT_NULL(isix::task_create(globals_task, &a, c_ex_stack, c_prio_low, isix_task_flag_newlib));
	TEST_ASSERT_NOT_NULL(isix::task_create(globals_task, &b, c_ex_stack, c_prio_high, isix_task_flag_newlib));
	TEST_ASSERT_EQUAL(ISIX_EOK, a.done.wait(ex_wait()));
	TEST_ASSERT_EQUAL(ISIX_EOK, b.done.wait(ex_wait()));
	TEST_ASSERT_NOT_NULL(a.first.load());
	TEST_ASSERT_NOT_NULL(b.first.load());
	TEST_ASSERT_TRUE(a.first.load() == a.second.load());
	TEST_ASSERT_TRUE(b.first.load() == b.second.load());
	TEST_ASSERT_TRUE(a.first.load() != b.first.load());
	TEST_ASSERT_TRUE(a.first.load() != main_first);
	TEST_ASSERT_TRUE(b.first.load() != main_first);
	TEST_ASSERT_TRUE(main_first == current_eh_globals());
}

TEST(exceptions, exception_preempted_inside_catch)
{
	preempt_ctx a, b;
	TEST_ASSERT_NOT_NULL(isix::task_create(preempt_task<ex_a>, &a, c_ex_stack, c_prio_low, isix_task_flag_newlib));
	TEST_ASSERT_EQUAL(ISIX_EOK, a.in_catch.wait(ex_wait()));
	TEST_ASSERT_NOT_NULL(isix::task_create(preempt_task<ex_b>, &b, c_ex_stack, c_prio_high, isix_task_flag_newlib));
	TEST_ASSERT_EQUAL(ISIX_EOK, b.in_catch.wait(ex_wait()));
	a.go.signal();
	TEST_ASSERT_EQUAL(ISIX_EOK, a.done.wait(ex_wait()));
	b.go.signal();
	TEST_ASSERT_EQUAL(ISIX_EOK, b.done.wait(ex_wait()));
	TEST_ASSERT_TRUE(a.has_current.load());
	TEST_ASSERT_FALSE(a.wrong_type.load());
	TEST_ASSERT_EQUAL_INT(111, a.value.load());
	TEST_ASSERT_EQUAL_INT(0, a.uncaught.load());
	TEST_ASSERT_TRUE(b.has_current.load());
	TEST_ASSERT_FALSE(b.wrong_type.load());
	TEST_ASSERT_EQUAL_INT(222, b.value.load());
	TEST_ASSERT_EQUAL_INT(0, b.uncaught.load());
	TEST_ASSERT_EQUAL_INT(0, std::uncaught_exceptions());
}

TEST(exceptions, nested_rethrow_across_tasks)
{
	nested_ctx c;
	TEST_ASSERT_NOT_NULL(isix::task_create(nested_task_a, &c, c_ex_stack, c_prio_low, isix_task_flag_newlib));
	TEST_ASSERT_EQUAL(ISIX_EOK, c.in_catch.wait(ex_wait()));
	TEST_ASSERT_NOT_NULL(isix::task_create(nested_task_b, &c, c_ex_stack, c_prio_high, isix_task_flag_newlib));
	TEST_ASSERT_EQUAL(ISIX_EOK, c.b_done.wait(ex_wait()));
	c.go.signal();
	TEST_ASSERT_EQUAL(ISIX_EOK, c.done.wait(ex_wait()));
	TEST_ASSERT_FALSE(c.wrong_type.load());
	TEST_ASSERT_EQUAL_INT(31, c.value.load());
	TEST_ASSERT_EQUAL_INT(77, c.b_value.load());
}

TEST(exceptions, many_tasks_throw_stress)
{
	stress_ctx c;
	stress_arg args[c_stress_tasks];
	isix::wait_ms(50);
	isix::memory_stat before {};
	isix::heap_stats(before);
	for (int i = 0; i < c_stress_tasks; ++i) {
		args[i] = { &c, i + 1 };
		TEST_ASSERT_NOT_NULL(isix::task_create(
			stress_task, &args[i], c_ex_stack, static_cast<osprio_t>(c_prio_high + (i & 1)), isix_task_flag_newlib));
	}
	for (int i = 0; i < c_stress_tasks; ++i) {
		TEST_ASSERT_EQUAL(ISIX_EOK, c.done.wait(ex_wait()));
	}
	isix::wait_ms(50);
	TEST_ASSERT_EQUAL_INT(c_stress_tasks * c_stress_iters, c.ok.load());
	TEST_ASSERT_EQUAL_INT(0, c.bad.load());
	isix::memory_stat after {};
	isix::heap_stats(after);
	TEST_ASSERT_EQUAL_UINT(before.free, after.free);
}

TEST_GROUP_RUNNER(exceptions)
{
	RUN_TEST_CASE(exceptions, basic);
	RUN_TEST_CASE(exceptions, rethrow);
	RUN_TEST_CASE(exceptions, eh_globals_differs_per_task);
	RUN_TEST_CASE(exceptions, exception_preempted_inside_catch);
	RUN_TEST_CASE(exceptions, nested_rethrow_across_tasks);
	RUN_TEST_CASE(exceptions, many_tasks_throw_stress);
}
#else
TEST_GROUP_RUNNER(exceptions) {}
#endif