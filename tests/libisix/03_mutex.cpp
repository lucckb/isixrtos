#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <isix/prv/list.h>
#include <isix/prv/mutex.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
#undef _ISIX_KERNEL_CORE_
#include <string>
#include <utils/test_prio.hpp>

#pragma GCC diagnostic ignored "-Wstrict-aliasing"

//Temporary private data access mutex stuff
// Structure of isix mutex

struct mtx_hacker {
	osmtx_t mtx;
};

namespace {
	std::string test_buf;
	isix::mutex mtx1;
	isix::mutex mtx2;
	isix::condvar mcv;
	constexpr auto STK_SIZ = 2048;

	using test_utils::cpu_busy;
	// Allowance for the tick quantization of the chained waits and busy loops
	constexpr auto quant_slack = ISIX_HZ < 1000U ? 3U : 1U;
}


TEST_GROUP(mutex);
TEST_SETUP(mutex) {}
namespace {
	// State of the abandoned mutex test, kept here so teardown can clean up after a failure
	int abandoned_fin[5];
	ostask_t abandoned_tasks[5];
	// State of the kill-waiting-task tests
	int kill_res[2];
	ostask_t kill_tasks[3];
	bool kill_main_locked;
}

namespace {
	test_utils::task_pool mx_tasks;
	osmtx_t mx_a, mx_b, mx_c;
	oscondvar_t mx_cv;
	volatile int mx_step;
	volatile int mx_res;
	volatile bool mx_owner_after;
	volatile bool mx_flag;
	std::string mx_order;

	unsigned count_owned(ostask_t t)
	{
		unsigned n = 0;
		const list_t* head = &t->owned_mutexes.head;
		for (const list_t* p = head->next; p != head && n < 10; p = p->next) {
			++n;
		}
		return n;
	}
	bool owns_first(ostask_t t, osmtx_t m)
	{
		return t->owned_mutexes.head.next == &m->inode;
	}
	void mx_destroy_all()
	{
		for (auto* m : { &mx_a, &mx_b, &mx_c }) {
			if (*m) { isix_mutex_destroy(*m); *m = nullptr; }
		}
		if (mx_cv) { isix_condvar_destroy(mx_cv); mx_cv = nullptr; }
	}
}

TEST_TEAR_DOWN(mutex)
{
	test_utils::restore_test_prio();
	mx_tasks.release();
	mx_destroy_all();
	mx_order.clear();
	for (auto& t : abandoned_tasks) {
		if (t) { isix_task_kill(t); t = nullptr; }
	}
	for (auto& t : kill_tasks) {
		if (t) { isix_task_kill(t); t = nullptr; }
	}
	if (kill_main_locked) {
		mtx1.unlock();
		kill_main_locked = false;
	}
}

TEST(mutex, delivery_order)
{
	constexpr auto thr1 = [](char ch)
	{
		if (mtx1.lock()) { test_buf.push_back('Z'); return; }
		test_buf.push_back(ch);
		if (mtx1.unlock()) { test_buf.push_back('Z'); return; }
	};
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.lock());
	auto tsk1 = isix::thread_create_and_run(STK_SIZ, 5, 0, thr1, 'A');
	auto tsk2 = isix::thread_create_and_run(STK_SIZ, 4, 0, thr1, 'B');
	auto tsk3 = isix::thread_create_and_run(STK_SIZ, 3, 0, thr1, 'C');
	auto tsk4 = isix::thread_create_and_run(STK_SIZ, 2, 0, thr1, 'D');
	auto tsk5 = isix::thread_create_and_run(STK_SIZ, 1, 0, thr1, 'E');
	TEST_ASSERT(tsk1);
	TEST_ASSERT(tsk2);
	TEST_ASSERT(tsk3);
	TEST_ASSERT(tsk4);
	TEST_ASSERT(tsk5);
	isix::wait_ms(100);
	TEST_ASSERT(test_buf.empty());
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.unlock());
	isix::wait_ms(200);
	TEST_ASSERT_EQUAL_STRING("EDCBA", test_buf.c_str());
	test_buf.clear();
}

TEST(mutex, priority_inheritance_basic_conditions)
{
	ostick_t fin[3] {};
	const auto thr2l = [&]()
	{
		if (mtx1.lock()) { test_buf.push_back('Z'); return; }
		cpu_busy(40);
		if (mtx1.unlock()) { test_buf.push_back('Z'); return; }
		cpu_busy(10);
		test_buf.push_back('C');
		fin[2] = isix::get_jiffies();
	};
	const auto thr2m = [&]() {
		isix::wait_ms(20);
		cpu_busy(40);
		test_buf.push_back('B');
		fin[1]=isix::get_jiffies();
	};
	const auto thr2h = [&]() {
		isix::wait_ms(40);
		if (mtx1.lock()) { test_buf.push_back('Z'); return; }
		cpu_busy(10);
		if (mtx1.unlock()) { test_buf.push_back('Z'); return; }
		test_buf.push_back('A');
		fin[0]=isix::get_jiffies();
	};
	TEST_ASSERT(test_buf.empty());
	const auto t1 = isix::get_jiffies();
	auto tsk1 = isix::thread_create_and_run(STK_SIZ,1,0,thr2h);
	auto tsk2 = isix::thread_create_and_run(STK_SIZ,2,0,thr2m);
	auto tsk3 = isix::thread_create_and_run(STK_SIZ,3,0,thr2l);
	TEST_ASSERT(tsk1);
	TEST_ASSERT(tsk2);
	TEST_ASSERT(tsk3);
	isix::wait_ms(350);
	TEST_ASSERT_EQUAL_STRING("ABC", test_buf.c_str());
	ostick_t max {};
	for(auto v: fin) {
		max = std::max(max, v - t1);
	}
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(isix::ms2tick(70U), max);
	TEST_ASSERT_LESS_THAN_UINT(isix::ms2tick(105U) + quant_slack, max);
	test_buf.clear();
	isix::wait_ms(5);
}

/** Priority inheritance complex case
Five threads are involved in the complex priority inversion
* scenario, the priority inheritance algorithm is tested for depths
* greater than one. The test expects the threads to perform their
* operations in increasing priority order by rearranging their
* priorities in order to avoid the priority inversion trap.
*/
TEST(mutex, priority_inheritance_complex)
{
	ostick_t fin[5] {};
	const auto thr3ll = [&]()
	{
		if (mtx1.lock()) { test_buf.push_back('Z'); return; }
		cpu_busy(30);
		if (mtx1.unlock()) { test_buf.push_back('Z'); return; }
		test_buf.push_back('E');
		fin[0]=isix::get_jiffies();
	};
	const auto thr3l = [&]()
	{
		isix::wait_ms(10);
		if (mtx2.lock()) { test_buf.push_back('Z'); return; }
		cpu_busy(20);
		if (mtx1.lock()) { test_buf.push_back('Z'); return; }
		cpu_busy(10);
		if (mtx1.unlock()) { test_buf.push_back('Z'); return; }
		cpu_busy(10);
		if (mtx2.unlock()) { test_buf.push_back('Z'); return; }
		test_buf.push_back('D');
		fin[1]=isix::get_jiffies();
	};
	const auto thr3m = [&]()
	{
		isix::wait_ms(20);
		if (mtx2.lock()) { test_buf.push_back('Z'); return; }
		cpu_busy(10);
		if (mtx2.unlock()) { test_buf.push_back('Z'); return; }
		test_buf.push_back('C');
		fin[2]=isix::get_jiffies();
	};
	const auto thr3h = [&]()
	{
		isix::wait_ms(40);
		cpu_busy(20);
		test_buf.push_back('B');
		fin[3]=isix::get_jiffies();
	};
	const auto thr3hh = [&]()
	{
		isix::wait_ms(50);
		if (mtx2.lock()) { test_buf.push_back('Z'); return; }
		cpu_busy(10);
		if (mtx2.unlock()) { test_buf.push_back('Z'); return; }
		test_buf.push_back('A');
		fin[4]=isix::get_jiffies();
	};
	const auto t1 = isix::get_jiffies();
	auto tsk1 = isix::thread_create_and_run(STK_SIZ,5,0,thr3ll);
	auto tsk2 = isix::thread_create_and_run(STK_SIZ,4,0,thr3l);
	auto tsk3 = isix::thread_create_and_run(STK_SIZ,3,0,thr3m);
	auto tsk4 = isix::thread_create_and_run(STK_SIZ,2,0,thr3h);
	auto tsk5 = isix::thread_create_and_run(STK_SIZ,1,0,thr3hh);
	TEST_ASSERT(tsk1);
	TEST_ASSERT(tsk2);
	TEST_ASSERT(tsk3);
	TEST_ASSERT(tsk4);
	TEST_ASSERT(tsk5);

	isix::wait_ms(350);
	TEST_ASSERT_EQUAL_STRING("ABCDE", test_buf.c_str());
	ostick_t max {};
	for(auto v: fin) {
		max = std::max(max, v - t1);
	}
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(isix::ms2tick(65U), max);
	TEST_ASSERT_LESS_THAN_UINT(isix::ms2tick(110U) + quant_slack, max);
	test_buf.clear();
}

/* Two threads are spawned that try to lock the mutexes already locked
* by the tester thread with precise timing. The test expects that the
* priority changes caused by the priority inheritance algorithm happen
* at the right moment and with the right values.<br> Thread A performs
* wait(50), lock(m1), unlock(m1), exit. Thread B performs wait(150),
* lock(m2), unlock(m2), exit.
*/
TEST(mutex, priority_inheritance_mutex_priority_values)
{
	ostick_t fin[2] {};
	const auto thr4a = [&]()
	{
		isix::wait_ms(50);
		if (mtx1.lock()) {
			return;
		}
		if (mtx1.unlock()) {
			return;
		}
		fin[0]=isix::get_jiffies();
	};
	const auto thr4b = [&]()
	{
		isix::wait_ms(150);
		//isix::enter_critical();
		if (mtx2.lock()) {
			return;
		}
		if (mtx2.unlock()) {
			return;
		}
		isix::yield();
		//isix::exit_critical();
		fin[1]=isix::get_jiffies();
	};
	// Change current priority to minimum
	const auto old_prio = isix::task_change_prio(nullptr, isix::get_min_priority());
	TEST_ASSERT_NOT_EQUAL(isix::get_min_priority(), old_prio);
	const auto p = isix::get_min_priority();
	const auto pa =  p - 1;
	const auto pb =  p - 2;
	auto tsk1 = isix::thread_create_and_run(STK_SIZ,pa,0,thr4a);
	auto tsk2 = isix::thread_create_and_run(STK_SIZ,pb,0,thr4b);
	TEST_ASSERT(tsk1);
	TEST_ASSERT(tsk2);
	/*   Locking the mutex M1 before thread A has a chance to lock
			it. The priority must not change because A has not yet reached
			mtx1 so the mutex is not locked.*/
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.lock());
	//Real prirority should be p
	TEST_ASSERT_EQUAL(p, isix::get_task_inherited_priority());
	//Thread A should reach the mtx1 after 100ms
	isix::wait_ms(100);
	TEST_ASSERT_EQUAL(pa, isix::get_task_inherited_priority());
	/*   Locking the mutex M2 before thread B has a chance to lock
			it. The priority must not change because B has not yet reached
			MTX2 */
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx2.lock());
	TEST_ASSERT_EQUAL(pa, isix::get_task_inherited_priority());
	/* Waiting 100mS, this makes thread B reach mtx2 and
		get the mutex. This must boost the priority of the current thread
		at the same level of thread B */
	isix::wait_ms(100);
	TEST_ASSERT_EQUAL(pb, isix::get_task_inherited_priority());
	/*  Unlocking M2, the priority should fall back to P(A).*/
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx2.unlock());
	TEST_ASSERT_EQUAL(pa, isix::get_task_inherited_priority());
	/*  Unlocking M1, the priority should fall back to P(0).*/
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.unlock());
	TEST_ASSERT_EQUAL(p, isix::get_task_inherited_priority());
	//Restore org prio
	TEST_ASSERT_EQUAL(isix::get_min_priority(), isix::task_change_prio(nullptr, old_prio));
	//Compare priorities
	TEST_ASSERT_GREATER_OR_EQUAL(50U, fin[0]);
	TEST_ASSERT_GREATER_OR_EQUAL(150U, fin[1]);
}

/* The behavior of multiple mutex locks from the same thread is tested
* Getting current thread priority for later checks.
* Locking the mutex first time, it must be possible because it is not owned.
* Locking the mutex second time, it must be possible because it is recursive.
* Unlocking the mutex then it must be still owned because recursivity.
* Unlocking the mutex then it must not be owned anymore and the queue must be empty.
* Testing that priority has not changed after operations.
* Testing consecutive try_lock calls and a final unlock_all()
* Testing consecutive lock/unlock calls and a  final unlock_all().
* Testing that priority has not changed after operations.
*/
TEST(mutex, multiple_mutex_lock_from_same_thread)
{
	const auto prio = isix::get_task_inherited_priority();
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.try_lock());
	//! Locking recursive mutex should be possible
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.try_lock());
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.unlock());
	//After single unlock it must be still owned
	TEST_ASSERT_NOT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.unlock());
	//After second unlock it should be owned
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	// Testing that priority is not changed
	TEST_ASSERT_EQUAL(prio, isix::get_task_inherited_priority());
	// Test consecutive lock unlock and unlock all
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.try_lock());
	//isix_enter_critical();
	int ret = mtx1.try_lock();
	//isix_exit_critical();
	TEST_ASSERT_EQUAL(ISIX_EOK, ret);
	//Check recursion counter
	TEST_ASSERT_EQUAL(2, ((mtx_hacker*)&mtx1)->mtx->count);
	//isix_enter_critical();
	isix::mutex_unlock_all();
	//isix_exit_critical();
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(0, ((mtx_hacker*)&mtx1)->mtx->count);
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
	// Final priority testing
	TEST_ASSERT_EQUAL(prio, isix::get_task_inherited_priority());
}

/** Main hiph priority tasks lock mutex. Five tasks are
 * created when mutex is released
 *  other created tasks should get mutex in the order */
TEST(mutex, high_priority_mutex_order)
{
	const auto thread = [](char ch)
	{
		if (mtx1.lock()) { test_buf.push_back('Z'); return; }
		test_buf.push_back(ch);
		if (mtx1.unlock()) { test_buf.push_back('Z'); return; }
	};
	test_buf.clear();
	auto thr1 = isix::thread_create_and_run(STK_SIZ,5,0,thread,'E');
	TEST_ASSERT(thr1);
	auto thr2 = isix::thread_create_and_run(STK_SIZ,4,0,thread,'D');
	TEST_ASSERT(thr2);
	auto thr3 = isix::thread_create_and_run(STK_SIZ,3,0,thread,'C');
	TEST_ASSERT(thr3);
	auto thr4 = isix::thread_create_and_run(STK_SIZ,2,0,thread,'B');
	TEST_ASSERT(thr4);
	auto thr5 = isix::thread_create_and_run(STK_SIZ,1,0,thread,'A');
	TEST_ASSERT(thr5);
	isix::wait_ms(500);
	TEST_ASSERT_EQUAL_STRING("ABCDE", test_buf.c_str());
}

/* Five tasks take a mutex and wait for
	* task abandon when the owner task is destroyed */
TEST(mutex, abandoned_mutex_when_task_is_destroyed)
{
	auto& fin = abandoned_fin;
	for (auto& f : fin) { f = -9999; }
	const auto thread = [](int& res)
	{
		res = mtx1.lock();
		for(;;) isix_wait_ms(100);
	};
	auto thr1 = isix::thread_create_and_run(STK_SIZ,4,0,thread,std::ref(fin[0]));
	abandoned_tasks[0] = thr1.tid();
	TEST_ASSERT(thr1);
	isix::wait_ms(10);
	auto thr2 = isix::thread_create_and_run(STK_SIZ,5,0,thread,std::ref(fin[1]));
	auto thr3 = isix::thread_create_and_run(STK_SIZ,5,0,thread,std::ref(fin[2]));
	auto thr4 = isix::thread_create_and_run(STK_SIZ,5,0,thread,std::ref(fin[3]));
	auto thr5 = isix::thread_create_and_run(STK_SIZ,5,0,thread,std::ref(fin[4]));
	abandoned_tasks[1] = thr2.tid();
	abandoned_tasks[2] = thr3.tid();
	abandoned_tasks[3] = thr4.tid();
	abandoned_tasks[4] = thr5.tid();
	isix::wait_ms(10);
	TEST_ASSERT(thr2);
	TEST_ASSERT(thr3);
	TEST_ASSERT(thr4);
	TEST_ASSERT(thr5);
	abandoned_tasks[0] = nullptr;
	thr1.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[0]);
	abandoned_tasks[1] = nullptr;
	thr2.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[1]);
	abandoned_tasks[2] = nullptr;
	thr3.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[2]);
	abandoned_tasks[3] = nullptr;
	thr4.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[3]);
	abandoned_tasks[4] = nullptr;
	thr5.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[4]);
	//Final test for the mutex state
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(0, ((mtx_hacker*)&mtx1)->mtx->count);
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
	isix::wait_ms(20);
}

/* Kill a task which waits for the mutex, the mutex must
 * be passed to the next waiting task and the wait list must stay valid */
TEST(mutex, kill_task_waiting_on_mutex)
{
	const auto thread = [](int& res)
	{
		res = mtx1.lock();
		if (res == ISIX_EOK) { mtx1.unlock(); }
		for(;;) isix_wait_ms(100);
	};
	kill_res[0] = kill_res[1] = -9999;
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.lock());
	kill_main_locked = true;
	auto thr_a = isix::thread_create_and_run(STK_SIZ,3,0,thread,std::ref(kill_res[0]));
	auto thr_b = isix::thread_create_and_run(STK_SIZ,3,0,thread,std::ref(kill_res[1]));
	kill_tasks[0] = thr_a.tid();
	kill_tasks[1] = thr_b.tid();
	TEST_ASSERT(thr_a);
	TEST_ASSERT(thr_b);
	isix::wait_ms(10);
	kill_tasks[0] = nullptr;
	thr_a.kill();
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.unlock());
	kill_main_locked = false;
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(-9999, kill_res[0]);
	TEST_ASSERT_EQUAL(ISIX_EOK, kill_res[1]);
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.lock());
	kill_main_locked = true;
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.unlock());
	kill_main_locked = false;
	kill_tasks[1] = nullptr;
	thr_b.kill();
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
}

/* Suspend and resume of a task waiting for the mutex must not give it the mutex */
TEST(mutex, suspend_resume_waiter_keeps_waiting)
{
	kill_res[0] = -9999;
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.lock());
	kill_main_locked = true;
	auto thr = isix::thread_create_and_run(STK_SIZ,3,0,[](int& res)
	{
		res = mtx1.lock();
		if (res == ISIX_EOK) { mtx1.unlock(); }
		for(;;) isix_wait_ms(100);
	},std::ref(kill_res[0]));
	kill_tasks[0] = thr.tid();
	TEST_ASSERT(thr);
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(OSTHR_STATE_WTMTX, thr.get_state());
	thr.suspend();
	TEST_ASSERT_EQUAL(OSTHR_STATE_SUSPEND, thr.get_state());
	TEST_ASSERT_EQUAL(ISIX_EOK, thr.resume());
	isix::wait_ms(10);
	const auto state = thr.get_state();
	const auto res = kill_res[0];
	TEST_ASSERT_EQUAL(OSTHR_STATE_WTMTX, state);
	TEST_ASSERT_EQUAL(-9999, res);
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.unlock());
	kill_main_locked = false;
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(ISIX_EOK, kill_res[0]);
	kill_tasks[0] = nullptr;
	thr.kill();
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
}

/* Kill of the waiting task must drop the priority inherited by the mutex owner */
TEST(mutex, kill_waiter_drops_inherited_prio)
{
	static constexpr auto owner_prio = 5;
	static constexpr auto waiter_prio = 2;
	const auto owner = []()
	{
		mtx1.lock();
		for(;;) isix_wait_ms(100);
	};
	const auto waiter = []()
	{
		mtx1.lock();
		for(;;) isix_wait_ms(100);
	};
	auto thr_o = isix::thread_create_and_run(STK_SIZ,owner_prio,0,owner);
	kill_tasks[0] = thr_o.tid();
	TEST_ASSERT(thr_o);
	isix::wait_ms(10);
	auto thr_w = isix::thread_create_and_run(STK_SIZ,waiter_prio,0,waiter);
	kill_tasks[1] = thr_w.tid();
	TEST_ASSERT(thr_w);
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(waiter_prio, thr_o.get_inherited_prio());
	kill_tasks[1] = nullptr;
	thr_w.kill();
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(owner_prio, thr_o.get_inherited_prio());
	kill_tasks[0] = nullptr;
	thr_o.kill();
	isix::wait_ms(10);
}

/* Mutex create and destroy test
* When tasks wait for mutexes which is
* destroyed it should be awaked with destroyed state*/
TEST(mutex, awake_tasks_when_mutex_is_destroyed)
{
	osmtx_t mloc = isix::mutex_create();
	TEST_ASSERT_NOT_NULL(mloc);
	const auto thr = [&](int& ret) -> void
	{
		ret = isix::mutex_lock(mloc);
		for(;;) {
			isix_wait_ms(10);
		}
	};
	int retp[4] { -9999, -9999, -9999, -9999 };
	auto thr1 = isix::thread_create_and_run(STK_SIZ,4,0,thr,std::ref(retp[0]));
	isix::wait_ms(2);
	auto thr2 = isix::thread_create_and_run(STK_SIZ,4,0,thr,std::ref(retp[1]));
	isix::wait_ms(2);
	auto thr3 = isix::thread_create_and_run(STK_SIZ,4,0,thr,std::ref(retp[2]));
	isix::wait_ms(2);
	auto thr4 = isix::thread_create_and_run(STK_SIZ,4,0,thr,std::ref(retp[3]));
	TEST_ASSERT(thr1);
	TEST_ASSERT(thr2);
	TEST_ASSERT(thr3);
	TEST_ASSERT(thr4);
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::mutex_destroy(mloc));
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(ISIX_EOK, retp[0]);	// First obtained
	TEST_ASSERT_EQUAL(ISIX_EDESTROY, retp[1]);	// Others waiting and destroyed
	TEST_ASSERT_EQUAL(ISIX_EDESTROY, retp[2]);
	TEST_ASSERT_EQUAL(ISIX_EDESTROY, retp[3]);
	isix::wait_ms(10);
}

// Condition variable order test
TEST(mutex, condition_variable_order)
{
	constexpr auto thr = [](char ch) {
		if ((mtx1.lock())) {
			return;
		}
		if (mcv.wait()) {
			return;
		}
		test_buf.push_back(ch);
		if (mtx1.unlock()) {
			return;
		}
		isix::wait_ms(100);
	};
	test_buf.clear();
	auto t1 = isix::thread_create_and_run(STK_SIZ, 5, 0, thr, 'E');
	auto t2 = isix::thread_create_and_run(STK_SIZ, 4, 0, thr, 'D');
	auto t3 = isix::thread_create_and_run(STK_SIZ, 3, 0, thr, 'C');
	auto t4 = isix::thread_create_and_run(STK_SIZ, 2, 0, thr, 'B');
	auto t5 = isix::thread_create_and_run(STK_SIZ, 1, 0, thr, 'A');
	TEST_ASSERT(t1);
	TEST_ASSERT(t2);
	TEST_ASSERT(t3);
	TEST_ASSERT(t4);
	TEST_ASSERT(t5);
	isix::wait_ms(200);
	TEST_ASSERT(test_buf.empty());
	TEST_ASSERT_EQUAL(ISIX_EOK, mcv.broadcast());
	isix::wait_ms(500);
	TEST_ASSERT_EQUAL_STRING("ABCDE", test_buf.c_str());
	// Owned mutexes should be in locked state
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(0, ((mtx_hacker*)&mtx1)->mtx->count);
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
}

TEST(mutex, condvar_mutex_signaling)
{
	constexpr auto thr = [](char ch) {
		if ((mtx1.lock())) {
			return;
		}
		if (mcv.wait()) {
			return;
		}
		test_buf.push_back(ch);
		if (mtx1.unlock()) {
			return;
		}
		isix::wait_ms(100);
	};
	test_buf.clear();
	auto t1 = isix::thread_create_and_run(STK_SIZ, 5, 0, thr, 'E');
	auto t2 = isix::thread_create_and_run(STK_SIZ, 4, 0, thr, 'D');
	auto t3 = isix::thread_create_and_run(STK_SIZ, 3, 0, thr, 'C');
	auto t4 = isix::thread_create_and_run(STK_SIZ, 2, 0, thr, 'B');
	auto t5 = isix::thread_create_and_run(STK_SIZ, 1, 0, thr, 'A');
	TEST_ASSERT(t1);
	TEST_ASSERT(t2);
	TEST_ASSERT(t3);
	TEST_ASSERT(t4);
	TEST_ASSERT(t5);
	isix::wait_ms(200);
	TEST_ASSERT(test_buf.empty());
	TEST_ASSERT_EQUAL(ISIX_EOK, mcv.signal());
	TEST_ASSERT_EQUAL(ISIX_EOK, mcv.signal());
	TEST_ASSERT_EQUAL(ISIX_EOK, mcv.signal());
	TEST_ASSERT_EQUAL(ISIX_EOK, mcv.signal());
	TEST_ASSERT_EQUAL(ISIX_EOK, mcv.signal());
	isix::wait_ms(500);
	TEST_ASSERT_EQUAL_STRING("ABCDE", test_buf.c_str());
}

TEST(mutex, condtion_wait_priority_boost)
{
	constexpr auto thr = [](char ch) {
		if ((mtx1.lock())) {
			return;
		}
		if (mcv.wait()) {
			return;
		}
		test_buf.push_back(ch);
		if (mtx1.unlock()) {
			return;
		}
		isix::wait_ms(100);
	};
	constexpr auto thr_a = [](char ch) {
		if (mtx2.lock()) {
			std::abort();
		}
		if (mtx1.lock()) {
			std::abort();
		}
		if (mcv.wait()) {
			std::abort();
		}
		test_buf.push_back(ch);
		if (mtx1.unlock()) {
			std::abort();
		}
		if (mtx2.unlock()) {
			std::abort();
		}
	};
	constexpr auto thr_b = [](char ch) {
		if (mtx2.lock()) {
			std::abort();
		}
		test_buf.push_back(ch);
		if (mtx2.unlock()) {
			std::abort();
		}
	};
	test_buf.clear();
	const auto old_prio = isix::task_change_prio(nullptr, isix::get_min_priority());
	TEST_ASSERT_EQUAL(isix::get_task_priority(), isix::get_task_inherited_priority());
	auto t1 = isix::thread_create_and_run(STK_SIZ, 5, 0, thr_a, 'A');
	auto t2 = isix::thread_create_and_run(STK_SIZ, 4, 0, thr, 'C');
	auto t3 = isix::thread_create_and_run(STK_SIZ, 3, 0, thr_b, 'B');
	TEST_ASSERT(t1);
	TEST_ASSERT(t2);
	TEST_ASSERT(t3);
	isix::wait_ms(100);
	mcv.signal();
	mcv.signal();
	isix::wait_ms(200);
	TEST_ASSERT_EQUAL_STRING("BAC", test_buf.c_str());
	//Restore org prio
	TEST_ASSERT_EQUAL(isix::get_min_priority(), isix::task_change_prio(nullptr, old_prio));
}

TEST(mutex, condvar_wait_for_timeout_and_not_owning_mutex)
{
	//! Mutex 1 shouldnt be aquired
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(0, ((mtx_hacker*)&mtx1)->mtx->count);
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
	//! Lock the mutex and wait for timeout
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.lock());
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, mcv.wait(100));
	//! Mutex should be
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(0, ((mtx_hacker*)&mtx1)->mtx->count);
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
	//! Locking and unlocking should be possible
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.try_lock());
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx1.unlock());
	//! Mutex should be empty
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(0, ((mtx_hacker*)&mtx1)->mtx->count);
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
	// Wait without owning mutexes should casue error
	TEST_ASSERT_EQUAL(ISIX_EINVARG, mcv.wait());
}

TEST(mutex, condvar_task_destroy_API)
{
	test_buf.clear();
	oscondvar_t cv = isix::condvar_create();
	TEST_ASSERT_NOT_NULL(cv);
	const auto thr = [&](char ch)
	{
		if ((mtx1.lock())) {
			return;
		}
		auto ret = isix::condvar_wait(cv, ISIX_TIME_INFINITE);
		if (ret == ISIX_EDESTROY) {
			test_buf.push_back(ch);
			return;
		}
		//NOTE: Don't unlock mtx1 it should be unlocked automaticaly
		//bug(mtx1.unlock());
	};
	auto t1 = isix::thread_create_and_run(STK_SIZ, 5, 0, thr, 'E');
	auto t2 = isix::thread_create_and_run(STK_SIZ, 4, 0, thr, 'D');
	auto t3 = isix::thread_create_and_run(STK_SIZ, 3, 0, thr, 'C');
	auto t4 = isix::thread_create_and_run(STK_SIZ, 2, 0, thr, 'B');
	auto t5 = isix::thread_create_and_run(STK_SIZ, 1, 0, thr, 'A');
	TEST_ASSERT(t1);
	TEST_ASSERT(t2);
	TEST_ASSERT(t3);
	TEST_ASSERT(t4);
	TEST_ASSERT(t5);
	isix::wait_ms(200);
	TEST_ASSERT(test_buf.empty());
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::condvar_destroy(cv));
	isix::wait_ms(500);
	TEST_ASSERT_EQUAL_STRING("ABCDE", test_buf.c_str());
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(0, ((mtx_hacker*)&mtx1)->mtx->count);
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
}


TEST(mutex, inheritance_to_owner_queued_behind_higher_waiter)
{
	mx_a = isix_mutex_create(nullptr);
	mx_b = isix_mutex_create(nullptr);
	TEST_ASSERT(mx_a && mx_b);
	// Test thread holds B, O owns A and waits for B behind W, H blocks on A
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_mutex_lock(mx_b));
	static constexpr auto owner = [](void*) {
		isix_mutex_lock(mx_a);
		isix_mutex_lock(mx_b);
		mx_order.push_back('O');
		isix_mutex_unlock(mx_b);
		isix_mutex_unlock(mx_a);
	};
	static constexpr auto waiter = [](void*) {
		isix_mutex_lock(mx_b);
		mx_order.push_back('W');
		isix_mutex_unlock(mx_b);
	};
	static constexpr auto high = [](void*) {
		isix_mutex_lock(mx_a);
		mx_order.push_back('H');
		isix_mutex_unlock(mx_a);
	};
	const auto to = mx_tasks.spawn(owner, nullptr, 8);
	isix::wait_ms(10);
	const auto tw = mx_tasks.spawn(waiter, nullptr, 3);
	isix::wait_ms(10);
	const auto th = mx_tasks.spawn(high, nullptr, 5);
	isix::wait_ms(10);
	TEST_ASSERT(to && tw && th);
	TEST_ASSERT_EQUAL(5, isix_get_task_inherited_priority(to));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_mutex_unlock(mx_b));
	isix::wait_ms(50);
	TEST_ASSERT_EQUAL_STRING("WOH", mx_order.c_str());
}

TEST(mutex, kill_owner_of_two_mutexes_keeps_lists)
{
	mx_a = isix_mutex_create(nullptr);
	mx_b = isix_mutex_create(nullptr);
	mx_c = isix_mutex_create(nullptr);
	TEST_ASSERT(mx_a && mx_b && mx_c);
	static constexpr auto victim = [](void*) {
		isix_mutex_lock(mx_a);
		isix_mutex_lock(mx_b);
		isix_wait_ms(5000);
	};
	static constexpr auto waiter = [](void*) {
		isix_mutex_lock(mx_a);
		mx_step = 1;
		isix_wait_ms(60);
		isix_mutex_lock(mx_c);
		isix_mutex_unlock(mx_c);
		isix_mutex_unlock(mx_a);
		mx_step = 2;
	};
	mx_step = 0;
	const auto tv = mx_tasks.spawn(victim, nullptr, 5);
	isix::wait_ms(10);
	const auto tw = mx_tasks.spawn(waiter, nullptr, 3);
	isix::wait_ms(10);
	TEST_ASSERT(tv && tw);
	isix_task_kill(tv);
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(1, mx_step);
	TEST_ASSERT_EQUAL(1U, count_owned(tw));
	TEST_ASSERT_TRUE(owns_first(tw, mx_a));
	isix::wait_ms(100);
	TEST_ASSERT_EQUAL(2, mx_step);
	TEST_ASSERT_EQUAL(0U, count_owned(tw));
}

TEST(mutex, unlock_all_with_waiters_then_relock)
{
	mx_a = isix_mutex_create(nullptr);
	mx_b = isix_mutex_create(nullptr);
	mx_c = isix_mutex_create(nullptr);
	TEST_ASSERT(mx_a && mx_b && mx_c);
	test_utils::lower_test_prio(10);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_mutex_lock(mx_a));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_mutex_lock(mx_b));
	static constexpr auto wa = [](void*) {
		isix_mutex_lock(mx_a);
		isix_wait_ms(60);
		isix_mutex_unlock(mx_a);
	};
	static constexpr auto wb = [](void*) {
		isix_mutex_lock(mx_b);
		isix_wait_ms(60);
		isix_mutex_unlock(mx_b);
	};
	const auto ta = mx_tasks.spawn(wa, nullptr, 3);
	const auto tb = mx_tasks.spawn(wb, nullptr, 4);
	TEST_ASSERT(ta && tb);
	TEST_ASSERT_EQUAL(3, isix_get_task_inherited_priority(nullptr));
	isix_mutex_unlock_all();
	TEST_ASSERT_EQUAL(10, isix_get_task_inherited_priority(nullptr));
	TEST_ASSERT_EQUAL(0U, count_owned(isix_task_self()));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_mutex_lock(mx_c));
	TEST_ASSERT_EQUAL(1U, count_owned(ta));
	TEST_ASSERT_EQUAL(1U, count_owned(tb));
	TEST_ASSERT_EQUAL(1U, count_owned(isix_task_self()));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_mutex_unlock(mx_c));
}

TEST(mutex, destroy_restores_owner_priority)
{
	mx_a = isix_mutex_create(nullptr);
	TEST_ASSERT_NOT_NULL(mx_a);
	static constexpr auto low = [](void*) {
		isix_mutex_lock(mx_a);
		isix_wait_ms(500);
	};
	static constexpr auto high = [](void*) {
		isix_mutex_lock(mx_a);
	};
	const auto tl = mx_tasks.spawn(low, nullptr, 10);
	isix::wait_ms(10);
	const auto th = mx_tasks.spawn(high, nullptr, 3);
	isix::wait_ms(10);
	TEST_ASSERT(tl && th);
	TEST_ASSERT_EQUAL(3, isix_get_task_inherited_priority(tl));
	const auto m = mx_a;
	mx_a = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_mutex_destroy(m));
	TEST_ASSERT_EQUAL(10, isix_get_task_inherited_priority(tl));
}

TEST(mutex, condvar_wait_eok_when_mutex_contended)
{
	mx_a = isix_mutex_create(nullptr);
	mx_cv = isix_condvar_create();
	TEST_ASSERT(mx_a && mx_cv);
	mx_res = -12345;
	mx_owner_after = false;
	static constexpr auto waiter = [](void*) {
		isix_mutex_lock(mx_a);
		mx_res = isix_condvar_wait(mx_cv, 3000);
		mx_owner_after = (mx_a->owner == isix_task_self());
		if (mx_owner_after) isix_mutex_unlock(mx_a);
		mx_step = 2;
	};
	static constexpr auto signaler = [](void*) {
		isix_mutex_lock(mx_a);
		isix_condvar_signal(mx_cv);
		test_utils::cpu_busy(5);
		isix_mutex_unlock(mx_a);
	};
	mx_step = 0;
	const auto tw = mx_tasks.spawn(waiter, nullptr, 3);
	isix::wait_ms(20);
	const auto ts = mx_tasks.spawn(signaler, nullptr, 8);
	TEST_ASSERT(tw && ts);
	isix::wait_ms(50);
	TEST_ASSERT_EQUAL(2, mx_step);
	TEST_ASSERT_EQUAL(ISIX_EOK, mx_res);
	TEST_ASSERT_TRUE(mx_owner_after);
}

TEST(mutex, kill_owner_runs_new_owner_immediately)
{
	mx_a = isix_mutex_create(nullptr);
	TEST_ASSERT_NOT_NULL(mx_a);
	test_utils::lower_test_prio(10);
	static constexpr auto owner = [](void*) {
		isix_mutex_lock(mx_a);
		isix_wait_ms(5000);
	};
	static constexpr auto waiter = [](void*) {
		isix_mutex_lock(mx_a);
		mx_flag = true;
		isix_mutex_unlock(mx_a);
	};
	mx_flag = false;
	const auto to = mx_tasks.spawn(owner, nullptr, 12);
	isix::wait_ms(10);
	const auto tw = mx_tasks.spawn(waiter, nullptr, 5);
	isix::wait_ms(10);
	TEST_ASSERT(to && tw);
	TEST_ASSERT_FALSE(mx_flag);
	isix_task_kill(to);
	TEST_ASSERT_TRUE(mx_flag);
}

TEST_GROUP_RUNNER(mutex)
{
	RUN_TEST_CASE(mutex, inheritance_to_owner_queued_behind_higher_waiter);
	RUN_TEST_CASE(mutex, kill_owner_of_two_mutexes_keeps_lists);
	RUN_TEST_CASE(mutex, unlock_all_with_waiters_then_relock);
	RUN_TEST_CASE(mutex, destroy_restores_owner_priority);
	RUN_TEST_CASE(mutex, condvar_wait_eok_when_mutex_contended);
	RUN_TEST_CASE(mutex, kill_owner_runs_new_owner_immediately);
	RUN_TEST_CASE(mutex, delivery_order);
	RUN_TEST_CASE(mutex, priority_inheritance_basic_conditions);
	RUN_TEST_CASE(mutex, priority_inheritance_complex);
	RUN_TEST_CASE(mutex, priority_inheritance_mutex_priority_values);
	RUN_TEST_CASE(mutex, multiple_mutex_lock_from_same_thread);
	RUN_TEST_CASE(mutex, high_priority_mutex_order);
	RUN_TEST_CASE(mutex, abandoned_mutex_when_task_is_destroyed);
	RUN_TEST_CASE(mutex, kill_task_waiting_on_mutex);
	RUN_TEST_CASE(mutex, kill_waiter_drops_inherited_prio);
	RUN_TEST_CASE(mutex, suspend_resume_waiter_keeps_waiting);
	RUN_TEST_CASE(mutex, awake_tasks_when_mutex_is_destroyed);
	RUN_TEST_CASE(mutex, condition_variable_order);
	RUN_TEST_CASE(mutex, condvar_mutex_signaling);
	RUN_TEST_CASE(mutex, condtion_wait_priority_boost);
	RUN_TEST_CASE(mutex, condvar_wait_for_timeout_and_not_owning_mutex);
	RUN_TEST_CASE(mutex, condvar_task_destroy_API);
}
