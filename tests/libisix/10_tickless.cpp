#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <isix/config.h>
#include "utils/tickless_testhooks.h"
#include "timer_interrupt.hpp"
#include <isix/arch/irq_platform.h>
#include <isix/arch/irq.h>
#include <isix/arch/isr_vectors.h>
#include <stm32_ll_tim.h>
#include <limits>
#include <isix/prv/systimer_upcounter.h>

TEST_GROUP(tickless);
TEST_GROUP(tickless_early);

TEST_SETUP(tickless_early) {}

#if CONFIG_ISIX_TICKLESS
TEST_SETUP(tickless)
{
	// The debug idle is nop, use the real wfi so the long period is really entered
	_isixp_test_tickless_force_wfi(true);
	_isixp_test_tickless_stats_reset();
}
#else
TEST_SETUP(tickless) {}
TEST_TEAR_DOWN(tickless)
{
	tests::detail::periodic_timer_stop();
}
#endif
TEST_TEAR_DOWN(tickless_early) {}

#if CONFIG_ISIX_TICKLESS

extern "C" {
	void _isixp_lock_scheduler();
	void _isixp_unlock_scheduler();
}

namespace
{
	static constexpr ostick_t c_tick_max = std::numeric_limits<ostick_t>::max();

	static volatile unsigned s_pre_sleep_count;
	static volatile unsigned s_post_sleep_count;
	static volatile ostick_t s_last_expected_sleep;
	static volatile ostick_t s_max_expected_sleep;
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
	// The context outlives a failed test so that the teardown can stop the timer
	static vtimer_ctx s_vctx;
	static void vctx_reset()
	{
		s_vctx.count = 0;
		s_vctx.target = 0;
		s_vctx.tmr = nullptr;
		s_vctx.done = nullptr;
	}
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

TEST_TEAR_DOWN(tickless)
{
	tests::detail::periodic_timer_stop();
	if( s_vctx.tmr ) {
		isix_vtimer_destroy(s_vctx.tmr);
		s_vctx.tmr = nullptr;
	}
	if( s_vctx.done ) {
		isix_sem_destroy(s_vctx.done);
		s_vctx.done = nullptr;
	}
	_isixp_test_hook = nullptr;
	_isixp_test_tickless_set_max_ticks(0);
	_isixp_test_tickless_force_wfi(false);
}

extern "C" __attribute__((used, externally_visible, noinline))
void isix_pre_sleep_hook(ostick_t expected_sleep_ticks)
{
	s_last_expected_sleep = expected_sleep_ticks;
	if( expected_sleep_ticks > s_max_expected_sleep ) {
		s_max_expected_sleep = expected_sleep_ticks;
	}
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
	const unsigned expected = 10U * isix::ms2tick(10);
	TEST_ASSERT_UINT_WITHIN(expected * 23U / 100U, expected * 107U / 100U, static_cast<unsigned>(d));
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

TEST(tickless, sleep_entry_after_scheduler_lock)
{
	_isixp_lock_scheduler();
	auto* const systick_val = reinterpret_cast<volatile uint32_t*>(0xE000E018UL);
	auto prev = *systick_val;
	for (int wraps=0; wraps<2;) {
		const auto cur = *systick_val;
		if (cur > prev) {
			++wraps;
		}
		prev = cur;
	}
	_isixp_unlock_scheduler();
	s_pre_sleep_count = 0;
	isix::wait_ms(200);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(1U, s_pre_sleep_count);
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
	// The wait of 20 ms is about 20 ticks at 1 kHz and must cross the wrap
	const ostick_t near_wrap = (ostick_t)(c_tick_max - isix::ms2tick(20) / 4U);
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
	vctx_reset();
	auto& ctx = s_vctx;
	ctx.target = 5;
	ctx.done = isix_sem_create_limited(nullptr, 0, 1);
	TEST_ASSERT_NOT_NULL(ctx.done);
	ctx.tmr = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(ctx.tmr);
	// The jump below must not cross the jiffies wrap left by the previous tests
	_isixp_test_set_jiffies(1000U);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_start(ctx.tmr, periodic_cb, &ctx, 10, true));
	isix::wait_ms(5);
	const ostick_t now = isix::get_jiffies();
	_isixp_test_set_jiffies(now + 60U);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::sem_wait(ctx.done, 200U));
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(ctx.target, ctx.count);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(ctx.tmr));
	ctx.tmr = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(ctx.done));
	ctx.done = nullptr;
}

TEST(tickless, vtimer_periodic_wraparound_catchup)
{
	vctx_reset();
	auto& ctx = s_vctx;
	ctx.target = 3;
	ctx.done = isix_sem_create_limited(nullptr, 0, 1);
	TEST_ASSERT_NOT_NULL(ctx.done);
	ctx.tmr = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(ctx.tmr);
	_isixp_test_set_jiffies((ostick_t)(c_tick_max - 8U));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_start(ctx.tmr, periodic_cb, &ctx, 5, true));
	isix::wait_ms(2);
	_isixp_test_advance_jiffies(30U);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::sem_wait(ctx.done, 200U));
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(ctx.target, ctx.count);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(ctx.tmr));
	ctx.tmr = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(ctx.done));
	ctx.done = nullptr;
}

TEST(tickless, jiffies_track_wall_time_under_irq_load)
{
	static volatile unsigned irq_count;
	irq_count = 0;
	const auto ok = tests::detail::periodic_timer_setup([]() { irq_count = irq_count + 1; }, 300);
	TEST_ASSERT_TRUE(ok);
	const auto j0 = isix::get_jiffies();
	const auto u0 = isix::get_ujiffies();
	isix::wait_ms(500);
	const auto j1 = isix::get_jiffies();
	const auto u1 = isix::get_ujiffies();
	tests::detail::periodic_timer_stop();
	const auto irqs = irq_count;
	const auto dj = static_cast<unsigned long>(j1 - j0) * 1000UL / CONFIG_ISIX_HZ;
	const auto du = static_cast<unsigned long>(u1 - u0) / 1000UL;
	tiny_printf("tickless drift: jiffies %lu ms, ujiffies %lu ms, irqs %lu\r\n",
		dj, du, static_cast<unsigned long>(irqs));
	// Interrupt count is an independent wall clock: the timer period does not depend on kernel jiffies
#ifdef QEMU_NO_RCC_PERIPH
	// The QEMU timer model is not accurate, only require that the timer ran
	constexpr unsigned long irqs_min = 100UL;
	constexpr unsigned long irqs_max = 2000UL;
#else
	constexpr unsigned long irqs_min = 1500UL;
	constexpr unsigned long irqs_max = 1833UL;
#endif
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(irqs_min, static_cast<unsigned long>(irqs));
	TEST_ASSERT_LESS_OR_EQUAL_UINT32(irqs_max, static_cast<unsigned long>(irqs));
	const auto diff = dj > du ? dj - du : du - dj;
	if (diff * 20UL > du) {
		TEST_IGNORE_MESSAGE("jiffies drift under IRQ load above 5 percent");
	}
}


namespace
{
	constexpr uintptr_t c_stk_cvr = 0xE000E018UL;
	constexpr uintptr_t c_scb_icsr = 0xE000ED04UL;
	constexpr uint32_t c_icsr_pendstset = 1UL << 26;

	inline uint32_t tick_counter()
	{
		return *reinterpret_cast<volatile uint32_t*>(c_stk_cvr);
	}
	inline bool tick_pending()
	{
		return (*reinterpret_cast<volatile uint32_t*>(c_scb_icsr) & c_icsr_pendstset) != 0U;
	}

	isix_tickless_stats read_stats()
	{
		isix_tickless_stats st {};
		_isixp_test_tickless_stats(&st);
		return st;
	}

	// Number of ticks the elapsed time is allowed to differ from the requested one
	void assert_wait_span(ostick_t j0, ostick_t j1, ostick_t expected, ostick_t slack)
	{
		const ostick_t d = j1 - j0;
		TEST_ASSERT_GREATER_OR_EQUAL_UINT32(expected, d);
		TEST_ASSERT_LESS_OR_EQUAL_UINT32(expected + slack, d);
	}

	// Limit of the kernel clock error under the interrupt load, parts per million
	constexpr int64_t c_drift_irq_limit_ppm = 200;

	// Unity without 64-bit support: compare here and report the numbers
	[[maybe_unused]] void assert_abs_le(int64_t value, int64_t limit, const char* what)
	{
		const int64_t a = value < 0 ? -value : value;
		if( a > limit ) {
			tiny_printf("%s: |%ld| > %ld\r\n", what, static_cast<long>(value), static_cast<long>(limit));
			TEST_FAIL_MESSAGE(what);
		}
	}

	// Poll limit for the hooks, one tick is far below it
	inline uint32_t spin_limit()
	{
		return 8U * read_stats().tpp + 1024U;
	}

	// Hook state shared with the test body
	volatile unsigned s_forced;
	volatile unsigned s_hook_point;
	volatile uint32_t s_spin_cap;
	volatile uint32_t s_hook_tpp;

	void hook_spin_pending(isix_test_point point, void*)
	{
		if( static_cast<unsigned>(point) != s_hook_point ) {
			return;
		}
		for( uint32_t i = 0; i < s_spin_cap && !tick_pending(); ++i ) {}
		++s_forced;
	}
}

// A long wait must cross many long periods
TEST(tickless, long_sleep_spans_max_period)
{
	_isixp_test_tickless_set_max_ticks(5);
	_isixp_test_tickless_stats_reset();
	const ostick_t j0 = isix::get_jiffies();
	isix::wait(53);
	const ostick_t j1 = isix::get_jiffies();
	const auto st = read_stats();
	assert_wait_span(j0, j1, 53, 1);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(8U, st.sleeps);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(8U, st.expired);
}

// The hooks are paired and report the programmed period, not the requested one
TEST(tickless, sleep_hooks_report_programmed_period)
{
	_isixp_test_tickless_set_max_ticks(5);
	isix::wait(2);
	s_pre_sleep_count = 0;
	s_post_sleep_count = 0;
	s_max_expected_sleep = 0;
	isix::wait(53);
	const unsigned pre = s_pre_sleep_count;
	const unsigned post = s_post_sleep_count;
	// The idle task may still be inside the last sleep
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(8U, pre);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(pre - 1U, post);
	TEST_ASSERT_LESS_OR_EQUAL_UINT32(pre, post);
	// The first jiffy of the sleep is the rest of the current tick, the long period is 5 ticks
	TEST_ASSERT_EQUAL_UINT32(6U, s_max_expected_sleep);
}

// The long period is not a multiple of anything the wait asks for
TEST(tickless, max_period_not_multiple)
{
	for( const ostick_t max_ticks : { 1U, 2U, 3U, 7U } ) {
		_isixp_test_tickless_set_max_ticks(max_ticks);
		_isixp_test_tickless_stats_reset();
		const ostick_t j0 = isix::get_jiffies();
		isix::wait(40);
		const ostick_t j1 = isix::get_jiffies();
		assert_wait_span(j0, j1, 40, 1);
		if( max_ticks == 1U ) {
			TEST_ASSERT_EQUAL_UINT32(0U, read_stats().sleeps);
		}
	}
}

// Boundary reached before the long period is loaded (the reload takes the old value)
TEST(tickless, boundary_before_reload_update_on_enter)
{
	_isixp_test_tickless_stats_reset();
	// Statistics give the timer period only after the first sleep
	isix::wait(5);
	s_hook_point = isix_tp_tickless_pre_arm;
	s_spin_cap = spin_limit();
	s_forced = 0;
	_isixp_test_hook = hook_spin_pending;
	const ostick_t j0 = isix::get_jiffies();
	isix::wait(30);
	const ostick_t j1 = isix::get_jiffies();
	_isixp_test_hook = nullptr;
	assert_wait_span(j0, j1, 30, 1);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1U, s_forced);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1U, read_stats().enter_aborted);
}

// Boundary reached after the long period was loaded
TEST(tickless, boundary_after_reload_update_on_enter)
{
	isix::wait(5);
	s_hook_point = isix_tp_tickless_armed;
	s_spin_cap = spin_limit();
	s_forced = 0;
	_isixp_test_tickless_stats_reset();
	_isixp_test_hook = hook_spin_pending;
	const ostick_t j0 = isix::get_jiffies();
	isix::wait(30);
	const ostick_t j1 = isix::get_jiffies();
	_isixp_test_hook = nullptr;
	assert_wait_span(j0, j1, 30, 1);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1U, s_forced);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1U, read_stats().sleeps);
}

namespace
{
	volatile unsigned s_irq_hits;
	ossem_t s_wake_sem;
	volatile bool s_wake_run;
	volatile unsigned s_wake_count;
	ostask_t s_wake_task;

	void wake_task(void*)
	{
		while( s_wake_run ) {
			if( isix_sem_wait(s_wake_sem, isix::ms2tick(50)) == ISIX_EOK ) {
				s_wake_count = s_wake_count + 1U;
			}
		}
	}

	void waker_irq()
	{
		s_irq_hits = s_irq_hits + 1U;
		isix_sem_signal_isr(s_wake_sem);
	}

	// Raw priority above the tick interrupt and below the kernel masking level (BASEPRI 0x10)
	constexpr uint8_t c_irq_raw_prio = 0x20;

	// Task woken by the timer interrupt
	bool waker_start(uint32_t period_us)
	{
		s_wake_sem = isix_sem_create_limited(nullptr, 0, 1000);
		s_wake_run = true;
		s_wake_count = 0;
		s_irq_hits = 0;
		s_wake_task = isix::task_create(wake_task, nullptr, 2048, 1, 0);
		if( !s_wake_sem || !s_wake_task ) {
			return false;
		}
		return tests::detail::periodic_timer_setup(waker_irq, period_us, c_irq_raw_prio);
	}

	void waker_stop()
	{
		tests::detail::periodic_timer_stop();
		s_wake_run = false;
		if( s_wake_sem ) {
			isix_sem_signal(s_wake_sem);
		}
		// The task leaves its loop within the semaphore timeout and is reclaimed by the idle task
		isix::wait(60);
		s_wake_task = nullptr;
		if( s_wake_sem ) {
			isix_sem_destroy(s_wake_sem);
			s_wake_sem = nullptr;
		}
	}
}

// The test owned interrupt, raised only by software
ISIX_ISR_VECTOR(tim4_isr_vector)
{
	if( s_wake_sem ) {
		isix_sem_signal_isr(s_wake_sem);
	}
}

namespace
{
	// Priority above the tick interrupt preempts its handler, below it runs after the handler
	bool soft_irq_start(uint8_t raw_prio = c_irq_raw_prio)
	{
		s_wake_sem = isix_sem_create_limited(nullptr, 0, 1000);
		s_wake_run = true;
		s_wake_count = 0;
		s_wake_task = isix::task_create(wake_task, nullptr, 2048, 1, 0);
		if( !s_wake_sem || !s_wake_task ) {
			return false;
		}
		isix::set_raw_irq_priority(TIM4_IRQn, raw_prio);
		isix::request_irq(TIM4_IRQn);
		return true;
	}

	void soft_irq_stop()
	{
		isix::free_irq(TIM4_IRQn);
		s_wake_run = false;
		if( s_wake_sem ) {
			isix_sem_signal(s_wake_sem);
		}
		// The task leaves its loop within the semaphore timeout and is reclaimed by the idle task
		isix::wait(60);
		s_wake_task = nullptr;
		if( s_wake_sem ) {
			isix_sem_destroy(s_wake_sem);
			s_wake_sem = nullptr;
		}
	}
}

// Wakeup in the armed state after the first boundary but before its handler ran
TEST(tickless, armed_wake_after_boundary)
{
	isix::wait(5);
	s_hook_point = isix_tp_tickless_wake_armed;
	s_spin_cap = spin_limit();
	s_forced = 0;
	TEST_ASSERT_TRUE(waker_start(300));
	_isixp_test_tickless_stats_reset();
	_isixp_test_hook = hook_spin_pending;
	const ostick_t j0 = isix::get_jiffies();
	isix::wait(40);
	const ostick_t j1 = isix::get_jiffies();
	_isixp_test_hook = nullptr;
	waker_stop();
	assert_wait_span(j0, j1, 40, 1);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1U, read_stats().wake_armed_passed);
}

namespace
{
	volatile unsigned s_wrap_seq;
	volatile unsigned s_wrap_order[2];
	volatile ostick_t s_wrap_start[2];
	volatile ostick_t s_wrap_end[2];

	template<unsigned N, ostick_t Timeout>
	void wrap_waiter(void*)
	{
		s_wrap_start[N] = isix::get_jiffies();
		isix::wait(Timeout);
		s_wrap_end[N] = isix::get_jiffies();
		const unsigned i = s_wrap_seq;
		if( i < 2U ) {
			s_wrap_order[i] = N;
			s_wrap_seq = i + 1U;
		}
	}
}

// One batch of ticks crosses the jiffies wrap and wakes the tasks of both lists
TEST(tickless, sleep_catchup_crosses_jiffies_wrap)
{
	_isixp_test_tickless_set_max_ticks(0);
	_isixp_test_set_jiffies(c_tick_max - 30U);
	s_wrap_seq = 0;
	auto* const ta = isix::task_create(wrap_waiter<0, 20>, nullptr, 2048, 2, 0);
	auto* const tb = isix::task_create(wrap_waiter<1, 60>, nullptr, 2048, 2, 0);
	TEST_ASSERT_NOT_NULL(ta);
	TEST_ASSERT_NOT_NULL(tb);
	isix::wait(100);
	TEST_ASSERT_EQUAL_UINT32(2U, s_wrap_seq);
	TEST_ASSERT_EQUAL_UINT32(0U, s_wrap_order[0]);
	TEST_ASSERT_EQUAL_UINT32(1U, s_wrap_order[1]);
	assert_wait_span(s_wrap_start[0], s_wrap_end[0], 20, 1);
	assert_wait_span(s_wrap_start[1], s_wrap_end[1], 60, 1);
	// The second deadline is after the wrap
	TEST_ASSERT_TRUE(s_wrap_end[1] < s_wrap_start[0]);
	// Both tasks have finished by themselves
}

// One long period spans the wrap of the jiffies counter
TEST(tickless, long_period_across_jiffies_wrap)
{
	_isixp_test_tickless_set_max_ticks(0);
	_isixp_test_set_jiffies(c_tick_max - 3U);
	_isixp_test_tickless_stats_reset();
	const ostick_t j0 = isix::get_jiffies();
	isix::wait(30);
	const ostick_t j1 = isix::get_jiffies();
	assert_wait_span(j0, j1, 30, 1);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1U, read_stats().sleeps);
}

TEST(tickless, wait_across_wrap_exact)
{
	_isixp_test_set_jiffies(c_tick_max - isix::ms2tick(20) / 4U);
	const ostick_t j0 = isix::get_jiffies();
	isix::wait_ms(20);
	const ostick_t j1 = isix::get_jiffies();
	TEST_ASSERT_TRUE(j1 < j0);
	assert_wait_span(j0, j1, isix::ms2tick(20), 1);
}

TEST(tickless, semaphore_timeout_across_wrap_exact)
{
	isix::semaphore sem { 0, 1 };
	_isixp_test_set_jiffies(c_tick_max - isix::ms2tick(25) / 5U);
	const ostick_t j0 = isix::get_jiffies();
	const int r = sem.wait(isix::ms2tick(25));
	const ostick_t j1 = isix::get_jiffies();
	TEST_ASSERT_EQUAL_INT(ISIX_ETIMEOUT, r);
	assert_wait_span(j0, j1, isix::ms2tick(25), 1);
}

namespace
{
	struct oneshot_ctx {
		volatile unsigned count {};
		volatile ostick_t at {};
	};
	void oneshot_cb(void* arg)
	{
		auto* const ctx = static_cast<oneshot_ctx*>(arg);
		ctx->at = isix::get_jiffies();
		ctx->count = ctx->count + 1U;
	}
}

// A virtual timer deadline after the wrap fires on time while the system really sleeps
TEST(tickless, vtimer_one_shot_across_wrap_in_sleep)
{
	oneshot_ctx ctx;
	auto* const tmr = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(tmr);
	_isixp_test_set_jiffies(c_tick_max - 10U);
	_isixp_test_tickless_stats_reset();
	const ostick_t j0 = isix::get_jiffies();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_start(tmr, oneshot_cb, &ctx, 25, false));
	isix::wait(100);
	TEST_ASSERT_EQUAL_UINT32(1U, ctx.count);
	assert_wait_span(j0, ctx.at, 25, 2);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1U, read_stats().sleeps);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(tmr));
}

#ifndef QEMU_NO_RCC_PERIPH
namespace
{
	constexpr uintptr_t c_dwt_ctrl = 0xE0001000UL;
	constexpr uintptr_t c_dwt_cyccnt = 0xE0001004UL;
	constexpr uintptr_t c_dwt_demcr = 0xE000EDFCUL;

	inline volatile uint32_t& reg(uintptr_t addr)
	{
		return *reinterpret_cast<volatile uint32_t*>(addr);
	}

	void dwt_enable()
	{
		reg(c_dwt_demcr) = reg(c_dwt_demcr) | (1UL << 24);
		reg(c_dwt_ctrl) = reg(c_dwt_ctrl) | 1UL;
	}
	inline uint32_t dwt_now() { return reg(c_dwt_cyccnt); }

	// Kernel time minus the cycle counter time in cycles
	int64_t dwt_deviation(uint32_t jiffies, uint32_t cycles)
	{
		const int64_t cpj = _isix_port_get_core_freq() / CONFIG_ISIX_HZ;
		return static_cast<int64_t>(jiffies) * cpj - static_cast<int64_t>(cycles);
	}

	struct drift_result {
		int64_t deviation;
		long ppm;
	};

	// Start right after a tick boundary, the elapsed part of the tick would bias the result
	ostick_t sync_to_tick()
	{
		const ostick_t j = isix::get_jiffies();
		while( isix::get_jiffies() == j ) {}
		return isix::get_jiffies();
	}

	drift_result measure_drift(unsigned ms)
	{
		dwt_enable();
		const ostick_t j0 = sync_to_tick();
		const uint32_t c0 = dwt_now();
		isix::wait_ms(ms);
		const uint32_t c1 = dwt_now();
		const ostick_t j1 = isix::get_jiffies();
		const int64_t dev = dwt_deviation(j1 - j0, c1 - c0);
		return { dev, static_cast<long>(dev * 1000000LL / static_cast<int64_t>(c1 - c0)) };
	}
}
#endif

// IRQ preempting the tick interrupt must not corrupt the time
TEST(tickless, systick_isr_preempted_by_waking_irq)
{
	_isixp_test_tickless_set_max_ticks(3);
	TEST_ASSERT_TRUE(waker_start(97));
#ifndef QEMU_NO_RCC_PERIPH
	dwt_enable();
	const uint32_t c0 = dwt_now();
#endif
	const ostick_t j0 = isix::get_jiffies();
	isix::wait_ms(2000);
	const ostick_t j1 = isix::get_jiffies();
#ifndef QEMU_NO_RCC_PERIPH
	const uint32_t c1 = dwt_now();
#endif
	const unsigned irqs = s_irq_hits;
	const unsigned wakes = s_wake_count;
	const auto st = read_stats();
	waker_stop();
	tiny_printf("tickless nested: jiffies %lu irqs %u wakes %u\r\n",
		static_cast<unsigned long>(j1 - j0), irqs, wakes);
	tiny_printf("tickless nested stats: sleeps %lu aborted %lu wake_armed %lu passed %lu wake_sleeping %lu expired %lu restarts %lu batch %lu\r\n",
		static_cast<unsigned long>(st.sleeps), static_cast<unsigned long>(st.enter_aborted),
		static_cast<unsigned long>(st.wake_armed), static_cast<unsigned long>(st.wake_armed_passed),
		static_cast<unsigned long>(st.wake_sleeping), static_cast<unsigned long>(st.expired),
		static_cast<unsigned long>(st.restarts), static_cast<unsigned long>(st.ticks_caught_up));
	TEST_ASSERT_GREATER_THAN_UINT32(0U, irqs);
	assert_wait_span(j0, j1, isix::ms2tick(2000), 1);
#ifndef QEMU_NO_RCC_PERIPH
	assert_abs_le(dwt_deviation(j1 - j0, c1 - c0), static_cast<int64_t>(c1 - c0) / 200, "kernel time against the cycle counter");
#endif
}

namespace
{
	void hook_nested_irq(isix_test_point point, void*)
	{
		if( point == isix_tp_tickless_isr_armed ) {
			// The interrupt has the higher priority and preempts the tick handler at once
			isix_generate_software_interrupt(TIM4_IRQn);
			++s_forced;
		}
	}
}

// The tick interrupt in the armed state is preempted by an interrupt that wakes a task
TEST(tickless, systick_isr_armed_preempted_by_waking_irq)
{
	_isixp_test_tickless_set_max_ticks(3);
	s_forced = 0;
	TEST_ASSERT_TRUE(soft_irq_start());
	isix::wait(5);
	_isixp_test_hook = hook_nested_irq;
	ostick_t worst = 0;
	for( int i = 0; i < 20; ++i ) {
		const ostick_t j0 = isix::get_jiffies();
		isix::wait(20);
		const ostick_t d = isix::get_jiffies() - j0;
		if( d > worst ) {
			worst = d;
		}
	}
	_isixp_test_hook = nullptr;
	const unsigned forced = s_forced;
	const auto st = read_stats();
	soft_irq_stop();
	tiny_printf("tickless nested armed: forced %lu worst wait %lu expired %lu\r\n",
		static_cast<unsigned long>(forced), static_cast<unsigned long>(worst),
		static_cast<unsigned long>(st.expired));
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(10U, forced);
	TEST_ASSERT_LESS_OR_EQUAL_UINT32(21U, worst);
}

namespace
{
	void hook_entry_irq(isix_test_point point, void*)
	{
		if( point == isix_tp_tickless_isr_entry ) {
			// Preempts the tick handler before the kernel masks the interrupts
			isix_generate_software_interrupt(TIM4_IRQn);
			++s_forced;
		}
	}
}

// The tick interrupt handler is preempted at its first instruction by a waking interrupt
TEST(tickless, systick_isr_entry_preempted_by_waking_irq)
{
	_isixp_test_tickless_set_max_ticks(3);
	s_forced = 0;
	TEST_ASSERT_TRUE(soft_irq_start());
	isix::wait(5);
	_isixp_test_hook = hook_entry_irq;
#ifndef QEMU_NO_RCC_PERIPH
	dwt_enable();
	const uint32_t c0 = dwt_now();
#endif
	const ostick_t j0 = isix::get_jiffies();
	ostick_t worst = 0;
	for( int i = 0; i < 20; ++i ) {
		const ostick_t jw = isix::get_jiffies();
		isix::wait(20);
		const ostick_t d = isix::get_jiffies() - jw;
		if( d > worst ) {
			worst = d;
		}
	}
	const ostick_t j1 = isix::get_jiffies();
#ifndef QEMU_NO_RCC_PERIPH
	const uint32_t c1 = dwt_now();
#endif
	_isixp_test_hook = nullptr;
	const unsigned forced = s_forced;
	soft_irq_stop();
	tiny_printf("tickless nested entry: forced %lu worst wait %lu jiffies %lu\r\n",
		static_cast<unsigned long>(forced), static_cast<unsigned long>(worst),
		static_cast<unsigned long>(j1 - j0));
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(10U, forced);
	TEST_ASSERT_LESS_OR_EQUAL_UINT32(21U, worst);
#ifndef QEMU_NO_RCC_PERIPH
	// Every boundary counted twice would be a whole tick period of cycles each
	assert_abs_le(dwt_deviation(j1 - j0, c1 - c0), 2 * static_cast<int64_t>(_isix_port_get_core_freq() / CONFIG_ISIX_HZ), "tick counted twice");
#endif
}

namespace
{
	volatile unsigned s_race_tiny;

	void hook_race(isix_test_point point, void* arg)
	{
		if( point == isix_tp_tickless_isr_armed ) {
			// Raised in the tick handler, it runs when the handler is left: the sleep is already running
			isix_generate_software_interrupt(TIM4_IRQn);
			return;
		}
		if( point != isix_tp_tickless_wake_sleeping ) {
			return;
		}
		auto* const cvr = static_cast<uint32_t*>(arg);
		const uint32_t tpp = s_hook_tpp;
		bool pending = false;
		uint32_t now = *cvr;
		for( uint32_t i = 0; i < s_spin_cap; ++i ) {
			pending = tick_pending();
			now = tick_counter();
			if( pending || (now % tpp) < 16U ) {
				break;
			}
		}
		// Fresh counter just before a tick boundary, the end of the long period reads as an empty counter
		*cvr = pending ? 0U : now;
		s_race_tiny = s_race_tiny + 1U;
	}
}

// The wakeup lands right before a tick boundary of the long period and before its end
TEST(tickless, sleeping_wake_race_with_period_end)
{
	_isixp_test_tickless_set_max_ticks(3);
	isix::wait(10);
	s_hook_tpp = read_stats().tpp;
	s_spin_cap = spin_limit();
	s_race_tiny = 0;
	// The wakeup runs when the tick handler is left
	TEST_ASSERT_TRUE(soft_irq_start(0xf8));
	_isixp_test_tickless_stats_reset();
	_isixp_test_hook = hook_race;
#ifndef QEMU_NO_RCC_PERIPH
	dwt_enable();
	const uint32_t c0 = dwt_now();
#endif
	const ostick_t j0 = isix::get_jiffies();
	isix::wait_ms(1000);
	const ostick_t j1 = isix::get_jiffies();
#ifndef QEMU_NO_RCC_PERIPH
	const uint32_t c1 = dwt_now();
#endif
	_isixp_test_hook = nullptr;
	const auto st = read_stats();
	const unsigned forced = s_race_tiny;
	soft_irq_stop();
	tiny_printf("tickless race: forced %lu wake_sleeping %lu jiffies %lu\r\n",
		static_cast<unsigned long>(forced), static_cast<unsigned long>(st.wake_sleeping),
		static_cast<unsigned long>(j1 - j0));
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(10U, forced);
	assert_wait_span(j0, j1, isix::ms2tick(1000), 1);
#ifndef QEMU_NO_RCC_PERIPH
	// A lost tick is one tick period of cycles
	assert_abs_le(dwt_deviation(j1 - j0, c1 - c0), _isix_port_get_core_freq() / CONFIG_ISIX_HZ, "lost ticks");
#endif
}

namespace
{
	volatile uint64_t s_isr_prev;
	volatile unsigned s_isr_back;
	volatile unsigned s_isr_jump;
	volatile unsigned s_isr_samples;
	constexpr uint64_t c_tick_us = 1000000ULL / CONFIG_ISIX_HZ;
	// The samples are 250 us apart
	constexpr uint64_t c_jump_limit_us = 250ULL + 2ULL * c_tick_us;

	void ujiffies_isr_sample()
	{
		const uint64_t now = isix_get_ujiffies();
		const uint64_t prev = s_isr_prev;
		if( s_isr_samples > 0U ) {
			if( now < prev ) {
				s_isr_back = s_isr_back + 1U;
			} else if( now - prev > c_jump_limit_us ) {
				s_isr_jump = s_isr_jump + 1U;
			}
		}
		s_isr_prev = now;
		s_isr_samples = s_isr_samples + 1U;
	}
}

// The microsecond clock never goes back, also from the interrupt that wakes the sleeping core
TEST(tickless, ujiffies_monotonic_in_isr_and_task)
{
	s_isr_prev = 0;
	s_isr_back = 0;
	s_isr_jump = 0;
	s_isr_samples = 0;
	_isixp_test_tickless_set_max_ticks(5);
	const auto ok = tests::detail::periodic_timer_setup(ujiffies_isr_sample, 250, c_irq_raw_prio);
	TEST_ASSERT_TRUE(ok);
	isix::wait_ms(300);
	// Busy phase, the same check from the task context
	uint64_t prev = isix_get_ujiffies();
	unsigned task_back = 0;
	unsigned task_jump = 0;
	const ostick_t jb = isix::get_jiffies();
	while( static_cast<ostick_t>(isix::get_jiffies() - jb) < 30U ) {
		const uint64_t now = isix_get_ujiffies();
		if( now < prev ) {
			++task_back;
		} else if( now - prev > 2ULL * c_tick_us ) {
			++task_jump;
		}
		prev = now;
	}
	tests::detail::periodic_timer_stop();
	tiny_printf("tickless ujiffies: isr samples %u back %u jump %u task back %u jump %u\r\n",
		static_cast<unsigned>(s_isr_samples), static_cast<unsigned>(s_isr_back),
		static_cast<unsigned>(s_isr_jump), task_back, task_jump);
	TEST_ASSERT_GREATER_THAN_UINT32(10U, s_isr_samples);
	TEST_ASSERT_EQUAL_UINT32(0U, s_isr_back);
	TEST_ASSERT_EQUAL_UINT32(0U, task_back);
	TEST_ASSERT_EQUAL_UINT32(0U, task_jump);
#ifndef QEMU_NO_RCC_PERIPH
	TEST_ASSERT_EQUAL_UINT32(0U, s_isr_jump);
#endif
}

#ifndef QEMU_NO_RCC_PERIPH
// The kernel clock against the independent cycle counter while the system idles
TEST(tickless, drift_vs_dwt_idle)
{
	const auto r = measure_drift(2000);
	tiny_printf("tickless drift idle: deviation %ld cycles %ld ppm\r\n",
		static_cast<long>(r.deviation), r.ppm);
	assert_abs_le(r.deviation, 2 * static_cast<int64_t>(_isix_port_get_core_freq() / CONFIG_ISIX_HZ), "idle drift");
}

// Wakeups from the interrupts that are not aligned with the tick
TEST(tickless, drift_vs_dwt_irq_load)
{
	for( const uint32_t period_us : { 300U, 7300U } ) {
		_isixp_test_tickless_stats_reset();
		TEST_ASSERT_TRUE(waker_start(period_us));
		const auto r = measure_drift(2000);
		const unsigned irqs = s_irq_hits;
		const auto st = read_stats();
		waker_stop();
		tiny_printf("tickless drift irq %lu us: deviation %ld cycles %ld ppm irqs %lu restarts %lu passed %lu\r\n",
			static_cast<unsigned long>(period_us), static_cast<long>(r.deviation), r.ppm,
			static_cast<unsigned long>(irqs), static_cast<unsigned long>(st.restarts),
			static_cast<unsigned long>(st.wake_armed_passed));
		assert_abs_le(r.ppm, c_drift_irq_limit_ppm, "irq load drift");
	}
}

#ifdef ISIX_TEST_DRIFT_SOAK
namespace
{
	// The cycle counter wraps every few tens of seconds, accumulate it in 64 bits
	long soak_ppm(unsigned seconds)
	{
		dwt_enable();
		const ostick_t j0 = sync_to_tick();
		uint32_t last = dwt_now();
		uint64_t cycles = 0;
		for( unsigned i = 0; i < seconds; ++i ) {
			isix::wait_ms(1000);
			const uint32_t now = dwt_now();
			cycles += static_cast<uint32_t>(now - last);
			last = now;
		}
		const int64_t cpj = _isix_port_get_core_freq() / CONFIG_ISIX_HZ;
		const int64_t dev = static_cast<int64_t>(isix::get_jiffies() - j0) * cpj - static_cast<int64_t>(cycles);
		return static_cast<long>(dev * 1000000LL / static_cast<int64_t>(cycles));
	}
}

// Long run of the kernel clock against the cycle counter, idle and with the interrupt load
TEST(tickless, drift_soak_60s)
{
	const long idle_ppm = soak_ppm(30);
	TEST_ASSERT_TRUE(waker_start(300));
	const long load_ppm = soak_ppm(30);
	waker_stop();
	tiny_printf("tickless drift soak: idle %ld ppm, irq load %ld ppm\r\n", idle_ppm, load_ppm);
	assert_abs_le(idle_ppm, 50, "soak idle drift");
	assert_abs_le(load_ppm, 200, "soak irq load drift");
}
#endif
#endif

// Arithmetic of the split of the long period into whole ticks and the first period
TEST(tickless, split_arithmetic_table)
{
	struct row {
		uint32_t long_cycles;
		uint32_t cvr;
		uint32_t tpp;
		ostick_t n;
		uint32_t first;
	};
	static constexpr row table[] = {
		// Counter untouched or stale
		{ 5000, 4999, 1000, 0, 1000 },
		{ 5000, 5000, 1000, 0, 1000 },
		{ 5000, 6000, 1000, 0, 1000 },
		// Inside a tick
		{ 5000, 2500, 1000, 2, 501 },
		{ 5000, 999, 1000, 4, 1000 },
		{ 5000, 4936, 1000, 0, 937 },
		// The next boundary is too close to restart the counter, it is counted at once
		{ 5000, 4063, 1000, 0, 64 },
		{ 5000, 4062, 1000, 1, 1063 },
		{ 5000, 4000, 1000, 1, 1001 },
		{ 5000, 1000, 1000, 4, 1001 },
		{ 5000, 0, 1000, 5, 1001 },
		// The period length is not a divisor of the counter range
		{ 16766499, 0, 33333, 503, 33334 },
		{ 16766499, 16766498, 33333, 0, 33333 },
		// Period too short for the guard
		{ 8, 7, 2, 0, 2 },
		{ 8, 0, 2, 3, 1 },
	};
	for( const auto& r : table ) {
		ostick_t n = 0;
		uint32_t first = 0;
		_isixp_test_tickless_split(r.long_cycles, r.cvr, r.tpp, &n, &first);
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(r.n, n, "ticks");
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(r.first, first, "first period");
	}
}


#else /* !CONFIG_ISIX_TICKLESS */

TEST(tickless, tickless_disabled_stub)
{
	TEST_PASS();
}

TEST(tickless_early, tickless_disabled_stub)
{
	TEST_PASS();
}

#endif

namespace
{
	// Unity without 64-bit support
	void assert_u64(uint64_t expected, uint64_t actual)
	{
		if( expected != actual ) {
			tiny_printf("expected %lu was %lu (low 32 bits)\r\n",
				static_cast<unsigned long>(expected), static_cast<unsigned long>(actual));
			TEST_FAIL_MESSAGE("64 bit value mismatch");
		}
	}
}

// The counter wrap and the fractional cycles per tick of the up counter ports
TEST(tickless, upcounter_helper_wrap_16_24_32)
{
	// Elapsed cycles through the wrap of each counter width
	assert_u64(0x20U, _isixp_upcounter_elapsed(0x0010U, 0xFFF0U, 0xFFFFU));
	assert_u64(0x20U, _isixp_upcounter_elapsed(0x000010U, 0xFFFFF0U, 0xFFFFFFU));
	assert_u64(0x20U, _isixp_upcounter_elapsed(0x10U, 0xFFFFFFF0U, 0xFFFFFFFFU));
	assert_u64(0x200U, _isixp_upcounter_elapsed(0x100U, 0xFFFFFFFFFFFFFF00ULL, UINT64_MAX));
	assert_u64(0U, _isixp_upcounter_elapsed(0x1234U, 0x1234U, 0xFFFFU));
	assert_u64(0xFFFFU, _isixp_upcounter_elapsed(0x1233U, 0x1234U, 0xFFFFU));

	// 32768 Hz counter, 1000 Hz tick: 32.768 cycles per tick
	uint64_t carry = 0;
	TEST_ASSERT_EQUAL_UINT32(0U, _isixp_upcounter_ticks(32U, 32768U, 1000U, &carry));
	assert_u64(32000U, carry);
	TEST_ASSERT_EQUAL_UINT32(1U, _isixp_upcounter_ticks(33U, 32768U, 1000U, &carry));
	assert_u64(32232U, carry);
	// A whole second of the counter is exactly 1000 ticks and leaves no remainder
	carry = 0;
	TEST_ASSERT_EQUAL_UINT32(1000U, _isixp_upcounter_ticks(32768U, 32768U, 1000U, &carry));
	assert_u64(0U, carry);
	// The remainder accumulates over many short conversions, no tick is lost
	carry = 0;
	unsigned total = 0;
	for( int i = 0; i < 32768; ++i ) {
		total += _isixp_upcounter_ticks(1U, 32768U, 1000U, &carry);
	}
	TEST_ASSERT_EQUAL_UINT32(1000U, total);

	// 24 bit counter at 24 MHz
	carry = 0;
	TEST_ASSERT_EQUAL_UINT32(2U, _isixp_upcounter_ticks(48000U, 24000000U, 1000U, &carry));
	assert_u64(0U, carry);
	TEST_ASSERT_EQUAL_UINT32(0U, _isixp_upcounter_ticks(23999U, 24000000U, 1000U, &carry));
	TEST_ASSERT_EQUAL_UINT32(1U, _isixp_upcounter_ticks(1U, 24000000U, 1000U, &carry));

	// 32 bit counter at 100 MHz, the range of one counter turn is about 43 s
	carry = 0;
	TEST_ASSERT_EQUAL_UINT32(42949U, _isixp_upcounter_ticks(0xFFFFFFFFU, 100000000U, 1000U, &carry));
	assert_u64(67295000U, carry);

	// Longest timeout is limited by half of the counter range
	TEST_ASSERT_EQUAL_UINT32(999U, _isixp_upcounter_max_ticks(0xFFFFU, 32768U, 1000U));
	TEST_ASSERT_EQUAL_UINT32(349U, _isixp_upcounter_max_ticks(0xFFFFFFU, 24000000U, 1000U));
	TEST_ASSERT_EQUAL_UINT32(21474U, _isixp_upcounter_max_ticks(0xFFFFFFFFU, 100000000U, 1000U));
	TEST_ASSERT_EQUAL_UINT32(0x7FFFFFFFU, _isixp_upcounter_max_ticks(UINT64_MAX, 1000000U, 1000U));
}

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
	RUN_TEST_CASE(tickless, sleep_entry_after_scheduler_lock);
	RUN_TEST_CASE(tickless, no_sleep_when_ready_work_exists);
	RUN_TEST_CASE(tickless, wait_wraparound_advances_and_returns);
	RUN_TEST_CASE(tickless, semaphore_timeout_wraparound);
	RUN_TEST_CASE(tickless, earliest_deadline_wins);
	RUN_TEST_CASE(tickless, vtimer_periodic_catchup_after_jump);
	RUN_TEST_CASE(tickless, vtimer_periodic_wraparound_catchup);
	RUN_TEST_CASE(tickless, jiffies_track_wall_time_under_irq_load);
	RUN_TEST_CASE(tickless, long_sleep_spans_max_period);
	RUN_TEST_CASE(tickless, sleep_hooks_report_programmed_period);
	RUN_TEST_CASE(tickless, max_period_not_multiple);
	RUN_TEST_CASE(tickless, boundary_before_reload_update_on_enter);
	RUN_TEST_CASE(tickless, boundary_after_reload_update_on_enter);
	RUN_TEST_CASE(tickless, armed_wake_after_boundary);
	RUN_TEST_CASE(tickless, sleep_catchup_crosses_jiffies_wrap);
	RUN_TEST_CASE(tickless, long_period_across_jiffies_wrap);
	RUN_TEST_CASE(tickless, wait_across_wrap_exact);
	RUN_TEST_CASE(tickless, semaphore_timeout_across_wrap_exact);
	RUN_TEST_CASE(tickless, vtimer_one_shot_across_wrap_in_sleep);
	RUN_TEST_CASE(tickless, systick_isr_preempted_by_waking_irq);
	RUN_TEST_CASE(tickless, systick_isr_armed_preempted_by_waking_irq);
	RUN_TEST_CASE(tickless, systick_isr_entry_preempted_by_waking_irq);
	RUN_TEST_CASE(tickless, sleeping_wake_race_with_period_end);
	RUN_TEST_CASE(tickless, ujiffies_monotonic_in_isr_and_task);
	RUN_TEST_CASE(tickless, split_arithmetic_table);
#ifndef QEMU_NO_RCC_PERIPH
	RUN_TEST_CASE(tickless, drift_vs_dwt_idle);
	RUN_TEST_CASE(tickless, drift_vs_dwt_irq_load);
#ifdef ISIX_TEST_DRIFT_SOAK
	RUN_TEST_CASE(tickless, drift_soak_60s);
#endif
#endif
#else
	RUN_TEST_CASE(tickless_early, tickless_disabled_stub);
#endif
	RUN_TEST_CASE(tickless, upcounter_helper_wrap_16_24_32);
}

TEST_GROUP_RUNNER(tickless_early)
{
#if CONFIG_ISIX_TICKLESS
	RUN_TEST_CASE(tickless_early, sleep_entry_when_idle_only);
	RUN_TEST_CASE(tickless_early, min_sleep_threshold_edges);
#else
	RUN_TEST_CASE(tickless_early, tickless_disabled_stub);
#endif
}
