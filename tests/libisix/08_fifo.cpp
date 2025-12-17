#define UNITY_FIXTURE_NO_EXTRAS
#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include "timer_interrupt.hpp"
#include <memory>


namespace {

	constexpr auto c_stack_margin = 100;
	class task_test
	{
	static constexpr auto STACK_SIZE = 1024;
	public:
		//Number of items per task
		static constexpr auto N_ITEMS = 4;
		//Constructor
		task_test(char ch_id, osprio_t prio, isix::fifo<char> &fifo)
			: m_fifo(fifo), m_id(ch_id), m_prio(prio)
			, m_thr(isix::thread_create(std::bind(&task_test::thread,std::ref(*this))))
		{
		}
		task_test(task_test&) = delete;
		task_test& operator=(task_test&) = delete;
		void start() {
			m_thr.start_thread(STACK_SIZE, m_prio);
		}
	private:
		//Main function from another task
		void thread() noexcept
		{
			//isix_wait_ms(100);
			for (int i = 0; i< N_ITEMS; ++i) {
				m_error = m_fifo.push(m_id);
				if (m_error) break;
			}
		}
	private:
		isix::fifo<char>& m_fifo;
		const char m_id;
		int m_error { -32768 };
		osprio_t m_prio;
		isix::thread m_thr;
	};
}


//Delivery class task
namespace {
	
	static constexpr auto IRQ_QTEST_SIZE = 64U;
	//Overflow task for testing the push
	class overflow_task
	{
	static constexpr auto STACK_SIZE = 2048;
	public:
		//Number of items per task
		static constexpr auto N_ITEMS = 4;
		//Constructor
		overflow_task(osprio_t prio, isix::fifo<int> &fifo)
			:  m_fifo(fifo), m_prio(prio)
			, m_thr(isix::thread_create(std::bind(&overflow_task::thread,std::ref(*this))))
		{
		}
		//Error code
		int error() const {
			return m_error;
		}
		//Start function
		void start() {
			m_thr.start_thread(STACK_SIZE, m_prio);
		}
		const auto tid() const noexcept {
			return m_thr.tid();
		}
	private:
		//Main function from another task
		void thread() noexcept
		{
			for (auto i=0U; i < IRQ_QTEST_SIZE; ++i) {
				m_error = m_fifo.push(i+1);
				if (m_error != ISIX_EOK) {
					break;
				}
			}
		}
	private:
		isix::fifo<int> &m_fifo;
		int m_error { -32768 };
		osprio_t m_prio;
		isix::thread m_thr;
	};
}

namespace {
	constexpr auto NOT_FROM_IRQ=0;
	constexpr auto TASKDEF_PRIORITY=0;
	isix::fifo<int> m_fifo_n { IRQ_QTEST_SIZE/2 };
	isix::fifo<int> m_fifo_noirq { IRQ_QTEST_SIZE/2, isix_fifo_f_noirq };
	volatile size_t m_irq_cnt {};
	volatile int m_last_irq_err {};

	//Verify incremental context
	template <typename T>
		bool verify_values(const std::vector<T>& vec, int scnt)
		{
			for (auto v : vec) {
				if (v != scnt) {
					return false;
				}
				++scnt;
			}
			return true;
		}
	void interrupt_handler(isix::fifo<int>& fifo) noexcept
	{
		if (m_irq_cnt++ < IRQ_QTEST_SIZE) {
			m_last_irq_err  = fifo.push_isr(m_irq_cnt);
			if (m_last_irq_err != ISIX_EOK) {
				tests::detail::periodic_timer_stop();
			}
		} else {
			tests::detail::periodic_timer_stop();
		}
	}

	void delivery_test(uint16_t time_irq, isix::fifo<int>& fifoin)
	{
		std::unique_ptr<overflow_task> task;
		if (time_irq != NOT_FROM_IRQ) {
			m_irq_cnt = 0;
			auto ec = tests::detail::periodic_timer_setup(
				std::bind(interrupt_handler,std::ref(m_fifo_n)), time_irq
			);
            TEST_ASSERT(ec);
		} else {
			task.reset(new overflow_task(isix_get_task_priority(nullptr), fifoin));
			task->start();
		}
		int val; int ret;
		std::vector<int> test_vec;
		for (int n=0; (ret=fifoin.pop(val,1000))==ISIX_EOK; ++n) {
			test_vec.push_back(val);
		}
		TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, ret);
		TEST_ASSERT_EQUAL_UINT(0, fifoin.size());

		//Final parameter check
		TEST_ASSERT_EQUAL_UINT(IRQ_QTEST_SIZE, test_vec.size());
		TEST_ASSERT(verify_values(test_vec, 1));
		TEST_ASSERT_EQUAL(int(ISIX_EOK), m_last_irq_err);
		tests::detail::periodic_timer_stop();
		if (time_irq != NOT_FROM_IRQ) {
			tests::detail::periodic_timer_stop();
		}
	}
}


TEST_GROUP(fifo);
TEST_SETUP(fifo) {}
TEST_TEAR_DOWN(fifo) {}

TEST(fifo, basic_delivery)
{
	int err;
	char ch;
	isix::fifo<char> fifo_tst(32);
	TEST_ASSERT(fifo_tst.is_valid());
	task_test t1('A', 3, fifo_tst);
	task_test t2('B', 2, fifo_tst);
	task_test t3('C', 1, fifo_tst);
	task_test t4('D', 0, fifo_tst);
	t1.start(); t2.start(); t3.start(); t4.start();
	std::string tbuf;
	for (; (err=fifo_tst.pop(ch, 1000))==ISIX_EOK; tbuf+=ch);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, err);
	TEST_ASSERT_EQUAL(task_test::N_ITEMS * 4U, tbuf.size());
	TEST_ASSERT_EQUAL_STRING("DDDDCCCCBBBBAAAA", tbuf.c_str());
	TEST_ASSERT_GREATER_THAN(c_stack_margin, isix::free_stack_space(nullptr));
	isix::wait_ms(20);
}

TEST(fifo, insert_overflow)
{
	static constexpr auto FIFO_SIZE = 64;
	isix::fifo<char> ovfifo(FIFO_SIZE);
	TEST_ASSERT(ovfifo.is_valid());
	int err {};
	for (size_t i = 0; i<FIFO_SIZE; ++i) {
		err = ovfifo.push('A');
		if (err != ISIX_EOK)
			break;
	}
	char ch;
	TEST_ASSERT_EQUAL(ISIX_EOK, err);
	TEST_ASSERT_EQUAL(FIFO_SIZE, ovfifo.size());
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, ovfifo.push('X',1000));
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, ovfifo.push('X',1000));
	TEST_ASSERT_EQUAL(ISIX_EOK, ovfifo.pop(ch));
	TEST_ASSERT_EQUAL_CHAR('A', ch);
	TEST_ASSERT_EQUAL(FIFO_SIZE -1, ovfifo.size());
	TEST_ASSERT_GREATER_THAN(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(fifo, irq_slow_delivery)
{
	delivery_test(65535 , m_fifo_n);
	TEST_ASSERT_GREATER_THAN(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(fifo, irq_fast_delivery)
{
	delivery_test(4000, m_fifo_n);
	TEST_ASSERT_GREATER_THAN(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(fifo, noirq_delivery)
{
	delivery_test(NOT_FROM_IRQ, m_fifo_n);
	TEST_ASSERT_GREATER_THAN(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(fifo, slow_fifo_after_noirq)
{
	delivery_test(65535, m_fifo_n);
	TEST_ASSERT_GREATER_THAN(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(fifo, not_from_irq_exchanged)
{
	delivery_test(NOT_FROM_IRQ, m_fifo_noirq);
	TEST_ASSERT_GREATER_THAN(c_stack_margin, isix::free_stack_space(nullptr));
}


TEST_GROUP_RUNNER(fifo)
{
	RUN_TEST_CASE(fifo, basic_delivery);
	RUN_TEST_CASE(fifo, insert_overflow);
	RUN_TEST_CASE(fifo, irq_slow_delivery);
	RUN_TEST_CASE(fifo, irq_fast_delivery);
	RUN_TEST_CASE(fifo, noirq_delivery);
	RUN_TEST_CASE(fifo, slow_fifo_after_noirq);
	RUN_TEST_CASE(fifo, not_from_irq_exchanged);
}