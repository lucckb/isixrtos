#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <stm32_ll_system.h>
#include <memory>

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


TEST_GROUP(sched_suspend);
TEST_SETUP(sched_suspend) {}
TEST_TEAR_DOWN(sched_suspend) {}

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


TEST_GROUP_RUNNER(sched_suspend)
{
	RUN_TEST_CASE(sched_suspend, basic_lock);
	RUN_TEST_CASE(sched_suspend, basic_resched);
	RUN_TEST_CASE(sched_suspend, tasks_reordering);
	RUN_TEST_CASE(sched_suspend, ujiffies);
}