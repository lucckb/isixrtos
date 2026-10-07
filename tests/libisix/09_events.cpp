#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <cstring>
#include <utils/test_prio.hpp>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/events.h>
#undef _ISIX_KERNEL_CORE_


namespace {
	constexpr auto c_stack_size = 1024U;
	constexpr auto c_stack_margin = 100U;

namespace base {
	constexpr unsigned EV0 = 1U<<0;
	constexpr unsigned EV1 = 1U<<1;
	constexpr unsigned EV2 = 1U<<2;
	const unsigned EV3 = 1U<<3;

	//Task for post events
	class task_post {
	public:
		task_post(osevent_t _ev)
			: ev(_ev)
			, m_thr(isix::thread_create(std::bind(&task_post::thread,std::ref(*this))))
		{}
		void start(osprio_t prio) {
			m_thr.start_thread(c_stack_size, prio);
		}
		task_post& operator=(task_post&) = delete;
		task_post(task_post&) = delete;
	private:
		void thread() noexcept
		{
			isix_wait_ms(20);
			static constexpr auto nposts = 1;
			for (unsigned n=0; n<nposts; ++n) {
				isix_event_set(ev,  EV0|EV1);
			}
		}
	private:
		osevent_t ev {};
		isix::thread m_thr;
	};

	//Task for listen event
	class task_listen {
		static constexpr auto STACK_SIZE = 1024;
	public:
		task_listen(osevent_t _ev, unsigned _id)
			: ev(_ev), id(_id)
			, m_thr(isix::thread_create(std::bind(&task_listen::thread,std::ref(*this))))
		{}
		void start(osprio_t prio) {
			m_thr.start_thread(STACK_SIZE, prio);
		}
		task_listen(task_listen&) = delete;
		task_listen& operator=(task_listen&) = delete;
	private:
		void thread() noexcept
		{
			while (1) {
				isix_event_wait(ev, id, true, true, ISIX_TIME_INFINITE);
			}
		}
	private:
		osevent_t ev {};
		unsigned id {};
		isix::thread m_thr;
	};
}
}

//Namespace for fifo conn
namespace {
namespace tfifo {
	struct evfifo
	{
		osfifo_t fifo1 {};
		osfifo_t fifo2 {};
		osevent_t ev {};
		ostask_t t1 {};
		ostask_t t2 {};
		int err1 {};
		int err2 {};
	};
	// Group state lives here so teardown can clean up after a failed test
	evfifo g_fstr;
	void fifo_task1(void *ptr)
	{
		constexpr auto ch = 'A';
		auto& dt = *static_cast<evfifo*>(ptr);
		for (;;) {
			dt.err1 = isix_fifo_write(dt.fifo1, &ch, ISIX_TIME_INFINITE);
			isix_task_suspend(nullptr);
		}
	}
	void fifo_task2(void *ptr)
	{
		constexpr auto ch = 'B';
		auto& dt = *static_cast<evfifo*>(ptr);
		for (;;) {
			dt.err2 = isix_fifo_write(dt.fifo2, &ch, ISIX_TIME_INFINITE);
			isix_task_suspend(nullptr);
		}
	}
}
}


namespace destroy_waiter {
	enum class mode { wait, sync };
	struct params {
		osevent_t ev;
		mode m;
		volatile int result;
	};
	void waiter(void* arg)
	{
		auto* const p = static_cast<params*>(arg);
		constexpr auto bit0 = 1U << 0;
		constexpr auto bit1 = 1U << 1;
		p->result = (p->m == mode::wait)
			? isix_event_wait(p->ev, bit0, true, true, 1000)
			: isix_event_sync(p->ev, bit1, bit0|bit1, 1000);
	}
	// Destroy an event while a lower priority task waits on it, then check the freed block is untouched
	void run(mode m)
	{
		constexpr auto pattern = 0xFF;
		constexpr auto ev_size = sizeof(struct isix_event);
		params p { isix_event_create(), m, 1 };
		TEST_ASSERT(p.ev);
		auto* const ev = p.ev;
		const auto th = isix_task_create(waiter, &p, c_stack_size, 3, isix_task_flag_ref);
		TEST_ASSERT(th);
		for (int n = 0; n < 100 && isix_get_task_state(th) != OSTHR_STATE_WTEVT; ++n) {
			isix_wait_ms(1);
		}
		TEST_ASSERT_EQUAL(OSTHR_STATE_WTEVT, isix_get_task_state(th));
		TEST_ASSERT_EQUAL(ISIX_EOK, isix_event_destroy(ev));
		auto* const buf = static_cast<unsigned char*>(isix_alloc(ev_size));
		TEST_ASSERT(buf);
		const bool reused = (buf == reinterpret_cast<unsigned char*>(ev));
		if (reused) {
			std::memset(buf, pattern, ev_size);
		}
		isix_wait_ms(20);
		bool intact = true;
		if (reused) {
			for (size_t n = 0; n < ev_size; ++n) {
				if (buf[n] != pattern) { intact = false; break; }
			}
		}
		isix_free(buf);
		isix_task_wait_for(th);
		isix_task_unref(th);
		TEST_ASSERT_EQUAL(ISIX_EDESTROY, p.result);
		TEST_ASSERT_TRUE(intact);
	}
}

namespace sync_prio {
	constexpr auto h_bit = 1U << 0;
	constexpr auto l_bit = 1U << 1;
	constexpr int n_loops = 100;
	struct params {
		osevent_t ev;
		volatile int count;
	};
	void partner(void* arg)
	{
		auto* const p = static_cast<params*>(arg);
		for (int n = 0; n < n_loops; ++n) {
			if (isix_event_wait(p->ev, h_bit, false, true, 1000) < 0) { return; }
			isix_event_set(p->ev, l_bit);
			p->count = p->count + 1;
		}
	}
	// Run the sync loop in the current task against a partner task of the given priority
	void run(osprio_t partner_prio)
	{
		params p { isix_event_create(), 0 };
		TEST_ASSERT(p.ev);
		const auto th = isix_task_create(partner, &p, c_stack_size, partner_prio, isix_task_flag_ref);
		TEST_ASSERT(th);
		int bad = 0;
		for (int n = 0; n < n_loops; ++n) {
			const auto r = isix_event_sync(p.ev, h_bit, h_bit|l_bit, 1000);
			if (r < 0 || (r & (h_bit|l_bit)) != (h_bit|l_bit)) { ++bad; break; }
		}
		isix_task_wait_for(th);
		isix_task_unref(th);
		isix_event_destroy(p.ev);
		TEST_ASSERT_EQUAL(0, bad);
		TEST_ASSERT_EQUAL(n_loops, p.count);
	}
}

TEST_GROUP(events);
TEST_SETUP(events) {}
namespace {
	test_utils::task_pool ev_tasks;
	osevent_t ev_obj;
	volatile unsigned ev_res;
	volatile bool ev_done;
	constexpr unsigned EVA = 1U << 0;
	constexpr unsigned EVB = 1U << 1;
}

TEST_TEAR_DOWN(events)
{
	using namespace tfifo;
	test_utils::restore_test_prio();
	ev_tasks.release();
	if (ev_obj) { isix_event_destroy(ev_obj); ev_obj = nullptr; }
	if (g_fstr.t1) { isix_task_kill(g_fstr.t1); }
	if (g_fstr.t2) { isix_task_kill(g_fstr.t2); }
	if (g_fstr.fifo1 && g_fstr.ev) { isix_fifo_event_disconnect(g_fstr.fifo1, g_fstr.ev); }
	if (g_fstr.fifo2 && g_fstr.ev) { isix_fifo_event_disconnect(g_fstr.fifo2, g_fstr.ev); }
	if (g_fstr.fifo1) { isix_fifo_destroy(g_fstr.fifo1); }
	if (g_fstr.fifo2) { isix_fifo_destroy(g_fstr.fifo2); }
	if (g_fstr.ev) { isix_event_destroy(g_fstr.ev); }
	g_fstr = {};
}

TEST(events, api_base)
{
	using namespace base;
	osevent_t ev = isix_event_create();
	TEST_ASSERT(ev);
	task_listen t1(ev, EV0);
	task_listen t2(ev, EV1|EV3);
	task_listen t3(ev, EV2);
	task_listen t4(ev, EV3);
	task_post tp(ev);
	t1.start(3);
	t2.start(3);
	t3.start(3);
	t4.start(3);
	tp.start(3);
	isix_wait_ms(5000);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_event_destroy(ev));
}

TEST(events, sync)
{
	static constexpr auto TASK_0_BIT  = (1U << 0);
	static constexpr auto TASK_1_BIT  = (1U << 1);
	static constexpr auto TASK_2_BIT  = (1U << 2);
	static constexpr auto ALL_SYNC_BITS  = TASK_0_BIT|TASK_1_BIT|TASK_2_BIT;
	osevent_t ev = isix_event_create();
	struct mystat {
		unsigned err {};
		unsigned ok {};
	} stats[3];
	const auto evsync_thr = [&](mystat& stat, unsigned bit)
	{
		for (;;) {
			auto ret = isix_event_sync(ev, bit, ALL_SYNC_BITS, ISIX_TIME_INFINITE);
			if ((ret&ALL_SYNC_BITS)==ALL_SYNC_BITS) {
				++stat.ok;
			} else {
				--stat.err;
				return;
			}
		}
	};
	TEST_ASSERT(ev);
	constexpr auto test_prio = 3;
	auto t1 = isix::thread_create_and_run(
		c_stack_size ,test_prio, 0, evsync_thr, std::ref(stats[0]), TASK_0_BIT
	);
	auto t2 = isix::thread_create_and_run(
		c_stack_size ,test_prio, 0, evsync_thr, std::ref(stats[1]), TASK_1_BIT
	);
	auto t3 = isix::thread_create_and_run(
		c_stack_size ,test_prio, 0, evsync_thr, std::ref(stats[2]), TASK_2_BIT
	);
	TEST_ASSERT(t1);
	TEST_ASSERT(t2);
	TEST_ASSERT(t3);
	isix_wait_ms(5000);
	t1.kill();
	t2.kill();
	t3.kill();
	for (auto& stat : stats) {
		TEST_ASSERT_GREATER_THAN_UINT(10000U, stat.ok);
		TEST_ASSERT_EQUAL_UINT(0U, stat.err);
	}
	auto df1 = std::abs(int(stats[0].ok - stats[1].ok));
	auto df2 = std::abs(int(stats[2].ok - stats[1].ok));
	TEST_ASSERT_LESS_THAN(3, df1);
	TEST_ASSERT_LESS_THAN(3, df2);
	isix_wait_ms(50);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_event_destroy(ev));
}

TEST(events, fifo_conn_api)
{
	using namespace tfifo;
	static constexpr auto EV1 = 1U<<0;
	static constexpr auto EV2 = 1U<<1;
	auto& fstr = g_fstr;
	fstr = {};
	fstr.fifo1 = isix_fifo_create(16, sizeof(char));
	fstr.fifo2 = isix_fifo_create(16, sizeof(char));
	fstr.ev = isix_event_create();
	//Check fifos
	TEST_ASSERT(fstr.fifo1);
	TEST_ASSERT(fstr.fifo2);
	TEST_ASSERT(fstr.ev);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_fifo_event_connect(fstr.fifo1,fstr.ev,0));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_fifo_event_connect(fstr.fifo2,fstr.ev,1));
	const auto t1 = isix_task_create(fifo_task1, &fstr, c_stack_size, 3, 0);
	const auto t2 = isix_task_create(fifo_task2, &fstr, c_stack_size, 3, 0);
	fstr.t1 = t1;
	fstr.t2 = t2;
	TEST_ASSERT(t1);
	TEST_ASSERT(t2);
	osbitset_t abits=0;
	for (int c=0;c<10;) {
		auto sbits = isix::event_wait(fstr.ev, EV1|EV2, true, false);
		if (sbits & EV1) {
			char ch;
			TEST_ASSERT_EQUAL(ISIX_EOK, isix::fifo_read(fstr.fifo1,&ch));
			TEST_ASSERT_EQUAL_CHAR('A', ch);
		}
		if (sbits & EV2) {
			char ch;
			TEST_ASSERT_EQUAL(ISIX_EOK, isix::fifo_read(fstr.fifo2,&ch));
			TEST_ASSERT_EQUAL_CHAR('B', ch);
		}
		abits |= sbits;
		if ((abits&(EV1|EV2)) == (EV1|EV2)) {
			c++;
			// Wait until both writers are suspended again
			int tries = 0;
			while (isix::get_task_state(t1) != OSTHR_STATE_SUSPEND ||
					isix::get_task_state(t2) != OSTHR_STATE_SUSPEND) {
				TEST_ASSERT_LESS_THAN_MESSAGE(200, tries++, "fifo writers did not suspend");
				isix_wait_ms(1);
			}
			abits = 0;
			TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_resume(t1));
			TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_resume(t2));
		}
	}
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_fifo_event_disconnect(fstr.fifo1,fstr.ev));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_fifo_event_disconnect(fstr.fifo2,fstr.ev));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_fifo_destroy(fstr.fifo1));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_fifo_destroy(fstr.fifo2));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_event_destroy(fstr.ev));
	isix_task_kill(t1);
	isix_task_kill(t2);
	fstr = {};
}


TEST(events, destroy_with_lower_prio_waiter)
{
	destroy_waiter::run(destroy_waiter::mode::wait);
}

TEST(events, sync_destroy_with_lower_prio_waiter)
{
	destroy_waiter::run(destroy_waiter::mode::sync);
}

TEST(events, sync_wakes_lower_prio)
{
	// Test thread has the highest priority: sync waits and a lower priority partner is woken
	sync_prio::run(3);
	// Test thread drops below the partner: sync wakes a higher priority task
	const auto orig_prio = isix_get_task_priority(nullptr);
	TEST_ASSERT_GREATER_OR_EQUAL(0, orig_prio);
	TEST_ASSERT_GREATER_OR_EQUAL(0, isix_task_change_prio(nullptr, 5));
	sync_prio::run(3);
	isix_task_change_prio(nullptr, orig_prio);
}


TEST(events, suspend_resume_waiter_keeps_waiting)
{
	ev_obj = isix_event_create();
	TEST_ASSERT_NOT_NULL(ev_obj);
	ev_done = false;
	ev_res = 0xdeadU;
	static constexpr auto waiter = [](void*) {
		ev_res = isix_event_wait(ev_obj, EVA, false, false, 3000);
		ev_done = true;
	};
	const auto t = ev_tasks.spawn(waiter, nullptr, 3);
	TEST_ASSERT_NOT_NULL(t);
	isix::wait_ms(20);
	isix_task_suspend(t);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_task_resume(t));
	isix::wait_ms(20);
	TEST_ASSERT_FALSE(ev_done);
	isix_event_set(ev_obj, EVA);
	isix::wait_ms(20);
	TEST_ASSERT_TRUE(ev_done);
	TEST_ASSERT_EQUAL_UINT(EVA, ev_res);
}

TEST(events, set_return_not_affected_by_woken_waiter)
{
	ev_obj = isix_event_create();
	TEST_ASSERT_NOT_NULL(ev_obj);
	static constexpr auto waiter = [](void*) {
		isix_event_wait(ev_obj, EVA, false, false, 3000);
		isix_event_set(ev_obj, EVB);
	};
	const auto t = ev_tasks.spawn(waiter, nullptr, 2);
	TEST_ASSERT_NOT_NULL(t);
	isix::wait_ms(20);
	test_utils::lower_test_prio(5);
	const auto res = isix_event_set(ev_obj, EVA);
	// The waiter preempted the setter and added EVB, the result must reflect the state at set time
	TEST_ASSERT_EQUAL_UINT(EVA, res);
	TEST_ASSERT_EQUAL_UINT(EVA | EVB, isix_event_get(ev_obj));
}

TEST(events, timeout_does_not_consume_bits)
{
	ev_obj = isix_event_create();
	TEST_ASSERT_NOT_NULL(ev_obj);
	ev_done = false;
	static constexpr auto waiter = [](void*) {
		ev_res = isix_event_wait(ev_obj, EVA, true, false, isix::ms2tick(5));
		ev_done = true;
	};
	const auto t = ev_tasks.spawn(waiter, nullptr, 10);
	TEST_ASSERT_NOT_NULL(t);
	isix::wait_ms(2);
	// The waiter times out but cannot run while the test thread is busy
	test_utils::cpu_busy(10);
	isix_event_set(ev_obj, EVA);
	isix::wait_ms(20);
	TEST_ASSERT_TRUE(ev_done);
	TEST_ASSERT_EQUAL_INT(ISIX_ETIMEOUT, static_cast<int>(ev_res));
	TEST_ASSERT_EQUAL_UINT(EVA, isix_event_get(ev_obj));
}

TEST_GROUP_RUNNER(events)
{
	RUN_TEST_CASE(events, suspend_resume_waiter_keeps_waiting);
	RUN_TEST_CASE(events, set_return_not_affected_by_woken_waiter);
	RUN_TEST_CASE(events, timeout_does_not_consume_bits);
	RUN_TEST_CASE(events, api_base);
	RUN_TEST_CASE(events, sync);
	RUN_TEST_CASE(events, fifo_conn_api);
	RUN_TEST_CASE(events, destroy_with_lower_prio_waiter);
	RUN_TEST_CASE(events, sync_destroy_with_lower_prio_waiter);
	RUN_TEST_CASE(events, sync_wakes_lower_prio);
}