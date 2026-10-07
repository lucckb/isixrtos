#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <isix/arch/sem_atomic.h>
#include <foundation/sys/dbglog.h>
#include "timer_interrupt.hpp"
#include <utils/tickless_testhooks.h>

TEST_GROUP(basic_primitives);
TEST_SETUP(basic_primitives) {}
TEST_TEAR_DOWN(basic_primitives)
{
	tests::detail::periodic_timer_stop();
}

// The emulated TIM3 and SysTick run on unrelated clocks, only the 1 kHz tick matches
#if !defined(QEMU_NO_RCC_PERIPH) || CONFIG_ISIX_HZ == 1000
#define TIME_BASE_TEST 1
#endif

#if TIME_BASE_TEST && CONFIG_ISIX_TICKLESS
namespace
{
	static volatile bool s_busy_run {};

	static void busy_task(void*)
	{
		while( s_busy_run ) {
			asm volatile("nop");
		}
	}
}
#endif

#if TIME_BASE_TEST
TEST(basic_primitives, time_base_timer_vs_systick)
{
	static constexpr auto period_us = 1000U;
	unsigned cnt = 0;
	auto ec = tests::detail::periodic_timer_setup([&cnt]() {
		++cnt;
	}, period_us);
	TEST_ASSERT(ec);

#if CONFIG_ISIX_TICKLESS
	/* In tickless mode, long idle windows can coalesce periodic IRQs,
	 * which makes exact cnt vs. wall-time assertions unreliable.
	 * Keep CPU active so tickless idle cannot trigger. */
	s_busy_run = true;
	auto t = isix::task_create(busy_task, nullptr, 2048, 2, 0);
	TEST_ASSERT_NOT_NULL(t);
#endif

	isix::wait_ms(1000);
	tests::detail::periodic_timer_stop();

#if CONFIG_ISIX_TICKLESS
	s_busy_run = false;
	isix::task_kill(t);
#endif
#ifdef QEMU_NO_RCC_PERIPH
	// The emulator can stall the tick for several milliseconds under host load
	static constexpr auto tolerance = 30U;
#else
	static constexpr auto tolerance = 5U;
#endif
	TEST_ASSERT_UINT_WITHIN(tolerance, period_us, cnt);
}
#endif

TEST(basic_primitives, basic_heap_allocator)
{
	TEST_ASSERT_EQUAL(0, reinterpret_cast<long>(isix::task_self())%4);
	auto ptr1 = isix_alloc(1);
	auto ptr2 = isix_alloc(1);
	TEST_ASSERT_NOT_NULL(ptr1);
	TEST_ASSERT_NOT_NULL(ptr2);
	TEST_ASSERT_EQUAL(0, reinterpret_cast<long>(ptr1)%ISIX_BYTE_ALIGNMENT_SIZE);
	TEST_ASSERT_EQUAL(0, reinterpret_cast<long>(ptr2)%ISIX_BYTE_ALIGNMENT_SIZE);

	isix::memory_stat mstat_before_free;
	isix::heap_stats(mstat_before_free);
	TEST_ASSERT_GREATER_THAN_size_t(0, mstat_before_free.free);

	if (ptr1) isix_free(ptr1);
	if (ptr2) isix_free(ptr2);

	isix::memory_stat mstat;
	isix::heap_stats(mstat);
	TEST_ASSERT_GREATER_THAN_size_t(mstat_before_free.free, mstat.free);
}

TEST(basic_primitives, atomic_semaphore)
{
	_isix_port_atomic_sem_t sem;
	_isix_port_atomic_sem_init(&sem, 1, sys_atomic_unlimited_value);
	//Basic aritmetic tests
	TEST_ASSERT_EQUAL(1, sem.value);
	TEST_ASSERT_EQUAL(2, _isix_port_atomic_sem_inc(&sem));
	TEST_ASSERT_EQUAL(2, sem.value);
	TEST_ASSERT_EQUAL(3, _isix_port_atomic_sem_inc(&sem));
	TEST_ASSERT_EQUAL(3, sem.value);
	TEST_ASSERT_EQUAL(2, _isix_port_atomic_sem_dec(&sem));
	TEST_ASSERT_EQUAL(2, sem.value);
	_isix_port_atomic_sem_dec(&sem);
	TEST_ASSERT_EQUAL(1, sem.value);
	TEST_ASSERT_EQUAL(0, _isix_port_atomic_sem_dec(&sem));
	TEST_ASSERT_EQUAL(-1, _isix_port_atomic_sem_dec(&sem));
	TEST_ASSERT_EQUAL(-1, sem.value);
	TEST_ASSERT_EQUAL(-2, _isix_port_atomic_sem_dec(&sem));
	TEST_ASSERT_EQUAL(-2, sem.value);
	// Testing try wait
	TEST_ASSERT_EQUAL(-2, _isix_port_atomic_sem_trydec(&sem));
	TEST_ASSERT_EQUAL(-2, sem.value);
	//UP to 0
	TEST_ASSERT_EQUAL(-1, _isix_port_atomic_sem_inc(&sem));
	TEST_ASSERT_EQUAL(0, _isix_port_atomic_sem_inc(&sem));
	//Testing on 0 level
	TEST_ASSERT_EQUAL(0, _isix_port_atomic_sem_trydec(&sem));
	TEST_ASSERT_EQUAL(0, sem.value);
	//Testing on 1 level should be changed to 0
	TEST_ASSERT_EQUAL(1, _isix_port_atomic_sem_inc(&sem));
	//Testing on 1 level should be 0 again
	TEST_ASSERT_EQUAL(1, _isix_port_atomic_sem_trydec(&sem));
	TEST_ASSERT_EQUAL(0, sem.value);
	//Testing for upper limit value
	_isix_port_atomic_sem_t lsem;
	_isix_port_atomic_sem_init(&lsem, 0, 3);
	TEST_ASSERT_EQUAL(1, _isix_port_atomic_sem_inc(&lsem));
	TEST_ASSERT_EQUAL(1, lsem.value);
	TEST_ASSERT_EQUAL(2, _isix_port_atomic_sem_inc(&lsem));
	TEST_ASSERT_EQUAL(2, lsem.value);
	TEST_ASSERT_EQUAL(3, _isix_port_atomic_sem_inc(&lsem));
	TEST_ASSERT_EQUAL(3, lsem.value);
	TEST_ASSERT_EQUAL(3, _isix_port_atomic_sem_inc(&lsem));
	TEST_ASSERT_EQUAL(3, lsem.value);
	TEST_ASSERT_EQUAL(3, _isix_port_atomic_sem_inc(&lsem));
	TEST_ASSERT_EQUAL(3, lsem.value);
	TEST_ASSERT_EQUAL(2, _isix_port_atomic_sem_dec(&lsem));
	TEST_ASSERT_EQUAL(2, lsem.value);
}

TEST(basic_primitives, heap_getsize_reports_block_size)
{
	static char static_buf[16];
	auto ptr = static_cast<char*>(isix_alloc(100));
	TEST_ASSERT_NOT_NULL(ptr);
	const auto sz = isix_heap_getsize(ptr);
	TEST_ASSERT_GREATER_OR_EQUAL_size_t(100, sz);
	TEST_ASSERT_LESS_THAN_size_t(100 + 64, sz);
	TEST_ASSERT_EQUAL_size_t(0, isix_heap_getsize(static_buf));
	isix_free(ptr);
}

TEST(basic_primitives, timer_elapsed_across_jiffies_wrap)
{
	_isixp_test_set_jiffies(0xFFFFFFFFU - 4U);
	const auto t1 = isix_get_jiffies();
	// Wait for the wrap using the raw counter
	while (isix_get_jiffies() >= t1) {
		asm volatile("nop\n");
	}
	TEST_ASSERT_FALSE(isix_timer_elapsed(t1, 20));
	TEST_ASSERT_TRUE(isix_timer_elapsed(t1, 2));
}

TEST_GROUP_RUNNER(basic_primitives)
{
	RUN_TEST_CASE(basic_primitives, timer_elapsed_across_jiffies_wrap)
	RUN_TEST_CASE(basic_primitives, heap_getsize_reports_block_size)
#if TIME_BASE_TEST
	RUN_TEST_CASE(basic_primitives, time_base_timer_vs_systick)
#endif
	RUN_TEST_CASE(basic_primitives, basic_heap_allocator)
	RUN_TEST_CASE(basic_primitives, atomic_semaphore)
}