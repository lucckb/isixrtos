#define UNITY_FIXTURE_NO_EXTRAS
#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include "timer_interrupt.hpp"
#include <string>

namespace
{
	//Basic semaphore test
	class semaphore_task_test
	{
		static constexpr auto STACK_SIZE = 2048;
		static constexpr auto JOIN_TIMEOUT = 4000;
	public:
		semaphore_task_test(char ch_id, osprio_t prio, isix::semaphore &sem, std::string &items)
            : m_sem(sem), m_id(ch_id), m_items(items), m_prio(prio)
			, m_thr(isix::thread_create(std::bind(&semaphore_task_test::thread,std::ref(*this))))
		{
		}
		void start() {
			m_thr.start_thread(STACK_SIZE, m_prio);
		}
        int error() const {
			return m_error;
        }
		int join() {
			return m_join_sem.wait(JOIN_TIMEOUT);
		}
		int val() const {
			return m_sem.getval();
		}
		bool is_valid() const noexcept {
			return m_thr;
		}
		semaphore_task_test& operator=(semaphore_task_test&) = delete;
		semaphore_task_test(semaphore_task_test&) = delete;
	private:
		//Main funcs
		void thread()  noexcept
		{
            m_error = m_sem.wait(ISIX_TIME_INFINITE);
            m_items.push_back(m_id);
			m_join_sem.signal();
			//for (;;) isix::isix_wait_ms(1000);
		}
    private:
        isix::semaphore& m_sem;
        const char  m_id;
        std::string& m_items;
        int m_error { -32768 };
		osprio_t m_prio;
		isix::semaphore m_join_sem { 0, 1 };
		isix::thread m_thr;
	};

	//Semaphore time test
	class semaphore_time_task {
		static constexpr auto TASK_PRIO = 3;
		static constexpr auto STACK_SIZE = 2048;
		static constexpr auto sem_tout = 500;
	public:
		explicit semaphore_time_task(isix::semaphore& sem )
			: m_sem(sem)
			, m_thr(isix::thread_create(std::bind(&semaphore_time_task::thread,std::ref(*this))))
		{
		}
		void start() {
			m_thr.start_thread(STACK_SIZE, TASK_PRIO);
		}
        int error() const {
            m_notify_sem.wait(ISIX_TIME_INFINITE);
			return m_error;
        }
		int val() const {
			return m_sem.getval();
		}
		bool is_valid() const noexcept {
			return m_thr;
		}
		semaphore_time_task& operator=(semaphore_time_task&) = delete;
		semaphore_time_task(semaphore_time_task&) = delete;
	private:
		void thread() noexcept {
			for (;;)
			{
				m_error = m_sem.wait (sem_tout);
				m_notify_sem.signal();
			}
		}
	private:
		isix::semaphore &m_sem;
        int m_error { -32768 };
		mutable isix::semaphore m_notify_sem { 0, 1 };
		isix::thread m_thr;
	};
}


TEST_GROUP(semaphores);
TEST_SETUP(semaphores) {}
TEST_TEAR_DOWN(semaphores) {}

TEST(semaphores, timeout)
{
	isix::semaphore sigs(0);
	semaphore_time_task t1(sigs); t1.start();
	TEST_ASSERT(t1.is_valid());
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, t1.error());
	sigs.signal();
	TEST_ASSERT_EQUAL(ISIX_EOK, t1.error());
}

TEST(semaphores, priority)
{
	static constexpr auto TASKDEF_PRIORITY = 0;
	//ThreadRunner<MyThreadClass>
	static constexpr auto test_prio = 3;
	TEST_ASSERT_EQUAL(TASKDEF_PRIORITY, isix_task_change_prio(nullptr, test_prio));
	std::string tstr;
	isix::semaphore sigs(0);
	TEST_ASSERT(sigs.is_valid());
	semaphore_task_test t1('A', 3, sigs, tstr);
	semaphore_task_test t2('B', 2, sigs, tstr);
	semaphore_task_test t3('C', 1, sigs, tstr);
	semaphore_task_test t4('D', 0, sigs, tstr);
	t1.start(); t2.start(); t3.start(); t4.start();
	TEST_ASSERT(t1.is_valid());
	TEST_ASSERT(t2.is_valid());
	TEST_ASSERT(t3.is_valid());
	TEST_ASSERT(t4.is_valid());
	TEST_ASSERT_EQUAL(ISIX_EOK, sigs.signal());
	TEST_ASSERT_EQUAL(ISIX_EOK, sigs.signal());
	TEST_ASSERT_EQUAL(ISIX_EOK, sigs.signal());
	TEST_ASSERT_EQUAL(ISIX_EOK, sigs.signal());
	TEST_ASSERT_EQUAL(ISIX_EOK, t1.join());
	TEST_ASSERT_EQUAL(ISIX_EOK, t2.join());
	TEST_ASSERT_EQUAL(ISIX_EOK, t3.join());
	TEST_ASSERT_EQUAL(ISIX_EOK, t4.join());
	TEST_ASSERT_EQUAL_STRING("DCBA", tstr.c_str());
	//Check semaphore status
	TEST_ASSERT_EQUAL(ISIX_EOK, t1.error());
	TEST_ASSERT_EQUAL(ISIX_EOK, t2.error());
	TEST_ASSERT_EQUAL(ISIX_EOK, t3.error());
	TEST_ASSERT_EQUAL(ISIX_EOK, t4.error());
	TEST_ASSERT_EQUAL(test_prio, isix_task_change_prio(nullptr,TASKDEF_PRIORITY));
}

TEST(semaphores, reset_api)
{
	auto sigs = new isix::semaphore(0);
	std::string tstr;
	TEST_ASSERT(sigs->is_valid());
	semaphore_task_test t1('A', 3, *sigs, tstr);
	semaphore_task_test t2('B', 2, *sigs, tstr);
	semaphore_task_test t3('C', 1, *sigs, tstr);
	semaphore_task_test t4('D', 0, *sigs, tstr);
	t1.start(); t2.start(); t3.start(); t4.start();
	TEST_ASSERT(t1.is_valid());
	TEST_ASSERT(t2.is_valid());
	TEST_ASSERT(t3.is_valid());
	TEST_ASSERT(t4.is_valid());
	//! Give some time for add to sem
	isix::wait_ms(25);
	TEST_ASSERT_EQUAL(ISIX_EOK, sigs->reset(5));
	TEST_ASSERT_EQUAL(ISIX_EOK, t1.join());
	TEST_ASSERT_EQUAL(ISIX_EOK, t2.join());
	TEST_ASSERT_EQUAL(ISIX_EOK, t3.join());
	TEST_ASSERT_EQUAL(ISIX_EOK, t4.join());
	TEST_ASSERT_EQUAL_STRING("DCBA", tstr.c_str());
	//Check semaphore status
	TEST_ASSERT_EQUAL(ISIX_ERESET, t1.error());
	TEST_ASSERT_EQUAL(ISIX_ERESET, t2.error());
	TEST_ASSERT_EQUAL(ISIX_ERESET, t3.error());
	TEST_ASSERT_EQUAL(ISIX_ERESET, t4.error());
	TEST_ASSERT_EQUAL(5, t1.val());
	TEST_ASSERT_EQUAL(5, t2.val());
	TEST_ASSERT_EQUAL(5, t3.val());
	TEST_ASSERT_EQUAL(5, t4.val());
	// Try to wait and next delete
	delete sigs;
	sigs = new isix::semaphore(0);
	semaphore_task_test t5('Z', 2, *sigs, tstr); t5.start();
	semaphore_task_test t6('Y', 2, *sigs, tstr); t6.start();
	isix::wait_ms(25);
	delete sigs;
	TEST_ASSERT_EQUAL(ISIX_EOK, t5.join());
	TEST_ASSERT_EQUAL(ISIX_EOK, t6.join());
	TEST_ASSERT_EQUAL(ISIX_EDESTROY, t5.error());
	TEST_ASSERT_EQUAL(ISIX_EDESTROY, t6.error());
}

TEST(semaphores, interrupt_api)
{
	constexpr auto N_TEST_POSTS = 25;
	volatile int irq_get_isr_nposts {};
	volatile int test_count { }; //Post IRQ sem five times
	isix::semaphore m_sem_irq { 0, 0 };
	isix::semaphore m_sem_irq_get { 0, 0 };
	const auto isr_test_handler = [&]()
	{
		if (test_count++ < N_TEST_POSTS)
		{
			m_sem_irq.signal_isr();
		} else {
			while (m_sem_irq_get.trywait() == ISIX_EOK) {
				if (++irq_get_isr_nposts > N_TEST_POSTS*2) {
					break;
				}
			}
			tests::detail::periodic_timer_stop();
		}
	};
	int ret;
	//First push 5 items
	for (int i = 0; i < N_TEST_POSTS; ++i) {
		m_sem_irq_get.signal();
	}
	auto ec = tests::detail::periodic_timer_setup(isr_test_handler, 1300);
	TEST_ASSERT(ec);
	//Do loop waits for irq
	int n_signals;
	for (n_signals=0; (ret=m_sem_irq.wait(1000))==ISIX_EOK; ++n_signals) {}
	//Check the result
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, ret);
	TEST_ASSERT_EQUAL(N_TEST_POSTS, n_signals);
	//Check get isr result
	TEST_ASSERT_EQUAL(N_TEST_POSTS, irq_get_isr_nposts);
	isix::wait_ms(250);
}


TEST_GROUP_RUNNER(semaphores)
{
	RUN_TEST_CASE(semaphores, timeout);
	RUN_TEST_CASE(semaphores, priority);
	RUN_TEST_CASE(semaphores, reset_api);
	RUN_TEST_CASE(semaphores, interrupt_api);
}