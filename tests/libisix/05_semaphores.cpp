#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include "timer_interrupt.hpp"
#include <stm32_ll_tim.h>
#include <string>
#include <memory>
#include <utils/test_prio.hpp>
#include <isix/prv/test_hooks.h>

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

namespace {
	test_utils::task_pool sem_tasks;
	ossem_t sem_obj;
	osevent_t hook_event;
	volatile int sem_order[4];
	volatile int sem_order_n;
	volatile int sem_res[4];
	volatile bool sem_done;
	struct sem_waiter { int id; };
	sem_waiter sem_waiters[4] { {0}, {1}, {2}, {3} };

	void sem_waiter_fn(void* arg)
	{
		const auto id = static_cast<sem_waiter*>(arg)->id;
		const auto res = isix_sem_wait(sem_obj, 3000);
		sem_res[id] = res;
		sem_order[sem_order_n] = id;
		sem_order_n = sem_order_n + 1;
	}
}
TEST_TEAR_DOWN(semaphores)
{
	tests::detail::periodic_timer_stop();
	test_utils::restore_test_prio();
	_isixp_test_hook = nullptr;
	sem_tasks.release();
	if (sem_obj) { isix_sem_destroy(sem_obj); sem_obj = nullptr; }
	sem_order_n = 0;
}

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
	auto sigs = std::make_unique<isix::semaphore>(0);
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
	sigs = std::make_unique<isix::semaphore>(0);
	semaphore_task_test t5('Z', 2, *sigs, tstr); t5.start();
	semaphore_task_test t6('Y', 2, *sigs, tstr); t6.start();
	isix::wait_ms(25);
	sigs.reset();
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


TEST(semaphores, change_prio_of_queued_waiter)
{
	sem_obj = isix_sem_create(nullptr, 0);
	TEST_ASSERT_NOT_NULL(sem_obj);
	sem_order_n = 0;
	const auto t3 = sem_tasks.spawn(sem_waiter_fn, &sem_waiters[0], 3);
	const auto t5 = sem_tasks.spawn(sem_waiter_fn, &sem_waiters[1], 5);
	const auto t7 = sem_tasks.spawn(sem_waiter_fn, &sem_waiters[2], 7);
	TEST_ASSERT(t3 && t5 && t7);
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(-3, isix_sem_getval(sem_obj));
	isix_task_change_prio(t7, 4);
	for (auto i = 0; i < 3; ++i) {
		TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_signal(sem_obj));
		isix::wait_ms(10);
	}
	TEST_ASSERT_EQUAL(3, sem_order_n);
	// Waiters are released in priority order 3, 4 (was 7), 5
	TEST_ASSERT_EQUAL(0, sem_order[0]);
	TEST_ASSERT_EQUAL(2, sem_order[1]);
	TEST_ASSERT_EQUAL(1, sem_order[2]);
	for (auto i = 0; i < 3; ++i) {
		TEST_ASSERT_EQUAL(ISIX_EOK, sem_res[i]);
	}
}

TEST(semaphores, suspend_resume_waiter_does_not_get_token)
{
	sem_obj = isix_sem_create(nullptr, 0);
	TEST_ASSERT_NOT_NULL(sem_obj);
	sem_order_n = 0;
	const auto t = sem_tasks.spawn(sem_waiter_fn, &sem_waiters[0], 3);
	TEST_ASSERT_NOT_NULL(t);
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(-1, isix_sem_getval(sem_obj));
	isix_task_suspend(t);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_resume(t));
	isix::wait_ms(20);
	// The waiter must keep waiting after suspend and resume
	TEST_ASSERT_EQUAL(0, sem_order_n);
	TEST_ASSERT_EQUAL(-1, isix_sem_getval(sem_obj));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_signal(sem_obj));
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(1, sem_order_n);
	TEST_ASSERT_EQUAL(ISIX_EOK, sem_res[0]);
	TEST_ASSERT_EQUAL(0, isix_sem_getval(sem_obj));
}

TEST(semaphores, suspend_resume_waiter_keeps_timeout)
{
	sem_obj = isix_sem_create(nullptr, 0);
	TEST_ASSERT_NOT_NULL(sem_obj);
	sem_order_n = 0;
	// The waiter uses 3000 ms, shorten with a dedicated task
	static constexpr auto short_wait = [](void*) {
		sem_res[0] = isix_sem_wait(sem_obj, isix::ms2tick(100));
		sem_order_n = sem_order_n + 1;
	};
	const auto t = sem_tasks.spawn(short_wait, nullptr, 3);
	TEST_ASSERT_NOT_NULL(t);
	isix::wait_ms(30);
	isix_task_suspend(t);
	isix_task_resume(t);
	isix::wait_ms(30);
	TEST_ASSERT_EQUAL(0, sem_order_n);
	isix::wait_ms(100);
	TEST_ASSERT_EQUAL(1, sem_order_n);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, sem_res[0]);
	TEST_ASSERT_EQUAL(0, isix_sem_getval(sem_obj));
}

TEST(semaphores, wait_dontwait_returns_immediately)
{
	sem_obj = isix_sem_create(nullptr, 0);
	TEST_ASSERT_NOT_NULL(sem_obj);
	sem_done = false;
	sem_res[0] = 12345;
	static constexpr auto dontwait = [](void*) {
		sem_res[0] = isix_sem_wait(sem_obj, ISIX_TIME_DONTWAIT);
		sem_done = true;
	};
	const auto t = sem_tasks.spawn(dontwait, nullptr, 3);
	TEST_ASSERT_NOT_NULL(t);
	isix::wait_ms(20);
	const auto done = sem_done;
	const auto res = sem_res[0];
	const auto val = isix_sem_getval(sem_obj);
	TEST_ASSERT_TRUE(done);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, res);
	TEST_ASSERT_EQUAL(0, val);
	// With a token available the call succeeds
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_signal(sem_obj));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_wait(sem_obj, ISIX_TIME_DONTWAIT));
	TEST_ASSERT_EQUAL(0, isix_sem_getval(sem_obj));
}

namespace {
	constexpr auto isr_lat_samples = 10U;
	constexpr auto isr_tick_us = 1000000U / ISIX_HZ;
	constexpr auto isr_lat_slow_us = isr_tick_us / 2U;
	volatile uint32_t isr_lat_t_irq;
	volatile unsigned isr_lat_slow;
	volatile unsigned isr_lat_done;
	volatile bool isr_lat_armed;
}

TEST(semaphores, isr_signal_wakes_higher_prio_within_tick)
{
	sem_obj = isix_sem_create(nullptr, 0);
	TEST_ASSERT_NOT_NULL(sem_obj);
	isr_lat_slow = 0;
	isr_lat_done = 0;
	isr_lat_armed = false;
	static constexpr auto waiter = [](void*) {
		while (isix_sem_wait(sem_obj, 1000) == ISIX_EOK) {
			const uint32_t lat = isix_get_ujiffies() - isr_lat_t_irq;
			if (lat >= isr_lat_slow_us) isr_lat_slow = isr_lat_slow + 1;
			isr_lat_done = isr_lat_done + 1;
		}
	};
	TEST_ASSERT_NOT_NULL(sem_tasks.spawn(waiter, nullptr, 1));
	isix::wait_ms(5);
	// The timer only installs the interrupt handler, the interrupt is raised from the test
	const auto ok = tests::detail::periodic_timer_setup([]() {
		if (isr_lat_armed) {
			isr_lat_armed = false;
			isr_lat_t_irq = isix_get_ujiffies();
			isix_sem_signal_isr(sem_obj);
		}
	}, 1000000);
	TEST_ASSERT_TRUE(ok);
	test_utils::lower_test_prio(5);
	for (auto i = 0U; i < isr_lat_samples; ++i) {
		// Raise the interrupt early in the system tick period
		while (isix_get_ujiffies() % isr_tick_us < isr_tick_us / 5U
			|| isix_get_ujiffies() % isr_tick_us > isr_tick_us * 3U / 10U) {
			asm volatile("nop\n");
		}
		isr_lat_armed = true;
		NVIC_SetPendingIRQ(TIM3_IRQn);
		const auto t0 = isix_get_jiffies();
		while (isr_lat_done <= i && !isix_timer_elapsed(t0, 5)) {
			asm volatile("nop\n");
		}
	}
	tests::detail::periodic_timer_stop();
	const auto done = isr_lat_done;
	const auto slow = isr_lat_slow;
	TEST_ASSERT_EQUAL_UINT(isr_lat_samples, done);
	// A woken higher priority task must not wait for the next system tick
	TEST_ASSERT_LESS_THAN_UINT(3U, slow);
}

TEST(semaphores, notify_reads_event_inside_critical)
{
	sem_obj = isix_sem_create(nullptr, 0);
	osevent_t ev = isix_event_create();
	TEST_ASSERT_TRUE(sem_obj && ev);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_event_connect(sem_obj, ev, 4));
	// Disconnect the event in the window after the signal has left the critical section
	_isixp_test_hook = [](isix_test_point point, void* arg) {
		if (point == isix_tp_sem_signal_notify && arg == sem_obj) {
			isix_sem_event_disconnect(sem_obj, hook_event);
		}
	};
	hook_event = ev;
	isix_sem_signal(sem_obj);
	_isixp_test_hook = nullptr;
	const auto bits = isix_event_get(ev);
	isix_event_destroy(ev);
	// The connection valid at the signal time decides about the notification
	TEST_ASSERT_EQUAL_UINT(1U << 4, bits);
}

TEST_GROUP_RUNNER(semaphores)
{
	RUN_TEST_CASE(semaphores, notify_reads_event_inside_critical);
	RUN_TEST_CASE(semaphores, isr_signal_wakes_higher_prio_within_tick);
	RUN_TEST_CASE(semaphores, change_prio_of_queued_waiter);
	RUN_TEST_CASE(semaphores, suspend_resume_waiter_does_not_get_token);
	RUN_TEST_CASE(semaphores, suspend_resume_waiter_keeps_timeout);
	RUN_TEST_CASE(semaphores, wait_dontwait_returns_immediately);
	RUN_TEST_CASE(semaphores, timeout);
	RUN_TEST_CASE(semaphores, priority);
	RUN_TEST_CASE(semaphores, reset_api);
	RUN_TEST_CASE(semaphores, interrupt_api);
}