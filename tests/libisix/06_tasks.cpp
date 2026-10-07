#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
#undef _ISIX_KERNEL_CORE_
#include <isix/prv/test_hooks.h>
#include "task_test_helper.h"
#include "utils/fpu_test_and_set.h"
#include "utils/timer_interrupt.hpp"
#include <memory>
#include <cstring>
#include <utils/test_prio.hpp>

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
namespace {
	// State of the kill-while-waiting test, kept here so teardown can clean up after a failure
	ostask_t wf_tasks[2];
	volatile bool wf_stop;
	// State of the cleanup/unref race test
	ossem_t s_hook_sem;
	ostask_t s_victim;
	void cleanup_hook(isix_test_point point, void* arg)
	{
		if (point == isix_tp_task_cleanup && arg == s_victim) { isix_sem_signal(s_hook_sem); }
	}
}

namespace {
	test_utils::task_pool tk_tasks;
	ostask_t tk_target;
	volatile bool tk_flag;
}

TEST_TEAR_DOWN(tasks)
{
	tests::detail::periodic_timer_stop();
	test_utils::restore_test_prio();
	tk_tasks.release();
	_isixp_test_hook = nullptr;
	wf_stop = true;
	for (auto& t : wf_tasks) {
		if (t) { isix_task_kill(t); t = nullptr; }
	}
}

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
	for (auto tc = isix_get_jiffies(); isix_get_jiffies()<tc+isix::ms2tick(5000);) {
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
	for (auto tc = isix_get_jiffies(); isix_get_jiffies()<tc+isix::ms2tick(5000);) {
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
#if defined(QEMU_NO_RCC_PERIPH) && CONFIG_ISIX_HZ > 1000
	// The emulated SysTick does not keep the requested rate above 1 kHz, the load window is noisy
	static constexpr auto epsilon = 80;
#else
	static constexpr auto epsilon = 50;
#endif
	for (iload=10; iload<=99; iload+=10) {
		bool created = false;
		int cpul = 0;
		int stack_free = 0;
		{
			auto thr = isix::thread_create_and_run(c_stack_size,1,0,cpuload_task, iload);
			created = static_cast<bool>(thr);
			isix::wait_ms(2000);
			cpul = isix::cpuload();
			isix::wait_ms(10);
			stack_free = isix::free_stack_space(thr.tid());
		}
		TEST_ASSERT(created);
		TEST_ASSERT_INT_WITHIN(epsilon, iload*10, cpul);
		TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, stack_free);
	}
	TEST_ASSERT_GREATER_OR_EQUAL(c_stack_margin, isix::free_stack_space(nullptr));
}

TEST(tasks, cpu_load_refresh_after_idle)
{
	// Window published after a busy period must be refreshed after a long idle sleep
	const auto end = isix::get_jiffies() + isix::ms2tick(1500);
	while (isix::get_jiffies() < end) {
		isix::wait_us(1000);
	}
	isix::wait_ms(3000);
	const auto cpul = isix::cpuload();
	TEST_ASSERT_LESS_THAN(300, cpul);
}

TEST(tasks, kill_task_waiting_for_exit)
{
	wf_stop = false;
	auto target = isix::thread_create_and_run(c_stack_size,3,0,[]()
	{
		while (!wf_stop) { isix::wait_ms(1); }
	});
	wf_tasks[0] = target.tid();
	TEST_ASSERT(target);
	const auto target_tid = target.tid();
	auto waiter = isix::thread_create_and_run(c_stack_size,3,0,[target_tid]()
	{
		isix_task_wait_for(target_tid);
	});
	wf_tasks[1] = waiter.tid();
	TEST_ASSERT(waiter);
	isix::wait_ms(10);
	wf_tasks[1] = nullptr;
	waiter.kill();
	wf_stop = true;
	isix::wait_ms(10);
	wf_tasks[0] = nullptr;
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, target.get_state());
}

TEST(tasks, suspend_resume_task_waiting_for_exit)
{
	static volatile int wait_res;
	wait_res = -9999;
	wf_stop = false;
	auto target = isix::thread_create_and_run(c_stack_size,3,0,[]()
	{
		while (!wf_stop) { isix::wait_ms(1); }
	});
	wf_tasks[0] = target.tid();
	TEST_ASSERT(target);
	const auto target_tid = target.tid();
	auto waiter = isix::thread_create_and_run(c_stack_size,3,0,[target_tid]()
	{
		wait_res = isix_task_wait_for(target_tid);
	});
	wf_tasks[1] = waiter.tid();
	TEST_ASSERT(waiter);
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(OSTHR_STATE_WTEXIT, waiter.get_state());
	waiter.suspend();
	TEST_ASSERT_EQUAL(OSTHR_STATE_SUSPEND, waiter.get_state());
	TEST_ASSERT_EQUAL(ISIX_EOK, waiter.resume());
	isix::wait_ms(10);
	const auto state = waiter.get_state();
	const auto res = wait_res;
	TEST_ASSERT_EQUAL(OSTHR_STATE_WTEXIT, state);
	TEST_ASSERT_EQUAL(-9999, res);
	wf_stop = true;
	isix::wait_ms(10);
	wf_tasks[0] = wf_tasks[1] = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EOK, wait_res);
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, waiter.get_state());
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

TEST(tasks, cpp11_thread_destroy_before_first_run)
{
	for (int i=0; i<200; ++i) {
		{
			auto thr = isix::thread_create_and_run(c_stack_size, 0, 0, []() {
				isix::wait_ms(1);
			});
			TEST_ASSERT(thr);
		}
		if ((i%8) == 0) {
			//Let the idle task reclaim the dead tasks
			isix::wait_ms(2);
		} else if ((i%3) == 0) {
			isix_yield();
		}
	}
	isix::wait_ms(20);
}

TEST(tasks, wait_and_referenced_api)
{
	// Let idle finish the cleanup of the tasks killed by the previous tests
	isix::wait_ms(100);
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
	TEST_ASSERT_UINT_WITHIN(isix::ms2tick(5), isix::ms2tick(505), t2);
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
	TEST_ASSERT_UINT_WITHIN(isix::ms2tick(25), isix::ms2tick(575), t2);
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


TEST(tasks, cleanup_unref_race)
{
	// The allocator keeps its free list pointers in the first bytes of a freed block
	constexpr auto tcb_skip = sizeof(void*) * 2;
	constexpr auto tcb_size = sizeof(struct isix_task);
	static unsigned char snapshot[tcb_size];
	// Let idle finish the cleanup of the tasks killed by the previous tests
	isix_wait_ms(100);
	isix::memory_stat ms_before;
	isix::heap_stats(ms_before);
	s_hook_sem = isix_sem_create_limited(nullptr, 0, 1);
	TEST_ASSERT(s_hook_sem);
	s_victim = isix_task_create([](void*) {}, nullptr, c_stack_size, c_task_prio,
			isix_task_flag_ref);
	TEST_ASSERT(s_victim);
	_isixp_test_hook = cleanup_hook;
	// Idle is now inside the cleanup window, the task is EXITED with one reference
	const auto wret = isix_sem_wait(s_hook_sem, 1000);
	_isixp_test_hook = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EOK, wret);
	const auto tcb = reinterpret_cast<const unsigned char*>(s_victim);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_unref(s_victim));
	// The TCB is freed now, idle must not write to it any more
	std::memcpy(snapshot, tcb, tcb_size);
	isix_wait_ms(20);
	const auto intact = std::memcmp(snapshot + tcb_skip, tcb + tcb_skip, tcb_size - tcb_skip) == 0;
	isix_sem_destroy(s_hook_sem);
	s_hook_sem = nullptr;
	s_victim = nullptr;
	TEST_ASSERT_TRUE(intact);
	isix::memory_stat ms_after;
	isix::heap_stats(ms_after);
	TEST_ASSERT_EQUAL_UINT(ms_before.free, ms_after.free);
}

TEST(tasks, wait_for_returns_eok_after_target_timeout)
{
	auto sem = isix_sem_create_limited(nullptr, 0, 1);
	TEST_ASSERT(sem);
	{
		auto target = isix::thread_create_and_run(c_stack_size,c_task_prio,0,[sem]()
		{
			isix_sem_wait(sem, isix::ms2tick(20));
		});
		TEST_ASSERT(target);
		const auto res = target.wait_for();
		TEST_ASSERT_EQUAL(ISIX_EOK, res);
	}
	isix_sem_destroy(sem);
	auto ev = isix_event_create();
	TEST_ASSERT(ev);
	{
		auto target = isix::thread_create_and_run(c_stack_size,c_task_prio,0,[ev]()
		{
			isix_event_wait(ev, 0x5, true, true, 1000);
		});
		TEST_ASSERT(target);
		isix::wait_ms(5);
		isix_event_set(ev, 0x5);
		const auto res = target.wait_for();
		TEST_ASSERT_EQUAL(ISIX_EOK, res);
	}
	isix_event_destroy(ev);
}

TEST(tasks, wait_for_target_killed_by_other)
{
	static volatile int wait_res;
	wait_res = -9999;
	wf_stop = false;
	auto target = isix::thread_create_and_run(c_stack_size,c_task_prio,0,[]()
	{
		while (!wf_stop) { isix::wait_ms(1); }
	});
	wf_tasks[0] = target.tid();
	TEST_ASSERT(target);
	const auto target_tid = target.tid();
	auto waiter = isix::thread_create_and_run(c_stack_size,c_task_prio,0,[target_tid]()
	{
		wait_res = isix_task_wait_for(target_tid);
	});
	wf_tasks[1] = waiter.tid();
	TEST_ASSERT(waiter);
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(OSTHR_STATE_WTEXIT, waiter.get_state());
	target.kill();
	isix::wait_ms(10);
	const auto res = wait_res;
	const auto wstate = waiter.get_state();
	wf_tasks[0] = wf_tasks[1] = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EDESTROY, res);
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, wstate);
}

TEST(tasks, wait_for_target_killed_itself)
{
	static volatile int wait_res;
	wait_res = -9999;
	wf_stop = false;
	auto target = isix::thread_create_and_run(c_stack_size,c_task_prio,0,[]()
	{
		isix::wait_ms(5);
		isix_task_kill(nullptr);
	});
	wf_tasks[0] = target.tid();
	TEST_ASSERT(target);
	const auto target_tid = target.tid();
	auto waiter = isix::thread_create_and_run(c_stack_size,c_task_prio,0,[target_tid]()
	{
		wait_res = isix_task_wait_for(target_tid);
	});
	wf_tasks[1] = waiter.tid();
	TEST_ASSERT(waiter);
	isix::wait_ms(20);
	const auto res = wait_res;
	const auto wstate = waiter.get_state();
	wf_tasks[0] = wf_tasks[1] = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EOK, res);
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, wstate);
}


TEST(tasks, suspend_zombie_task_is_noop)
{
	// A higher priority task finishes at once and stays a zombie because idle cannot run
	test_utils::lower_test_prio(5);
	static constexpr auto quick = [](void*) {};
	const auto t = tk_tasks.spawn(quick, nullptr, 3);
	TEST_ASSERT_NOT_NULL(t);
	TEST_ASSERT_EQUAL(OSTHR_STATE_ZOMBIE, isix_get_task_state(t));
	isix_task_suspend(t);
	TEST_ASSERT_EQUAL(OSTHR_STATE_ZOMBIE, isix_get_task_state(t));
	TEST_ASSERT_EQUAL(ISIX_ESTATE, isix_task_resume(t));
	test_utils::restore_test_prio();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(OSTHR_STATE_EXITED, isix_get_task_state(t));
}

TEST(tasks, kill_wakes_exit_waiter_immediately)
{
	test_utils::lower_test_prio(10);
	static constexpr auto target = [](void*) { isix_wait_ms(5000); };
	static constexpr auto waiter = [](void*) {
		isix_task_wait_for(tk_target);
		tk_flag = true;
	};
	tk_flag = false;
	tk_target = tk_tasks.spawn(target, nullptr, 12);
	TEST_ASSERT_NOT_NULL(tk_target);
	const auto w = tk_tasks.spawn(waiter, nullptr, 5);
	TEST_ASSERT_NOT_NULL(w);
	isix::wait_ms(10);
	TEST_ASSERT_FALSE(tk_flag);
	isix_task_kill(tk_target);
	TEST_ASSERT_TRUE(tk_flag);
}

TEST(tasks, cpp11_thread_move_keeps_single_owner)
{
	isix::memory_stat before;
	isix::heap_stats(before);
	{
		volatile int ran = 0;
		isix::thread src = isix::thread_create([&ran]() { ran = 1; });
		// Moving a thread which was not started yet is allowed
		isix::thread dst(std::move(src));
		TEST_ASSERT_FALSE(src.tid() != nullptr);
		dst.start_thread(1024, 3);
		TEST_ASSERT_NOT_NULL(dst.tid());
		isix::wait_ms(20);
		TEST_ASSERT_EQUAL(1, ran);
		// Started thread returned by value keeps running with the single owner
		isix::thread run = isix::thread_create_and_run(1024, 3, 0, [&ran]() { ran = 2; });
		isix::wait_ms(20);
		TEST_ASSERT_EQUAL(2, ran);
	}
	isix::wait_ms(20);
	isix::memory_stat after;
	isix::heap_stats(after);
	TEST_ASSERT_EQUAL_size_t(before.free, after.free);
}

TEST_GROUP_RUNNER(tasks)
{
	RUN_TEST_CASE(tasks, cpp11_thread_move_keeps_single_owner);
	RUN_TEST_CASE(tasks, suspend_zombie_task_is_noop);
	RUN_TEST_CASE(tasks, kill_wakes_exit_waiter_immediately);
	RUN_TEST_CASE(tasks, basic_api);
	RUN_TEST_CASE(tasks, tasks_suspended);
	RUN_TEST_CASE(tasks, CPU_load_api);
	RUN_TEST_CASE(tasks, cpu_load_refresh_after_idle);
	RUN_TEST_CASE(tasks, kill_task_waiting_for_exit);
	RUN_TEST_CASE(tasks, suspend_resume_task_waiting_for_exit);
	RUN_TEST_CASE(tasks, cpp11_thread_api_creation);
	RUN_TEST_CASE(tasks, cpp11_thread_destroy_before_first_run);
	RUN_TEST_CASE(tasks, wait_and_referenced_api);
	RUN_TEST_CASE(tasks, wait_reference_notice);
	RUN_TEST_CASE(tasks, errno_threadsafe);
	RUN_TEST_CASE(tasks, simple_FPU_single_precision_test_without_interrupts);
	RUN_TEST_CASE(tasks, simple_FPU_double_precision_test_without_interrupts);
	RUN_TEST_CASE(tasks, FPU_single_precision_two_tasks_and_interrupt);
	RUN_TEST_CASE(tasks, cleanup_unref_race);
	RUN_TEST_CASE(tasks, wait_for_returns_eok_after_target_timeout);
	RUN_TEST_CASE(tasks, wait_for_target_killed_by_other);
	RUN_TEST_CASE(tasks, wait_for_target_killed_itself);
}
