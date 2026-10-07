#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <stm32_ll_system.h>
#include <memory>
#include <isix/prv/test_hooks.h>
#include "timer_interrupt.hpp"
#include <utils/test_prio.hpp>

//Internal API MOCK
extern "C" {
	
	void _isixp_lock_scheduler();
	void _isixp_unlock_scheduler();
}

namespace
{
	//Task for testing lock scheduler
	class task
	{
	public:
		task& operator=(task&) = delete;
		task(task&) = delete;

		//Constructor
		explicit task(char pattern)
			: m_act_pattern(pattern)
			, m_thr(isix::thread_create(std::bind(&task::thread,std::ref(*this))))
		{
		}
		//Start task
		void start() {
			m_thr.start_thread(STACK_SIZE, THREAD_PRIO);
		}
		char get_id() const {
			return m_act_id;
		}
		void reset_id() {
			m_act_id = ' ';
		}
	private:
		//Main test task
		void thread() noexcept
		{
			for (;;) {
				m_act_id = m_act_pattern;
				asm volatile("nop\n");
				//isix::isix_wait_ms(1);
				asm volatile("nop\n");
				asm volatile("nop\n");
				asm volatile("nop\n");
				asm volatile("nop\n");
			}
		}
	private:
		static constexpr auto STACK_SIZE = 2048;
		static constexpr auto THREAD_PRIO = 0;
		volatile char m_act_id { ' ' };
		const char  m_act_pattern;
		isix::thread m_thr;
	};
}


namespace {
	test_utils::task_pool sl_tasks;
	volatile bool s_h_ran;
	ossem_t s_nest_sem;

	void nest_waiter(void*)
	{
		isix_sem_wait(s_nest_sem, 1000);
		s_h_ran = true;
	}
}

TEST_GROUP(sched_suspend);
TEST_SETUP(sched_suspend) {}
TEST_TEAR_DOWN(sched_suspend)
{
	tests::detail::periodic_timer_stop();
	sl_tasks.release();
	if (s_nest_sem) {
		isix_sem_destroy(s_nest_sem);
		s_nest_sem = nullptr;
	}
	test_utils::restore_test_prio();
}

TEST(sched_suspend, basic_lock)
{
	_isixp_lock_scheduler();
	for (int i=0;i<100;++i) {
		isix_yield();
	}
	_isixp_unlock_scheduler();
	_isixp_lock_scheduler();
	for (int i=0;i<10000000;++i)
		asm volatile("nop\n");
	_isixp_unlock_scheduler();
	for (int i=0; i<100; ++i) {
		isix_yield();
	}
	TEST_ASSERT(isix::is_scheduler_active());
}

TEST(sched_suspend, nested_lock)
{
	s_h_ran = false;
	s_nest_sem = isix_sem_create_limited(nullptr, 0, 1);
	TEST_ASSERT_NOT_NULL(s_nest_sem);
	TEST_ASSERT_NOT_NULL(sl_tasks.spawn(nest_waiter, nullptr, 1));
	test_utils::lower_test_prio(3);
	// Let the waiter block on the semaphore
	isix_wait_ms(5);
	_isixp_lock_scheduler();
	_isixp_lock_scheduler();
	isix_sem_signal(s_nest_sem);
	_isixp_unlock_scheduler();
	const bool early = s_h_ran;
	_isixp_unlock_scheduler();
	isix_wait_ms(5);
	TEST_ASSERT_FALSE(early);
	TEST_ASSERT_TRUE(s_h_ran);
}

TEST(sched_suspend, alloc_inside_lock)
{
	s_h_ran = false;
	s_nest_sem = isix_sem_create_limited(nullptr, 0, 1);
	TEST_ASSERT_NOT_NULL(s_nest_sem);
	TEST_ASSERT_NOT_NULL(sl_tasks.spawn(nest_waiter, nullptr, 1));
	test_utils::lower_test_prio(3);
	// Let the waiter block on the semaphore
	isix_wait_ms(5);
	_isixp_lock_scheduler();
	isix_sem_signal(s_nest_sem);
	void* const p = isix_alloc(16);
	isix_free(p);
	const bool early = s_h_ran;
	_isixp_unlock_scheduler();
	isix_wait_ms(5);
	TEST_ASSERT_NOT_NULL(p);
	TEST_ASSERT_FALSE(early);
	TEST_ASSERT_TRUE(s_h_ran);
}

TEST(sched_suspend, basic_resched)
{
	task t1('A');
	task t2('B');
	task t3('C');
	task t4('D');
	t1.start();
	t2.start();
	t3.start();
	t4.start();
	isix_wait_ms(500);
	TEST_ASSERT_EQUAL_CHAR('A', t1.get_id());
	TEST_ASSERT_EQUAL_CHAR('B', t2.get_id());
	TEST_ASSERT_EQUAL_CHAR('C', t3.get_id());
	TEST_ASSERT_EQUAL_CHAR('D', t4.get_id());
}

TEST(sched_suspend, tasks_reordering)
{
	auto t1 = std::make_unique<task>('A');
	auto t2 = std::make_unique<task>('B');
	auto t3 = std::make_unique<task>('C');
	auto t4 = std::make_unique<task>('D');
	TEST_ASSERT_EQUAL_UINT(0U, reinterpret_cast<unsigned>(t1.get()) % 4);
	TEST_ASSERT_EQUAL_UINT(0U, reinterpret_cast<unsigned>(t2.get()) % 4);
	TEST_ASSERT_EQUAL_UINT(0U, reinterpret_cast<unsigned>(t3.get()) % 4);
	TEST_ASSERT_EQUAL_UINT(0U, reinterpret_cast<unsigned>(t4.get()) % 4);
	_isixp_lock_scheduler();
	t1->start();
	t2->start();
	t3->start();
	t4->start();
	for (int i=0;i<1000000;++i) asm volatile("nop\n");
	TEST_ASSERT_EQUAL_CHAR(' ', t1->get_id());
	TEST_ASSERT_EQUAL_CHAR(' ', t2->get_id());
	TEST_ASSERT_EQUAL_CHAR(' ', t3->get_id());
	TEST_ASSERT_EQUAL_CHAR(' ', t4->get_id());
	_isixp_unlock_scheduler();
}

TEST(sched_suspend, unlock_replays_skipped_ticks)
{
	//SysTick current value register, counts down and reloads
	auto* const systick_val = reinterpret_cast<volatile uint32_t*>(0xE000E018UL);
	constexpr auto wraps_needed = 3;
	const auto j0 = isix_get_jiffies();
	_isixp_lock_scheduler();
	auto prev = *systick_val;
	for (int wraps=0; wraps<wraps_needed;) {
		const auto cur = *systick_val;
		if (cur > prev) {
			++wraps;
		}
		prev = cur;
	}
	_isixp_unlock_scheduler();
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(2U, isix_get_jiffies() - j0);
}

TEST(sched_suspend, ujiffies)
{
	//Test 1
	auto t1 = isix_get_jiffies();
	isix_wait_us(5000);
	auto t2 = isix_get_jiffies();
	TEST_ASSERT_UINT_WITHIN(1, 5U, t2-t1);
	//Test2 long
	t1 = isix_get_jiffies();
	isix_wait_us(500000);
	t2 = isix_get_jiffies();
	TEST_ASSERT_UINT_WITHIN(1, 500U, t2-t1);
	//Final give a chance to cleanup resources
	isix::wait_ms(10);
}


TEST(sched_suspend, isr_never_sees_open_critical_section)
{
	static volatile unsigned violations;
	static volatile unsigned irq_count;
	violations = 0;
	irq_count = 0;
	const auto ok = tests::detail::periodic_timer_setup([]() {
		if (_isixp_test_critical_count() != 0) {
			violations = violations + 1;
		}
		irq_count = irq_count + 1;
	}, 20);
	TEST_ASSERT_TRUE(ok);
	for (auto n = 0U; n < 200000U; ++n) {
		isix_enter_critical();
		isix_exit_critical();
	}
	tests::detail::periodic_timer_stop();
	const auto viol = violations;
	const auto irqs = irq_count;
	TEST_ASSERT_GREATER_THAN_UINT(100U, irqs);
	TEST_ASSERT_EQUAL_UINT(0U, viol);
}

TEST_GROUP_RUNNER(sched_suspend)
{
	RUN_TEST_CASE(sched_suspend, isr_never_sees_open_critical_section);
	RUN_TEST_CASE(sched_suspend, basic_lock);
	RUN_TEST_CASE(sched_suspend, nested_lock);
	RUN_TEST_CASE(sched_suspend, alloc_inside_lock);
	RUN_TEST_CASE(sched_suspend, basic_resched);
	RUN_TEST_CASE(sched_suspend, tasks_reordering);
	RUN_TEST_CASE(sched_suspend, unlock_replays_skipped_ticks);
	RUN_TEST_CASE(sched_suspend, ujiffies);
}