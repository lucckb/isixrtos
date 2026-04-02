#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <isix/config.h>
#include "utils/tickless_testhooks.h"
#include <limits>

TEST_GROUP(tickless);
TEST_GROUP(tickless_early);

TEST_SETUP(tickless) {}
TEST_SETUP(tickless_early) {}

TEST_TEAR_DOWN(tickless) {}
TEST_TEAR_DOWN(tickless_early) {}

#if CONFIG_ISIX_TICKLESS

namespace
{
	static constexpr ostick_t c_tick_max = std::numeric_limits<ostick_t>::max();

	static volatile unsigned s_pre_sleep_count;
	static volatile unsigned s_post_sleep_count;
	static volatile ostick_t s_last_expected_sleep;
	static volatile ostick_t s_last_actual_sleep;

	static volatile bool s_wait_done;
	static void waiter_task(void* arg)
	{
		(void)arg;
		isix::wait_ms(50);
		s_wait_done = true;
	}
	static volatile bool s_delayed_done;
	static void delayed_worker_task(void* arg)
	{
		(void)arg;
		isix::wait_ms(35);
		s_delayed_done = true;
	}

	static volatile bool s_busy_run;
	static void busy_task(void* arg)
	{
		(void)arg;
		while( s_busy_run ) {
			asm volatile("nop");
		}
	}

	struct wait_order_ctx {
		volatile int idx {};
		volatile int order[2] {};
	};
	static wait_order_ctx s_order_ctx;
	static void short_wait_task(void*)
	{
		isix::wait_ms(10);
		const int i = s_order_ctx.idx;
		if( i < 2 ) {
			s_order_ctx.order[i] = 1;
			s_order_ctx.idx = i + 1;
		}
	}
	static void long_wait_task(void*)
	{
		isix::wait_ms(30);
		const int i = s_order_ctx.idx;
		if( i < 2 ) {
			s_order_ctx.order[i] = 2;
			s_order_ctx.idx = i + 1;
		}
	}

	struct vtimer_ctx {
		volatile unsigned count {};
		unsigned target {};
		osvtimer_t tmr {};
		ossem_t done { nullptr };
	};
	static void periodic_cb(void* arg)
	{
		auto* ctx = static_cast<vtimer_ctx*>(arg);
		++ctx->count;
		if( ctx->count >= ctx->target ) {
			isix_vtimer_mod(ctx->tmr, OSVTIMER_CB_CANCEL);
			isix_sem_signal(ctx->done);
		}
	}
}

extern "C" __attribute__((used, externally_visible, noinline))
void isix_pre_sleep_hook(ostick_t expected_sleep_ticks)
{
	s_last_expected_sleep = expected_sleep_ticks;
	++s_pre_sleep_count;
}

extern "C" __attribute__((used, externally_visible, noinline))
void isix_post_sleep_hook(ostick_t actual_sleep_ticks)
{
	s_last_actual_sleep = actual_sleep_ticks;
	++s_post_sleep_count;
}

TEST(tickless, wait_advances_jiffies)
{
	s_wait_done = false;
	/* Lower numeric priority than test thread (0): otherwise yield loop never runs waiter. */
	auto t = isix::task_create(waiter_task, nullptr, 4096, 2, 0);
	TEST_ASSERT_NOT_NULL(t);
	const ostick_t j0 = isix::get_jiffies();
	for (int n = 0; n < 500 && !s_wait_done; ++n) {
		isix::wait_ms(5);
	}
	TEST_ASSERT_TRUE(s_wait_done);
	const ostick_t j1 = isix::get_jiffies();
	TEST_ASSERT_GREATER_THAN_UINT32(j0, j1);
	/* Waiter task exits on its own; kill is redundant if already EXITED. */
	isix::task_kill(t);
}

TEST(tickless, semaphore_timeout_returns_etimeout)
{
	isix::semaphore sem { 0, 1 };
	const int r = sem.wait(isix::ms2tick(20));
	TEST_ASSERT_EQUAL_INT(ISIX_ETIMEOUT, r);
}

TEST(tickless, ms2tick_one_second_matches_hz)
{
	TEST_ASSERT_EQUAL_UINT(static_cast<unsigned>(CONFIG_ISIX_HZ),
		static_cast<unsigned>(isix::ms2tick(1000)));
}

TEST(tickless, wait_ms_advances_ujiffies_wall_time)
{
	const osutick_t u0 = isix::get_ujiffies();
	isix::wait_ms(75);
	const osutick_t u1 = isix::get_ujiffies();
	const osutick_t delta = u1 - u0;
	/* Tickless + QEMU: allow wide slack; still proves monotonic wall-ish time. */
	TEST_ASSERT_UINT_WITHIN(100ULL * 1000ULL, 150ULL * 1000ULL, delta);
}

TEST(tickless, timer_elapsed_after_wait_ms)
{
	const ostick_t t0 = isix::get_jiffies();
	isix::wait_ms(45);
	TEST_ASSERT_TRUE(isix::timer_elapsed(t0, isix::ms2tick(30)));
}

TEST(tickless, chained_short_waits_advance_jiffies)
{
	const ostick_t j0 = isix::get_jiffies();
	for (int i = 0; i < 10; ++i) {
		isix::wait_ms(10);
	}
	const ostick_t j1 = isix::get_jiffies();
	const ostick_t d = j1 - j0;
	/* Expect ~100 ticks at 1 kHz; tickless may skip one edge. */
	TEST_ASSERT_UINT_WITHIN(23U, 107U, static_cast<unsigned>(d));
}

TEST(tickless, wait_tick_api_returns_ok)
{
	const int r = isix::wait(isix::ms2tick(12));
	TEST_ASSERT_EQUAL_INT(ISIX_EOK, r);
}

TEST(tickless, trywait_empty_semaphore_returns_ebusy)
{
	isix::semaphore sem { 0, 1 };
	TEST_ASSERT_EQUAL_INT(ISIX_EBUSY, sem.trywait());
}

TEST(tickless, delayed_worker_and_main_wait_coherent)
{
	s_delayed_done = false;
	auto* th = isix::task_create(delayed_worker_task, nullptr, 4096, 2, 0);
	TEST_ASSERT_NOT_NULL(th);
	const ostick_t j0 = isix::get_jiffies();
	for (int n = 0; n < 600 && !s_delayed_done; ++n) {
		isix::wait_ms(3);
	}
	TEST_ASSERT_TRUE(s_delayed_done);
	const ostick_t j1 = isix::get_jiffies();
	TEST_ASSERT_GREATER_THAN_UINT32(j0, j1);
	isix::task_kill(th);
}

TEST(tickless_early, sleep_entry_when_idle_only)
{
	s_pre_sleep_count = 0;
	s_post_sleep_count = 0;
	isix::wait_ms(1000);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(1U, s_pre_sleep_count);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(1U, s_post_sleep_count);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(CONFIG_ISIX_TICKLESS_MIN_SLEEP_TICKS, s_last_expected_sleep);
}

TEST(tickless, no_sleep_when_ready_work_exists)
{
	s_pre_sleep_count = 0;
	s_busy_run = true;
	auto t = isix::task_create(busy_task, nullptr, 2048, 2, 0);
	TEST_ASSERT_NOT_NULL(t);
	isix::wait_ms(20);
	/* Assert while busy task is still running and should keep idle unscheduled. */
	TEST_ASSERT_EQUAL_UINT(0U, s_pre_sleep_count);
	s_busy_run = false;
	isix::task_kill(t);
}

TEST(tickless_early, min_sleep_threshold_edges)
{
	s_pre_sleep_count = 0;
	/* Below threshold should not trigger tickless entry. */
	isix::wait(1);
	TEST_ASSERT_EQUAL_UINT(0U, s_pre_sleep_count);
	/* Re-arm for above-threshold phase. */
	s_pre_sleep_count = 0;
	s_post_sleep_count = 0;
	isix::wait_ms(1000);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(1U, s_pre_sleep_count);
}

TEST(tickless, wait_wraparound_advances_and_returns)
{
	const ostick_t near_wrap = (ostick_t)(c_tick_max - 5U);
	_isixp_test_set_jiffies(near_wrap);
	const ostick_t j0 = isix::get_jiffies();
	isix::wait_ms(20);
	const ostick_t j1 = isix::get_jiffies();
	TEST_ASSERT_TRUE(j1 < j0);
}

TEST(tickless, semaphore_timeout_wraparound)
{
	isix::semaphore sem { 0, 1 };
	_isixp_test_set_jiffies((ostick_t)(c_tick_max - 4U));
	const int r = sem.wait(isix::ms2tick(15));
	TEST_ASSERT_EQUAL_INT(ISIX_ETIMEOUT, r);
}

TEST(tickless, earliest_deadline_wins)
{
	s_order_ctx.idx = 0;
	s_order_ctx.order[0] = 0;
	s_order_ctx.order[1] = 0;
	_isixp_test_set_jiffies((ostick_t)(c_tick_max - 20U));
	auto ts = isix::task_create(short_wait_task, nullptr, 2048, 2, 0);
	auto tl = isix::task_create(long_wait_task, nullptr, 2048, 2, 0);
	TEST_ASSERT_NOT_NULL(ts);
	TEST_ASSERT_NOT_NULL(tl);
	for( int n=0; n<200 && s_order_ctx.idx < 2; ++n ) {
		isix::wait_ms(2);
	}
	TEST_ASSERT_EQUAL_INT(2, s_order_ctx.idx);
	TEST_ASSERT_EQUAL_INT(1, s_order_ctx.order[0]);
	TEST_ASSERT_EQUAL_INT(2, s_order_ctx.order[1]);
	isix::task_kill(ts);
	isix::task_kill(tl);
}

TEST(tickless, vtimer_periodic_catchup_after_jump)
{
	vtimer_ctx ctx;
	ctx.target = 5;
	ctx.done = isix_sem_create_limited(nullptr, 0, 1);
	TEST_ASSERT_NOT_NULL(ctx.done);
	ctx.tmr = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(ctx.tmr);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_start(ctx.tmr, periodic_cb, &ctx, 10, true));
	isix::wait_ms(5);
	const ostick_t now = isix::get_jiffies();
	_isixp_test_set_jiffies(now + 60U);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::sem_wait(ctx.done, isix::ms2tick(200)));
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(ctx.target, ctx.count);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(ctx.tmr));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(ctx.done));
}

TEST(tickless, vtimer_periodic_wraparound_catchup)
{
	vtimer_ctx ctx;
	ctx.target = 3;
	ctx.done = isix_sem_create_limited(nullptr, 0, 1);
	TEST_ASSERT_NOT_NULL(ctx.done);
	ctx.tmr = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(ctx.tmr);
	_isixp_test_set_jiffies((ostick_t)(c_tick_max - 8U));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_start(ctx.tmr, periodic_cb, &ctx, 5, true));
	isix::wait_ms(2);
	_isixp_test_advance_jiffies(30U);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::sem_wait(ctx.done, isix::ms2tick(200)));
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(ctx.target, ctx.count);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(ctx.tmr));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(ctx.done));
}

#else /* !CONFIG_ISIX_TICKLESS */

TEST(tickless, tickless_disabled_stub)
{
	TEST_PASS();
}

#endif

TEST_GROUP_RUNNER(tickless)
{
#if CONFIG_ISIX_TICKLESS
	RUN_TEST_CASE(tickless, wait_advances_jiffies);
	RUN_TEST_CASE(tickless, semaphore_timeout_returns_etimeout);
	RUN_TEST_CASE(tickless, ms2tick_one_second_matches_hz);
	RUN_TEST_CASE(tickless, wait_ms_advances_ujiffies_wall_time);
	RUN_TEST_CASE(tickless, timer_elapsed_after_wait_ms);
	RUN_TEST_CASE(tickless, chained_short_waits_advance_jiffies);
	RUN_TEST_CASE(tickless, wait_tick_api_returns_ok);
	RUN_TEST_CASE(tickless, trywait_empty_semaphore_returns_ebusy);
	RUN_TEST_CASE(tickless, delayed_worker_and_main_wait_coherent);
	RUN_TEST_CASE(tickless, no_sleep_when_ready_work_exists);
	RUN_TEST_CASE(tickless, wait_wraparound_advances_and_returns);
	RUN_TEST_CASE(tickless, semaphore_timeout_wraparound);
	RUN_TEST_CASE(tickless, earliest_deadline_wins);
	RUN_TEST_CASE(tickless, vtimer_periodic_catchup_after_jump);
	RUN_TEST_CASE(tickless, vtimer_periodic_wraparound_catchup);
#else
	RUN_TEST_CASE(tickless, tickless_disabled_stub);
#endif
}

TEST_GROUP_RUNNER(tickless_early)
{
#if CONFIG_ISIX_TICKLESS
	RUN_TEST_CASE(tickless_early, sleep_entry_when_idle_only);
	RUN_TEST_CASE(tickless_early, min_sleep_threshold_edges);
#else
	RUN_TEST_CASE(tickless, tickless_disabled_stub);
#endif
}
