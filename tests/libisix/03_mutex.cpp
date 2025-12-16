#define UNITY_FIXTURE_NO_EXTRAS
#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <isix/prv/list.h>
#include <isix/prv/mutex.h>
#include <string>

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

	void cpu_busy(unsigned ms_duration) {
		const auto t1 = isix::get_jiffies();
		do {
			asm volatile("nop\n");
		} while (!isix::timer_elapsed(t1, ms_duration));
	}
}


TEST_GROUP(mutex);
TEST_SETUP(mutex) {}
TEST_TEAR_DOWN(mutex) {}

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
	TEST_ASSERT_EQUAL_CHAR_ARRAY("EDCBA", test_buf.c_str(), test_buf.size());
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
	TEST_ASSERT_EQUAL_CHAR_ARRAY("ABC", test_buf.c_str(), test_buf.size());
	ostick_t max {};
	for(auto v: fin) {
		max = std::max(max, v - t1);
	}
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(70U, max);
	TEST_ASSERT_LESS_THAN_UINT(105U, max);
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
	TEST_ASSERT_EQUAL_CHAR_ARRAY("ABCDE", test_buf.c_str(), test_buf.size());
	ostick_t max {};
	for(auto v: fin) {
		max = std::max(max, v - t1);
	}
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(65U, max);
	TEST_ASSERT_LESS_THAN_UINT(110U, max);
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
	TEST_ASSERT_EQUAL_CHAR_ARRAY("ABCDE", test_buf.c_str(), test_buf.size());
}

/* Five tasks take a mutex and wait for
	* task abandon when the owner task is destroyed */
TEST(mutex, abandoned_mutex_when_task_is_destroyed)
{
	int fin[5] { -9999, -9999, -9999, -9999, -9999 };
	const auto thread = [](int& res)
	{
		res = mtx1.lock();
		for(;;) isix_wait_ms(100);
	};
	auto thr1 = isix::thread_create_and_run(STK_SIZ,4,0,thread,std::ref(fin[0]));
	TEST_ASSERT(thr1);
	isix::wait_ms(10);
	auto thr2 = isix::thread_create_and_run(STK_SIZ,5,0,thread,std::ref(fin[1]));
	auto thr3 = isix::thread_create_and_run(STK_SIZ,5,0,thread,std::ref(fin[2]));
	auto thr4 = isix::thread_create_and_run(STK_SIZ,5,0,thread,std::ref(fin[3]));
	auto thr5 = isix::thread_create_and_run(STK_SIZ,5,0,thread,std::ref(fin[4]));
	isix::wait_ms(10);
	TEST_ASSERT(thr2);
	TEST_ASSERT(thr3);
	TEST_ASSERT(thr4);
	TEST_ASSERT(thr5);
	thr1.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[0]);
	thr2.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[1]);
	thr3.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[2]);
	thr4.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[3]);
	thr5.kill();
	isix::wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, fin[4]);
	//Final test for the mutex state
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(0, ((mtx_hacker*)&mtx1)->mtx->count);
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
	isix::wait_ms(20);
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
	TEST_ASSERT_EQUAL_CHAR_ARRAY("ABCDE", test_buf.c_str(), test_buf.size());
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
	TEST_ASSERT_EQUAL_CHAR_ARRAY("ABCDE", test_buf.c_str(), test_buf.size());
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
	TEST_ASSERT_EQUAL_CHAR_ARRAY("BAC", test_buf.c_str(), test_buf.size());
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
	TEST_ASSERT_EQUAL_CHAR_ARRAY("ABCDE", test_buf.c_str(), test_buf.size());
	TEST_ASSERT_NULL(((mtx_hacker*)&mtx1)->mtx->owner);
	TEST_ASSERT_EQUAL(0, ((mtx_hacker*)&mtx1)->mtx->count);
	TEST_ASSERT(list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
}

TEST_GROUP_RUNNER(mutex)
{
	RUN_TEST_CASE(mutex, delivery_order);
	RUN_TEST_CASE(mutex, priority_inheritance_basic_conditions);
	RUN_TEST_CASE(mutex, priority_inheritance_complex);
	RUN_TEST_CASE(mutex, priority_inheritance_mutex_priority_values);
	RUN_TEST_CASE(mutex, multiple_mutex_lock_from_same_thread);
	RUN_TEST_CASE(mutex, high_priority_mutex_order);
	RUN_TEST_CASE(mutex, abandoned_mutex_when_task_is_destroyed);
	RUN_TEST_CASE(mutex, awake_tasks_when_mutex_is_destroyed);
	RUN_TEST_CASE(mutex, condition_variable_order);
	RUN_TEST_CASE(mutex, condvar_mutex_signaling);
	RUN_TEST_CASE(mutex, condtion_wait_priority_boost);
	RUN_TEST_CASE(mutex, condvar_wait_for_timeout_and_not_owning_mutex);
	RUN_TEST_CASE(mutex, condvar_task_destroy_API);
}