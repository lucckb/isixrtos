#define UNITY_FIXTURE_NO_EXTRAS
#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <isix/arch/sem_atomic.h>
#include <foundation/sys/dbglog.h>
#include "timer_interrupt.hpp"

TEST_GROUP(basic_primitives);
TEST_SETUP(basic_primitives) {}
TEST_TEAR_DOWN(basic_primitives) {}

TEST(basic_primitives, time_base_timer_vs_systick)
{
	static constexpr auto period_us = 1000U;
	unsigned cnt = 0;
	auto ec = tests::detail::periodic_timer_setup([&cnt]() {
		++cnt;
	}, period_us);
	TEST_ASSERT(ec);
	isix::wait_ms(1000);
	tests::detail::periodic_timer_stop();
	TEST_ASSERT_UINT_WITHIN(5, period_us, cnt);
}

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

TEST_GROUP_RUNNER(basic_primitives)
{
	RUN_TEST_CASE(basic_primitives, time_base_timer_vs_systick)
	RUN_TEST_CASE(basic_primitives, basic_heap_allocator)
	RUN_TEST_CASE(basic_primitives, atomic_semaphore)
}