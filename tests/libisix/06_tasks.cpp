#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include "task_test_helper.h"
#include "utils/fpu_test_and_set.h"
#include "utils/timer_interrupt.hpp"
#include <memory>

namespace
{
	static const auto BASE_TASK_PRIO = 1;
	//Test basic task functionality
	class base_task_tests {
		static constexpr auto STACK_SIZE = 1024;
		volatile unsigned m_exec_count {};
		volatile bool m_req_selfsusp {};
		//Main function
		void thread() noexcept
		{
			for (;;) {
				++m_exec_count;
				if (m_req_selfsusp) {
					m_req_selfsusp = false;
					isix::task_suspend(nullptr);
				}
			}
		}
	public:
		base_task_tests()
			: m_thr(isix::thread_create(std::bind(&base_task_tests::thread,std::ref(*this))))
		{
		}
		base_task_tests(base_task_tests&) = delete;
		base_task_tests& operator=(base_task_tests&) = delete;
		void start() {
			m_thr.start_thread(STACK_SIZE, BASE_TASK_PRIO);
		}
		void selfsuspend() {
			m_req_selfsusp = true;
		}
		unsigned exec_count() const {
			return m_exec_count;
		}
		void exec_count(unsigned v) {
			m_exec_count = v;
		}
		auto tid() const noexcept {
			return m_thr.tid();
		}
	private:
		isix::thread m_thr;
	};

	namespace thr11 {
		bool fin2 = false;
		void thread2_func()
		{
			fin2 = false;
			for (int i=0;i<5;++i) {
				isix::wait_ms(100);
			}
			fin2 = true;
		}
	}
	namespace waitref  {
		void task_ref(void*) {
			for (int i=0;i<5;++i) {
				isix::wait_ms(100);
			}
		}
		void task_ref2(void* arg)  {
			ostask_t t = reinterpret_cast<ostask_t>(arg);
			if (isix::task_wait_for (t) != ISIX_EOK) std::abort();
			isix::wait_ms(50);
		}
	}
}

namespace
{
	constexpr auto c_stack_size = ISIX_MIN_STACK_SIZE*2;
	constexpr auto c_task_prio = 3;
	constexpr auto c_stack_margin = 100;
}


TEST_GROUP(tasks);
TEST_SETUP(tasks) {}
TEST_TEAR_DOWN(tasks) {}

TEST(tasks, basic_api)
{
	static constexpr auto MIN_STACK_FREE = 64U;
	//Check if scheduler is running
	TEST_ASSERT(isix_is_scheduler_active()==true);
	auto t1 = std::make_unique<base_task_tests>();
	auto t2 = std::make_unique<base_task_tests>();
	auto t3 = std::make_unique<base_task_tests>();
	auto t4 = std::make_unique<base_task_tests>();
	t1->start(); t2->start(); t3->start(); t4->start();
	//Active wait tasks shouldnt run
	for (auto tc = isix_get_jiffies(); isix_get_jiffies()<tc+5000;) {
		asm volatile("nop\n");
	}
	TEST_ASSERT_EQUAL_UINT(0U, t1->exec_count());
	TEST_ASSERT_EQUAL_UINT(0U, t2->exec_count());
	TEST_ASSERT_EQUAL_UINT(0U, t3->exec_count());
	TEST_ASSERT_EQUAL_UINT(0U, t4->exec_count());
	//Now goto sleep
	isix_wait_ms(5000);
	//TASK should run now
	TEST_ASSERT_GREATER_THAN_UINT(0U, t1->exec_count());
	TEST_ASSERT_GREATER_THAN_UINT(0U, t2->exec_count());
	TEST_ASSERT_GREATER_THAN_UINT(0U, t3->exec_count());
	TEST_ASSERT_GREATER_THAN_UINT(0U, t4->exec_count());
	//Zero task count.. change prio and go active wait
	t1->exec_count(0);
	t2->exec_count(0);
	t3->exec_count(0);
	t4->exec_count(0);
	TEST_ASSERT_EQUAL_UINT(0U, t1->exec_count());
	TEST_ASSERT_EQUAL_UINT(0U, t2->exec_count());
	TEST_ASSERT_EQUAL_UINT(0U, t3->exec_count());
	TEST_ASSERT_EQUAL_UINT(0U, t4->exec_count());
	TEST_ASSERT_EQUAL_UINT(BASE_TASK_PRIO, isix_task_change_prio(t1->tid(),0));
	TEST_ASSERT_EQUAL_UINT(BASE_TASK_PRIO, isix_task_change_prio(t2->tid(),0));
	TEST_ASSERT_EQUAL_UINT(BASE_TASK_PRIO, isix_task_change_prio(t3->tid(),0));
	TEST_ASSERT_EQUAL_UINT(BASE_TASK_PRIO, isix_task_change_prio(t4->tid(),0));
	//Active wait tasks should doesn't run
	for (auto tc = isix_get_jiffies(); isix_get_jiffies()<tc+5000;) {
		asm volatile("nop\n");
	}
	//TASK should run now
	TEST_ASSERT_GREATER_THAN_UINT(0U, t1->exec_count());
	TEST_ASSERT_GREATER_THAN_UINT(0U, t4->exec_count());
	//Validate stack space functionality
	TEST_ASSERT_GREATER_THAN(ssize_t(MIN_STACK_FREE), isix_free_stack_space(t1->tid()));
	TEST_ASSERT_GREATER_THAN(ssize_t(MIN_STACK_FREE), isix_free_stack_space(t2->tid()));
	TEST_ASSERT_GREATER_THAN(ssize_t(MIN_STACK_FREE), isix_free_stack_space(t3->tid()));
	TEST_ASSERT_GREATER_THAN(ssize_t(MIN_STACK_FREE), isix_free_stack_space(t4->tid()));
	TEST_ASSERT_GREATER_THAN(ssize_t(MIN_STACK_FREE), isix_free_stack_space(nullptr));
	//! Get task state should be ready or running
	auto state = isix::get_task_state(t1->tid());
	TEST_ASSERT(state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING);
	state = isix::get_task_state(t2->tid());
	TEST_ASSERT(state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING);
	state = isix::get_task_state(t3->tid());
	TEST_ASSERT(state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING);
	state = isix::get_task_state(t4->tid());
	TEST_ASSERT(state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING);
	//! Sleep the task and check it state
	isix::task_suspend(t4->tid());
	//! Suspend special for delete
	isix::task_suspend(t1->tid());
	auto old_count = t4->exec_count();
	state = isix::get_task_state(t4->tid());
	TEST_ASSERT_EQUAL(OSTHR_STATE_SUSPEND, state);
	isix::wait_ms(50);
	//! Resume the task now
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::task_resume(t4->tid()));
	isix::wait_ms(50);
	TEST_ASSERT_GREATER_THAN(old_count + 10, t4->exec_count());
	state = isix::get_task_state(t1->tid());
	TEST_ASSERT_EQUAL(OSTHR_STATE_SUSPEND, state);
	// Check T3 for self suspend
	t3->selfsuspend();
	isix::wait_ms(1);
	old_count = t3->exec_count();
	isix::wait_ms(2);
	TEST_ASSERT_EQUAL(t3->exec_count(), old_count);
	TEST_ASSERT_EQUAL(OSTHR_STATE_SUSPEND, state);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::task_resume(t3->tid()));
	state = isix::get_task_state(t3->tid());
	TEST_ASSERT(state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING);
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(t1->tid()));
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(t2->tid()));
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(t3->tid()));
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(t4->tid()));
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(nullptr));
	//Now delete tasks
}

TEST(tasks, tasks_suspended)
{
	const auto oprio = isix::task_change_prio(nullptr, isix::get_min_priority());
	TEST_ASSERT(oprio >= 0);
	bool to_change = false;
	const auto test_task_suspended = [&]()
	{
		to_change = true;
		for (;;) {
			isix::wait_ms(10000);
		}
	};
	auto thr = isix::thread_create_and_run(c_stack_size,1,
			isix_task_flag_suspended,test_task_suspended);
	// Check for task create suspended
	TEST_ASSERT(thr);
	TEST_ASSERT_EQUAL(OSTHR_STATE_SUSPEND, isix::get_task_state(thr.tid()));
	TEST_ASSERT_FALSE(to_change);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::task_resume(thr.tid()));
	auto state = isix::get_task_state(thr.tid());
	TEST_ASSERT(state==OSTHR_STATE_RUNNING || state==OSTHR_STATE_SLEEPING);
	TEST_ASSERT(to_change);
	TEST_ASSERT_GREATER_OR_EQUAL(0, isix::task_change_prio(nullptr, oprio));
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(thr.tid()));
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(tasks, CPU_load_api)
{
	int iload = 10;
	const auto cpuload_task = [](int load)
	{
		for (;;) {
			isix::wait_us(load * 1000);
			isix::wait_ms(100 - load);
		}
	};
	isix::wait_ms(5000);
	static constexpr auto epsilon = 50;
	for (iload=10; iload<=99; iload+=10) {
		auto thr = isix::thread_create_and_run(c_stack_size,1,0,cpuload_task, iload);
		TEST_ASSERT(thr);
		isix::wait_ms(4000);
		const auto cpul = isix::cpuload();
		isix::wait_ms(10);
		TEST_ASSERT_INT_WITHIN(epsilon, iload*10, cpul);
		TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(thr.tid()));
	}
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(tasks, cpp11_thread_api_creation)
{
	{
		bool finished = false;
		auto thr1 = isix::thread_create_and_run(2048, c_task_prio, 0,
				[&](volatile bool &a, int b)
				{
					TEST_ASSERT_EQUAL(15, b);
					isix::wait_ms(100);
					a = true;
				},
				std::ref(finished),
				15
				);
		isix::wait_ms(200);
		TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(nullptr));
		TEST_ASSERT(finished);
	}
	{
		isix::memory_stat ms;
		isix::heap_stats(ms);
		const auto ram_beg = ms.free;
		{
			auto thr1 = isix::thread_create_and_run(c_stack_size, c_task_prio,
					0, thr11::thread2_func);
			isix_wait_ms(900);
			TEST_ASSERT(thr11::fin2);
		}
		// Not referenced task must free whole memory in idle task
		isix_wait_ms(100);
		isix::heap_stats(ms);
		const auto ram_end = ms.free;
		TEST_ASSERT_GREATER_OR_EQUAL_size_t(ram_end, ram_beg);
		TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(nullptr));
	}
}

TEST(tasks, wait_and_referenced_api)
{
	//! Create referenced
	isix::memory_stat ms;
	isix::heap_stats(ms);
	auto ram_beg = ms.free;
	auto th1 = isix::task_create(waitref::task_ref, nullptr, c_stack_size, c_task_prio,
			isix_task_flag_ref|isix_task_flag_newlib);
	TEST_ASSERT(th1);
	auto t1 = isix::get_jiffies();
	auto ret = isix::task_wait_for (th1);
	auto t2 = isix::get_jiffies() - t1;
	//! Should return 0
	TEST_ASSERT_EQUAL(ISIX_EOK, ret);
	// Should match in range
	TEST_ASSERT_UINT_WITHIN(5, 505U, t2);
	// Task wait list should be empty
	TEST_ASSERT(thack_task_wait_list_is_empty(th1));
	/** Check memory usage before and after because task is referenced
	 * difference between memory areas should be equal task stack size */
	isix::wait_ms(300);
	isix::heap_stats(ms);
	auto ram_end = ms.free;
	TEST_ASSERT_GREATER_OR_EQUAL_size_t(ram_beg, ram_end+thack_struct_size());
	TEST_ASSERT_EQUAL_size_t(thack_struct_size(), (ram_beg-ram_end));
	TEST_ASSERT_EQUAL(1, thack_getref_cnt(th1));
	// Check the task state
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, isix::get_task_state(th1));
	//Increment reference
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_ref(th1));
	TEST_ASSERT_EQUAL(2, thack_getref_cnt(th1));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_unref(th1));
	TEST_ASSERT_EQUAL(1, thack_getref_cnt(th1));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_unref(th1));
	//! NOTE: Should be 0 but pointer to th is invalid now
	///TEST_ASSERT(thack_getref_cnt(th1) == 0);
	isix::heap_stats(ms);
	ram_end = ms.free;
	TEST_ASSERT_GREATER_OR_EQUAL_size_t(ram_beg, ram_end);
	isix::heap_stats(ms);
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(tasks, wait_reference_notice)
{
	isix::memory_stat ms;
	isix::heap_stats(ms);
	auto ram_beg = ms.free;
	// Create first for normal task_ref task. Five task_ref2 tasks wait when task 1 fin
	// and notice task 1 when ends
	auto th1 = isix::task_create(waitref::task_ref, nullptr, c_stack_size, c_task_prio,
			isix_task_flag_ref|isix_task_flag_newlib);
	auto tn1 = isix::task_create(waitref::task_ref2, th1, c_stack_size, c_task_prio,
			isix_task_flag_ref|isix_task_flag_newlib);
	auto tn2 = isix::task_create(waitref::task_ref2, th1, c_stack_size, c_task_prio,
			isix_task_flag_ref|isix_task_flag_newlib);
	auto tn3 = isix::task_create(waitref::task_ref2, th1, c_stack_size, c_task_prio,
			isix_task_flag_ref|isix_task_flag_newlib);
	auto tn4 = isix::task_create(waitref::task_ref2, th1, c_stack_size, c_task_prio,
			isix_task_flag_ref|isix_task_flag_newlib);
	auto t1 = isix::get_jiffies();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::task_wait_for (tn1));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::task_wait_for (tn2));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::task_wait_for (tn3));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::task_wait_for (tn4));
	auto t2 = isix::get_jiffies() - t1;
	TEST_ASSERT_UINT_WITHIN(25, 575U, t2);
	//Task th1 also should be in exited state
	isix_wait_ms(25);
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, isix::get_task_state(th1));
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, isix::get_task_state(tn1));
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, isix::get_task_state(tn2));
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, isix::get_task_state(tn3));
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, isix::get_task_state(tn4));
	TEST_ASSERT_EQUAL(1,  thack_getref_cnt(th1));
	TEST_ASSERT_EQUAL(1,  thack_getref_cnt(tn1));
	TEST_ASSERT_EQUAL(1,  thack_getref_cnt(tn2));
	TEST_ASSERT_EQUAL(1,  thack_getref_cnt(tn3));
	TEST_ASSERT_EQUAL(1,  thack_getref_cnt(tn4));
	TEST_ASSERT(thack_task_wait_list_is_empty(th1));
	TEST_ASSERT(thack_task_wait_list_is_empty(tn1));
	TEST_ASSERT(thack_task_wait_list_is_empty(tn2));
	TEST_ASSERT(thack_task_wait_list_is_empty(tn3));
	TEST_ASSERT(thack_task_wait_list_is_empty(tn4));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_unref(th1));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_unref(tn1));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_unref(tn2));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_unref(tn3));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_unref(tn4));
	isix::heap_stats(ms);
	auto ram_end = ms.free;
	//Check if size match
	TEST_ASSERT_EQUAL(ram_end, ram_beg);
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(tasks, errno_threadsafe)
{
	constexpr auto errno_thread = [](int& err)
	{
		for (int i=0;i<16;++i)
		{
			errno = err;
		}
		isix_wait_ms(1);
		err = errno;
	};

	int err[4] { 5, 10, 15, 20 };
	static constexpr int except[4] { 5, 10, 15, 20 };
	errno = 50;

	auto th1 = isix::thread_create_and_run(c_stack_size, c_task_prio,
				isix_task_flag_newlib, errno_thread, std::ref(err[0]));
	auto th2 = isix::thread_create_and_run(c_stack_size, c_task_prio,
				isix_task_flag_newlib, errno_thread, std::ref(err[1]));
	auto th3 = isix::thread_create_and_run(c_stack_size, c_task_prio,
				isix_task_flag_newlib, errno_thread, std::ref(err[2]));
	auto th4 = isix::thread_create_and_run(c_stack_size, c_task_prio,
				isix_task_flag_newlib, errno_thread, std::ref(err[3]));
	TEST_ASSERT(th1);
	TEST_ASSERT(th2);
	TEST_ASSERT(th3);
	TEST_ASSERT(th4);
	isix_wait_ms(10);
	for (int i=0; i<4; ++i) {
		TEST_ASSERT_EQUAL(except[i], err[i]);
	}
	TEST_ASSERT_EQUAL(50, errno);
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(tasks, simple_FPU_single_precision_test_without_interrupts)
{
	volatile float val = 1.0;
	const auto thr = [&]()
	{
		for (int i=0;i<100000; ++i) {
			val += 0.5;
		}
	};
	auto th1 = isix::thread_create_and_run(c_stack_size, c_task_prio,
				isix_task_flag_newlib, thr
	);
	TEST_ASSERT(th1);
	TEST_ASSERT_EQUAL(ISIX_EOK, th1.wait_for());
	const auto newval { int(val) };
	TEST_ASSERT_EQUAL(50001, newval);
}
#if (__ARM_FP > 0)

TEST(tasks, simple_FPU_double_precision_test_without_interrupts)
{
	volatile double val = 1.0;
	static constexpr auto n_loops = 100000U;
	const auto thr = [&]()
	{
		for (unsigned i=0;i<n_loops; ++i) {
			val += 0.5;
		}
	};
	auto th1 = isix::thread_create_and_run(c_stack_size, c_task_prio,
				isix_task_flag_newlib, thr
	);
	TEST_ASSERT(th1);
	TEST_ASSERT_EQUAL(ISIX_EOK, th1.wait_for());
	const auto newval { int(val) };
	TEST_ASSERT_EQUAL(50001, newval);
}

TEST(tasks, FPU_single_precision_two_tasks_and_interrupt)
{
	static constexpr auto n_loops = 10000000U;
	using namespace tests::fpu_sp;
	constexpr auto thr = [](int begin_val, bool& ok) -> void
	{
		fill_and_add(begin_val);
		for (unsigned i=0; i<n_loops;++i) {
			if (fill_and_add_check(begin_val)) {
				ok = false;
				break;
			}
		}
		ok = true;
	};
	constexpr auto thr_c = [](float& val) -> void
	{
		for (unsigned i=0;i<n_loops; ++i) {
			val += 0.5;
		}
	};
	volatile bool irq_failed {};
	bool res1 {}; bool res2 {};
	float val = 1.0;
	int irq_nums {};
	const auto irq_fun = [&]()
	{
		using namespace tests::fpu_sp;
		irq_nums++;
		if (irq_failed) {
			return;
		}
		base_regs_fill(0x44);
		if (base_regs_check(0x44)) {
			irq_failed = true;
		}
	};
	auto ec = tests::detail::periodic_timer_setup(irq_fun, 10);
	TEST_ASSERT(ec);
	auto th1 = isix::thread_create_and_run(2048, c_task_prio,
			isix_task_flag_newlib, thr, 4, std::ref(res1));
	auto th2 = isix::thread_create_and_run(2048, c_task_prio,
			isix_task_flag_newlib, thr, 2, std::ref(res2));
	auto th3 = isix::thread_create_and_run(2048, c_task_prio,
			isix_task_flag_newlib, thr_c, std::ref(val));
	TEST_ASSERT(th1);
	TEST_ASSERT(th2);
	TEST_ASSERT(th3);
	TEST_ASSERT_EQUAL(ISIX_EOK, th1.wait_for ());
	TEST_ASSERT_EQUAL(ISIX_EOK, th2.wait_for ());
	TEST_ASSERT_EQUAL(ISIX_EOK, th3.wait_for ());
	TEST_ASSERT(res1);
	TEST_ASSERT(res2);
	TEST_ASSERT_EQUAL(int(n_loops/2+1), int(val));
	tests::detail::periodic_timer_stop();
	TEST_ASSERT_GREATER_THAN(10, irq_nums);
	TEST_ASSERT_FALSE(irq_failed);
	isix::wait_ms(100);
}
#endif /* __ARM_FP > 0 */


TEST_GROUP_RUNNER(tasks)
{
	RUN_TEST_CASE(tasks, basic_api);
	RUN_TEST_CASE(tasks, tasks_suspended);
	RUN_TEST_CASE(tasks, CPU_load_api);
	RUN_TEST_CASE(tasks, cpp11_thread_api_creation);
	RUN_TEST_CASE(tasks, wait_and_referenced_api);
	RUN_TEST_CASE(tasks, wait_reference_notice);
	RUN_TEST_CASE(tasks, errno_threadsafe);
	RUN_TEST_CASE(tasks, simple_FPU_single_precision_test_without_interrupts);
	RUN_TEST_CASE(tasks, simple_FPU_double_precision_test_without_interrupts);
	RUN_TEST_CASE(tasks, FPU_single_precision_two_tasks_and_interrupt);
}