/* Enable isix::co via isix.h only in this translation unit (keeps other tests lean). */
#define CONFIG_ISIX_CPP_COROUTINES 1

#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include "timer_interrupt.hpp"
#include <optional>
#include <tuple>

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#include <stdexcept>
#define CORO_UT_EXCEPTIONS
#endif

#if CONFIG_ISIX_CPP_COROUTINES

namespace {

constexpr auto c_stack_size = 2048U;
constexpr auto c_task_prio = 3;

//! Step the scheduler until pred() holds or the time limit elapses
template<typename F>
bool drive(isix::co::scheduler& sched, F&& pred, unsigned limit_ms = 2000U)
{
	const auto t0 = isix::get_jiffies();
	while (!pred()) {
		if (isix::timer_elapsed(t0, isix::ms2tick(limit_ms))) {
			return false;
		}
		sched.step();
	}
	return true;
}

size_t heap_used()
{
	isix::memory_stat st {};
	isix::heap_stats(st);
	return st.used;
}

} // namespace

TEST_GROUP(coroutines);
TEST_SETUP(coroutines) {}
TEST_TEAR_DOWN(coroutines)
{
	tests::detail::periodic_timer_stop();
}

TEST(coroutines, returns_value)
{
	auto t = []() -> isix::co::task<int> { co_return 7; };
	const int v = isix::co::run(t());
	TEST_ASSERT_EQUAL(7, v);
}

TEST(coroutines, event_cross_task)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	auto thr = isix::thread_create([&ev] {
		isix::wait_ms(40);
		ev.set();
	});
	thr.start_thread(c_stack_size, c_task_prio);

	int wait_rc { ISIX_ESTATE };
	auto coro = [&ev, &wait_rc]() -> isix::co::task<int> {
		wait_rc = co_await ev.wait_for(isix::ms2tick(400));
		co_return 42;
	};
	const int r = isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(ISIX_EOK, wait_rc);
	TEST_ASSERT_EQUAL(42, r);
	isix::wait_ms(50);
}

TEST(coroutines, sleep_sequence)
{
	// Do not call Unity TEST_* from inside the coroutine: Unity uses longjmp and
	// must not bypass coroutine suspend/resume (undefined behavior, breaks later tests).
	// Tickless may coalesce jiffies across a wait; use ujiffies for wall-ish progress.
	osutick_t u0 {};
	osutick_t u1 {};
	osutick_t u2 {};
	isix::co::scheduler sched;
	auto coro = [&]() -> isix::co::task<void> {
		u0 = isix::get_ujiffies();
		co_await isix::co::sleep_ms(30);
		u1 = isix::get_ujiffies();
		co_await isix::co::sleep_ms(30);
		u2 = isix::get_ujiffies();
	};
	isix::co::run(sched, coro());
	const osutick_t d1 = u1 - u0;
	const osutick_t d2 = u2 - u1;
	/* ~30 ms each; UINT_WITHIN(delta, expected, actual) => |actual-expected| <= delta */
	TEST_ASSERT_UINT_WITHIN(55ULL * 1000ULL, 30ULL * 1000ULL, d1);
	TEST_ASSERT_UINT_WITHIN(55ULL * 1000ULL, 30ULL * 1000ULL, d2);
}

TEST(coroutines, many_coroutines_interleave_on_one_thread)
{
	isix::co::scheduler sched;
	isix::co::event ev_a(sched);
	isix::co::event ev_b(sched);

	// Producer thread sets A then B.
	auto thr = isix::thread_create([&] {
		isix::wait_ms(20);
		ev_a.set();
		isix::wait_ms(20);
		ev_b.set();
	});
	thr.start_thread(c_stack_size, c_task_prio);

	int a_seen { 0 };
	int b_seen { 0 };

	auto coro_a = [&]() -> isix::co::task<void> {
		co_await ev_a;
		a_seen = 1;
		co_await isix::co::sleep_ms(5);
		co_return;
	};
	auto coro_b = [&]() -> isix::co::task<void> {
		co_await ev_b;
		b_seen = 1;
		co_return;
	};

	auto ta = coro_a();
	auto tb = coro_b();
	isix::co::spawn(sched, ta);
	isix::co::spawn(sched, tb);
	drive(sched, [&] { return ta.done() && tb.done(); });
	TEST_ASSERT(ta.done());
	TEST_ASSERT(tb.done());
	TEST_ASSERT_EQUAL(1, a_seen);
	TEST_ASSERT_EQUAL(1, b_seen);
}

TEST(coroutines, run_returns_when_other_coroutine_still_waiting)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	bool b_resumed { false };
	auto coro_b = [&]() -> isix::co::task<void> {
		co_await ev;
		b_resumed = true;
	};
	auto coro_a = []() -> isix::co::task<int> { co_return 5; };
	auto tb = coro_b();
	isix::co::spawn(sched, tb);
	const int r = isix::co::run(sched, coro_a());
	TEST_ASSERT_EQUAL(5, r);
	TEST_ASSERT_FALSE(tb.done());
	TEST_ASSERT_FALSE(b_resumed);
}

TEST(coroutines, sleep_already_due_does_not_hang)
{
	isix::co::scheduler sched;
	bool a_done { false };
	bool b_done { false };
	auto coro_a = [&]() -> isix::co::task<void> {
		co_await isix::co::sleep_ticks(1);
		a_done = true;
	};
	auto coro_b = [&]() -> isix::co::task<void> {
		const auto t0 = isix::get_jiffies();
		while (!isix::timer_elapsed(t0, isix::ms2tick(10))) {
		}
		b_done = true;
		co_return;
	};
	auto ta = coro_a();
	auto tb = coro_b();
	isix::co::spawn(sched, ta);
	isix::co::spawn(sched, tb);
	const bool ok = drive(sched, [&] { return a_done && b_done; });
	TEST_ASSERT_TRUE(ok);
}

TEST(coroutines, scheduler_teardown_with_armed_timer)
{
	const auto run_once = [] {
		isix::co::scheduler sched;
		isix::co::event ev(sched);
		auto thr = isix::thread_create([&ev] {
			isix::wait_ms(10);
			ev.set();
		});
		thr.start_thread(c_stack_size, c_task_prio);
		auto sleeper = []() -> isix::co::task<void> { co_await isix::co::sleep_ms(1000); };
		auto waiter = [&ev]() -> isix::co::task<void> { co_await ev; };
		auto ts = sleeper();
		isix::co::spawn(sched, ts);
		isix::co::run(sched, waiter());
	};
	run_once();
	isix::wait_ms(100);
	const auto before = heap_used();
	run_once();
	isix::wait_ms(100);
	TEST_ASSERT_EQUAL_UINT32(before, heap_used());
}

TEST(coroutines, many_schedulers_sequential_no_leak)
{
	const auto one_round = [](bool with_sleep) {
		isix::co::scheduler sched;
		isix::co::event ev(sched);
		if (with_sleep) {
			auto coro = []() -> isix::co::task<void> { co_await isix::co::sleep_ms(1); };
			isix::co::run(sched, coro());
		}
	};
	one_round(true);
	isix::wait_ms(50);
	const auto before = heap_used();
	for (int i = 0; i < 100; ++i) {
		one_round(false);
	}
	for (int i = 0; i < 5; ++i) {
		one_round(true);
	}
	isix::wait_ms(50);
	TEST_ASSERT_EQUAL_UINT32(before, heap_used());
}

TEST(coroutines, event_set_from_isr)
{
	constexpr int c_isr_sets = 10;
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	volatile int isr_count { 0 };
	volatile bool isr_done { false };
	int wakeups { 0 };
	int bad_rc { 0 };
	auto coro = [&]() -> isix::co::task<void> {
		while (!isr_done) {
			if (co_await ev.wait_for(isix::ms2tick(200)) == ISIX_EOK) {
				++wakeups;
			} else {
				++bad_rc;
				break;
			}
		}
	};
	const bool started = tests::detail::periodic_timer_setup([&] {
		if (isr_count < c_isr_sets) {
			isr_count = isr_count + 1;
			ev.set_isr();
			if (isr_count == c_isr_sets) {
				isr_done = true;
			}
		} else {
			tests::detail::periodic_timer_stop();
		}
	}, 1000);
	TEST_ASSERT_TRUE(started);
	isix::co::run(sched, coro());
	tests::detail::periodic_timer_stop();
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(0, bad_rc);
	TEST_ASSERT_TRUE(wakeups >= 1);
	TEST_ASSERT_TRUE(wakeups <= c_isr_sets);
	TEST_ASSERT_EQUAL(c_isr_sets, isr_count);
}

TEST(coroutines, event_set_before_wait_latched)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc1 { ISIX_ESTATE };
	int rc2 { ISIX_ESTATE };
	ev.set();
	ev.set();
	auto coro = [&]() -> isix::co::task<void> {
		rc1 = co_await ev;
		rc2 = co_await ev.wait_for(ISIX_TIME_DONTWAIT);
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(ISIX_EOK, rc1);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc2);
}

TEST(coroutines, event_wait_for_timeout)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc { ISIX_ESTATE };
	osutick_t u0 {};
	osutick_t u1 {};
	auto coro = [&]() -> isix::co::task<void> {
		u0 = isix::get_ujiffies();
		rc = co_await ev.wait_for(isix::ms2tick(30));
		u1 = isix::get_ujiffies();
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc);
	TEST_ASSERT_UINT_WITHIN(55ULL * 1000ULL, 30ULL * 1000ULL, u1 - u0);
}

TEST(coroutines, event_wait_for_dontwait)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc1 { ISIX_ESTATE };
	int rc2 { ISIX_ESTATE };
	auto coro = [&]() -> isix::co::task<void> {
		rc1 = co_await ev.wait_for(ISIX_TIME_DONTWAIT);
		ev.set();
		rc2 = co_await ev.wait_for(ISIX_TIME_DONTWAIT);
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc1);
	TEST_ASSERT_EQUAL(ISIX_EOK, rc2);
}

TEST(coroutines, event_timeout_then_set_no_double_resume)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	auto thr = isix::thread_create([&ev] {
		isix::wait_ms(40);
		ev.set();
	});
	thr.start_thread(c_stack_size, c_task_prio);
	int resumes { 0 };
	int rc { ISIX_ESTATE };
	osutick_t u0 {};
	osutick_t u1 {};
	auto coro = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(20));
		++resumes;
		u0 = isix::get_ujiffies();
		co_await isix::co::sleep_ms(60);
		u1 = isix::get_ujiffies();
		++resumes;
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc);
	TEST_ASSERT_EQUAL(2, resumes);
	TEST_ASSERT_TRUE((u1 - u0) >= 50ULL * 1000ULL);
	isix::wait_ms(30);
}

TEST(coroutines, event_wake_all_waiters)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rcs[3] { ISIX_ESTATE, ISIX_ESTATE, ISIX_ESTATE };
	const auto make = [&](int idx) -> isix::co::task<void> {
		rcs[idx] = co_await ev.wait_for(isix::ms2tick(500));
	};
	auto t0 = make(0);
	auto t1 = make(1);
	auto t2 = make(2);
	isix::co::spawn(sched, t0);
	isix::co::spawn(sched, t1);
	isix::co::spawn(sched, t2);
	auto thr = isix::thread_create([&ev] {
		isix::wait_ms(20);
		ev.set();
	});
	thr.start_thread(c_stack_size, c_task_prio);
	const bool ok = drive(sched, [&] { return t0.done() && t1.done() && t2.done(); });
	TEST_ASSERT_TRUE(ok);
	for (const auto rc : rcs) {
		TEST_ASSERT_EQUAL(ISIX_EOK, rc);
	}
	isix::wait_ms(30);
}

TEST(coroutines, task_destroy_while_waiting)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	bool resumed { false };
	{
		auto coro = [&]() -> isix::co::task<void> {
			std::ignore = co_await ev.wait_for(isix::ms2tick(50));
			resumed = true;
		};
		auto t = coro();
		isix::co::spawn(sched, t);
		sched.step();
		TEST_ASSERT_FALSE(t.done());
		t.destroy();
	}
	ev.set();
	isix::wait_ms(100);
	sched.step();
	TEST_ASSERT_FALSE(resumed);
}

TEST(coroutines, semaphore_and_mutex_coroutine_native)
{
	isix::co::scheduler sched;
	isix::co::semaphore sem(sched, 0, 1);
	isix::co::mutex mtx(sched);

	int sem_rc { ISIX_ESTATE };
	int lock_rc { ISIX_ESTATE };
	int counter { 0 };

	auto producer = isix::thread_create([&] {
		isix::wait_ms(20);
		sem.signal();
	});
	producer.start_thread(c_stack_size, c_task_prio);

	auto coro = [&]() -> isix::co::task<void> {
		sem_rc = co_await sem.take(isix::ms2tick(200));
		lock_rc = co_await mtx.lock(isix::ms2tick(200));
		if (lock_rc == ISIX_EOK) {
			++counter;
			mtx.unlock();
		}
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(ISIX_EOK, sem_rc);
	TEST_ASSERT_EQUAL(ISIX_EOK, lock_rc);
	TEST_ASSERT_EQUAL(1, counter);
	isix::wait_ms(10);
}

TEST(coroutines, semaphore_burst_from_thread)
{
	constexpr int c_tokens = 10;
	isix::co::scheduler sched;
	isix::co::semaphore sem(sched);
	int taken[2] {};
	int bad_rc { 0 };

	auto producer = isix::thread_create([&] {
		isix::wait_ms(10);
		for (int i = 0; i < c_tokens; ++i) {
			sem.signal();
		}
	});
	producer.start_thread(c_stack_size, c_task_prio);

	auto consumer = [&](int idx) -> isix::co::task<void> {
		for (;;) {
			const int rc = co_await sem.take(isix::ms2tick(100));
			if (rc == ISIX_EOK) {
				++taken[idx];
			} else {
				if (rc != ISIX_ETIMEOUT) {
					++bad_rc;
				}
				break;
			}
		}
	};
	auto t0 = consumer(0);
	auto t1 = consumer(1);
	isix::co::spawn(sched, t0);
	isix::co::spawn(sched, t1);
	const bool ok = drive(sched, [&] { return t0.done() && t1.done(); });
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(0, bad_rc);
	TEST_ASSERT_EQUAL(c_tokens, taken[0] + taken[1]);
	isix::wait_ms(10);
}

TEST(coroutines, semaphore_signal_from_isr)
{
	constexpr int c_isr_signals = 25;
	isix::co::scheduler sched;
	isix::co::semaphore sem(sched);
	volatile int isr_count { 0 };
	int taken { 0 };
	int last_rc { ISIX_ESTATE };
	auto coro = [&]() -> isix::co::task<void> {
		while (taken < c_isr_signals) {
			if (co_await sem.take(isix::ms2tick(200)) != ISIX_EOK) {
				co_return;
			}
			++taken;
		}
		last_rc = co_await sem.take(isix::ms2tick(100));
	};
	const bool started = tests::detail::periodic_timer_setup([&] {
		if (isr_count < c_isr_signals) {
			isr_count = isr_count + 1;
			sem.signal_isr();
		} else {
			tests::detail::periodic_timer_stop();
		}
	}, 1000);
	TEST_ASSERT_TRUE(started);
	isix::co::run(sched, coro());
	tests::detail::periodic_timer_stop();
	isix::wait_ms(10);
	TEST_ASSERT_EQUAL(c_isr_signals, isr_count);
	TEST_ASSERT_EQUAL(c_isr_signals, taken);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, last_rc);
}

TEST(coroutines, semaphore_take_timeout)
{
	isix::co::scheduler sched;
	isix::co::semaphore sem(sched, 1);
	int rc[3] { ISIX_ESTATE, ISIX_ESTATE, ISIX_ESTATE };
	ostick_t elapsed {};
	auto coro = [&]() -> isix::co::task<void> {
		rc[0] = co_await sem.take(isix::ms2tick(10));
		const auto t0 = isix::get_jiffies();
		rc[1] = co_await sem.take(isix::ms2tick(30));
		elapsed = isix::get_jiffies() - t0;
		rc[2] = co_await sem.take(ISIX_TIME_DONTWAIT);
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(ISIX_EOK, rc[0]);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc[1]);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc[2]);
	TEST_ASSERT_TRUE(elapsed >= isix::ms2tick(30));
	TEST_ASSERT_TRUE(elapsed < isix::ms2tick(100));
	TEST_ASSERT_FALSE(sem.try_take());
	isix::wait_ms(10);
}

TEST(coroutines, semaphore_limit)
{
	isix::co::scheduler sched;
	isix::co::semaphore sem(sched, 0, 2);
	for (int i = 0; i < 5; ++i) {
		sem.signal();
	}
	TEST_ASSERT_TRUE(sem.try_take());
	TEST_ASSERT_TRUE(sem.try_take());
	TEST_ASSERT_FALSE(sem.try_take());
}

TEST(coroutines, mutex_contention_fifo)
{
	isix::co::scheduler sched;
	isix::co::mutex mtx(sched);
	int inside { 0 };
	int max_inside { 0 };
	int order[3] {};
	int next { 0 };
	int bad_rc { 0 };
	auto worker = [&](int id) -> isix::co::task<void> {
		if (co_await mtx.lock() != ISIX_EOK) {
			++bad_rc;
			co_return;
		}
		++inside;
		max_inside = inside > max_inside ? inside : max_inside;
		order[next++] = id;
		co_await isix::co::sleep_ms(2);
		--inside;
		mtx.unlock();
	};
	auto t0 = worker(0);
	auto t1 = worker(1);
	auto t2 = worker(2);
	isix::co::spawn(sched, t0);
	isix::co::spawn(sched, t1);
	isix::co::spawn(sched, t2);
	const bool ok = drive(sched, [&] { return t0.done() && t1.done() && t2.done(); });
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(0, bad_rc);
	TEST_ASSERT_EQUAL(1, max_inside);
	TEST_ASSERT_EQUAL(3, next);
	TEST_ASSERT_EQUAL(0, order[0]);
	TEST_ASSERT_EQUAL(1, order[1]);
	TEST_ASSERT_EQUAL(2, order[2]);
	TEST_ASSERT_FALSE(mtx.is_locked());
}

TEST(coroutines, mutex_lock_timeout)
{
	isix::co::scheduler sched;
	isix::co::mutex mtx(sched);
	int rc { ISIX_ESTATE };
	int try_rc { ISIX_ESTATE };
	bool locked_after { false };
	auto holder = [&]() -> isix::co::task<void> {
		co_await mtx.lock();
		co_await isix::co::sleep_ms(60);
		mtx.unlock();
	};
	auto waiter = [&]() -> isix::co::task<void> {
		rc = co_await mtx.lock(isix::ms2tick(20));
		try_rc = co_await mtx.lock(ISIX_TIME_DONTWAIT);
		locked_after = mtx.is_locked();
	};
	auto th = holder();
	auto tw = waiter();
	isix::co::spawn(sched, th);
	isix::co::spawn(sched, tw);
	const bool ok = drive(sched, [&] { return th.done() && tw.done(); });
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, try_rc);
	TEST_ASSERT_TRUE(locked_after);
	TEST_ASSERT_FALSE(mtx.is_locked());
}

TEST(coroutines, mutex_scoped_lock)
{
	isix::co::scheduler sched;
	isix::co::mutex mtx(sched);
	bool held_inside { false };
	int err { ISIX_ESTATE };
	auto coro = [&]() -> isix::co::task<void> {
		{
			auto lk = co_await mtx.scoped();
			err = lk.error();
			held_inside = lk.owns_lock() && mtx.is_locked();
		}
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(ISIX_EOK, err);
	TEST_ASSERT_TRUE(held_inside);
	TEST_ASSERT_FALSE(mtx.is_locked());
	TEST_ASSERT_EQUAL(ISIX_ENOTLOCKED, mtx.unlock());
}

TEST(coroutines, channel_send_recv)
{
	isix::co::scheduler sched;
	isix::co::channel<int, 2> ch(sched);

	int got { -1 };
	bool recv_has_value { false };
	int send_rc { ISIX_ESTATE };
	auto receiver = [&]() -> isix::co::task<void> {
		auto v = co_await ch.recv(isix::ms2tick(200));
		recv_has_value = v.has_value();
		if (v.has_value()) {
			got = *v;
		}
	};
	auto sender = [&]() -> isix::co::task<void> {
		send_rc = co_await ch.send(123, isix::ms2tick(200));
	};

	auto tr = receiver();
	auto ts = sender();
	isix::co::spawn(sched, tr);
	isix::co::spawn(sched, ts);
	const bool ok = drive(sched, [&] { return tr.done() && ts.done(); });
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT(recv_has_value);
	TEST_ASSERT_EQUAL(ISIX_EOK, send_rc);
	TEST_ASSERT_EQUAL(123, got);
}

TEST(coroutines, channel_thread_producer_order)
{
	constexpr int c_items = 50;
	isix::co::scheduler sched;
	isix::co::channel<int, 4> ch(sched);
	int received { 0 };
	int out_of_order { 0 };
	bool timed_out { false };

	auto producer = isix::thread_create([&] {
		for (int i = 0; i < c_items; ++i) {
			while (!ch.try_send(i)) {
				isix::wait_ms(1);
			}
		}
	});
	producer.start_thread(c_stack_size, c_task_prio);

	auto consumer = [&]() -> isix::co::task<void> {
		while (received < c_items) {
			const auto v = co_await ch.recv(isix::ms2tick(500));
			if (!v) {
				timed_out = true;
				co_return;
			}
			if (*v != received) {
				++out_of_order;
			}
			++received;
		}
	};
	isix::co::run(sched, consumer());
	TEST_ASSERT_FALSE(timed_out);
	TEST_ASSERT_EQUAL(c_items, received);
	TEST_ASSERT_EQUAL(0, out_of_order);
	isix::wait_ms(10);
}

TEST(coroutines, channel_isr_producer)
{
	constexpr int c_items = 20;
	isix::co::scheduler sched;
	isix::co::channel<int, 8> ch(sched);
	volatile int next_item { 0 };
	int received { 0 };
	int out_of_order { 0 };
	bool timed_out { false };
	auto consumer = [&]() -> isix::co::task<void> {
		while (received < c_items) {
			const auto v = co_await ch.recv(isix::ms2tick(200));
			if (!v) {
				timed_out = true;
				co_return;
			}
			if (*v != received) {
				++out_of_order;
			}
			++received;
		}
	};
	const bool started = tests::detail::periodic_timer_setup([&] {
		const int n = next_item;
		if (n < c_items) {
			if (ch.try_send_isr(n)) {
				next_item = n + 1;
			}
		} else {
			tests::detail::periodic_timer_stop();
		}
	}, 1000);
	TEST_ASSERT_TRUE(started);
	isix::co::run(sched, consumer());
	tests::detail::periodic_timer_stop();
	isix::wait_ms(10);
	TEST_ASSERT_FALSE(timed_out);
	TEST_ASSERT_EQUAL(c_items, received);
	TEST_ASSERT_EQUAL(0, out_of_order);
}

TEST(coroutines, channel_backpressure_coroutines)
{
	constexpr int c_items = 10;
	isix::co::scheduler sched;
	isix::co::channel<int, 2> ch(sched);
	int sent { 0 };
	int received { 0 };
	int out_of_order { 0 };
	int bad_rc { 0 };
	auto sender = [&]() -> isix::co::task<void> {
		for (int i = 0; i < c_items; ++i) {
			if (co_await ch.send(i, isix::ms2tick(500)) != ISIX_EOK) {
				++bad_rc;
				co_return;
			}
			++sent;
		}
	};
	auto receiver = [&]() -> isix::co::task<void> {
		for (int i = 0; i < c_items; ++i) {
			const auto v = co_await ch.recv(isix::ms2tick(500));
			if (!v) {
				++bad_rc;
				co_return;
			}
			if (*v != i) {
				++out_of_order;
			}
			++received;
			co_await isix::co::sleep_ms(1);
		}
	};
	auto ts = sender();
	auto tr = receiver();
	isix::co::spawn(sched, ts);
	isix::co::spawn(sched, tr);
	const bool ok = drive(sched, [&] { return ts.done() && tr.done(); });
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(0, bad_rc);
	TEST_ASSERT_EQUAL(c_items, sent);
	TEST_ASSERT_EQUAL(c_items, received);
	TEST_ASSERT_EQUAL(0, out_of_order);
}

TEST(coroutines, channel_recv_timeout)
{
	isix::co::scheduler sched;
	isix::co::channel<int, 2> ch(sched);
	bool has_value { true };
	bool dontwait_has_value { true };
	ostick_t elapsed {};
	auto coro = [&]() -> isix::co::task<void> {
		const auto t0 = isix::get_jiffies();
		has_value = (co_await ch.recv(isix::ms2tick(30))).has_value();
		elapsed = isix::get_jiffies() - t0;
		dontwait_has_value = (co_await ch.recv(ISIX_TIME_DONTWAIT)).has_value();
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_FALSE(has_value);
	TEST_ASSERT_FALSE(dontwait_has_value);
	TEST_ASSERT_TRUE(elapsed >= isix::ms2tick(30));
	TEST_ASSERT_TRUE(elapsed < isix::ms2tick(100));
}

TEST(coroutines, channel_send_timeout_full)
{
	isix::co::scheduler sched;
	isix::co::channel<int, 1> ch(sched);
	int rc[2] { ISIX_ESTATE, ISIX_ESTATE };
	int first { -1 };
	bool second { true };
	auto coro = [&]() -> isix::co::task<void> {
		rc[0] = co_await ch.send(1, isix::ms2tick(20));
		rc[1] = co_await ch.send(2, isix::ms2tick(20));
		const auto v = co_await ch.recv(isix::ms2tick(20));
		first = v ? *v : -2;
		second = (co_await ch.recv(ISIX_TIME_DONTWAIT)).has_value();
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(ISIX_EOK, rc[0]);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc[1]);
	TEST_ASSERT_EQUAL(1, first);
	TEST_ASSERT_FALSE(second);
}

TEST(coroutines, await_nested_task_value)
{
	auto leaf = [](int x) -> isix::co::task<int> {
		co_await isix::co::sleep_ms(2);
		co_return x * 2;
	};
	auto mid = [&](int x) -> isix::co::task<int> {
		const int a = co_await leaf(x);
		const int b = co_await leaf(a);
		co_return a + b;
	};
	auto top = [&]() -> isix::co::task<int> {
		co_return co_await mid(3);
	};
	TEST_ASSERT_EQUAL(18, isix::co::run(top()));
}

TEST(coroutines, await_nested_void_and_event)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc { ISIX_ESTATE };
	int after { 0 };
	auto child = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(500));
	};
	auto parent = [&]() -> isix::co::task<void> {
		co_await child();
		after = 1;
	};
	auto setter = [&]() -> isix::co::task<void> {
		co_await isix::co::sleep_ms(5);
		ev.set();
	};
	auto tp = parent();
	auto ts = setter();
	isix::co::spawn(sched, tp);
	isix::co::spawn(sched, ts);
	const bool ok = drive(sched, [&] { return tp.done() && ts.done(); });
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(ISIX_EOK, rc);
	TEST_ASSERT_EQUAL(1, after);
}

TEST(coroutines, await_nested_destroy_parent_while_waiting)
{
	isix::co::scheduler sched;
	bool resumed { false };
	{
		auto child = [&]() -> isix::co::task<void> {
			co_await isix::co::sleep_ms(1000);
			resumed = true;
		};
		auto parent = [&]() -> isix::co::task<void> {
			co_await child();
			resumed = true;
		};
		auto tp = parent();
		isix::co::spawn(sched, tp);
		// Step 1 starts the parent, step 2 starts the child (then it sleeps)
		sched.step();
		sched.step();
		TEST_ASSERT_FALSE(tp.done());
	}
	TEST_ASSERT_FALSE(resumed);
}

TEST(coroutines, two_schedulers_two_threads)
{
	constexpr int c_items = 20;
	isix::co::scheduler sched_a;
	isix::co::scheduler sched_b;
	isix::co::channel<int, 4> to_a(sched_a);
	isix::co::channel<int, 4> to_b(sched_b);
	volatile int a_sum { 0 };
	volatile int b_sum { 0 };
	volatile int bad { 0 };
	volatile bool b_done { false };

	// Thread B: echoes every received value back to A, incremented by 100.
	auto thr_b = isix::thread_create([&] {
		auto echo = [&]() -> isix::co::task<void> {
			for (int i = 0; i < c_items; ++i) {
				const auto v = co_await to_b.recv(isix::ms2tick(500));
				if (!v) {
					bad = bad + 1;
					co_return;
				}
				b_sum = b_sum + *v;
				while (!to_a.try_send(*v + 100)) {
					co_await isix::co::sleep_ms(1);
				}
			}
		};
		isix::co::run(sched_b, echo());
		b_done = true;
	});
	thr_b.start_thread(c_stack_size, c_task_prio);

	auto main_coro = [&]() -> isix::co::task<void> {
		for (int i = 0; i < c_items; ++i) {
			while (!to_b.try_send(i)) {
				co_await isix::co::sleep_ms(1);
			}
			const auto v = co_await to_a.recv(isix::ms2tick(500));
			if (!v || *v != i + 100) {
				bad = bad + 1;
				co_return;
			}
			a_sum = a_sum + *v;
		}
	};
	isix::co::run(sched_a, main_coro());
	const auto t0 = isix::get_jiffies();
	while (!b_done && !isix::timer_elapsed(t0, isix::ms2tick(1000))) {
		isix::wait_ms(1);
	}
	TEST_ASSERT_TRUE(b_done);
	TEST_ASSERT_EQUAL(0, bad);
	TEST_ASSERT_EQUAL(190, b_sum);
	TEST_ASSERT_EQUAL(190 + 100 * c_items, a_sum);
	isix::wait_ms(10);
}

#if CONFIG_ISIX_FIFO_EVENT_NOTIFY

TEST(coroutines, fifo_reader_preloaded_and_dontwait)
{
	isix::co::scheduler sched;
	isix::fifo<int> fifo(4);
	TEST_ASSERT_TRUE(fifo.is_valid());
	isix::co::fifo_reader<int> rd(sched, fifo);
	TEST_ASSERT_TRUE(rd.is_valid());
	TEST_ASSERT_EQUAL(ISIX_EOK, fifo.push(11, ISIX_TIME_INFINITE));
	int first { -1 };
	bool second_empty { false };
	auto coro = [&]() -> isix::co::task<void> {
		const auto a = co_await rd.read(ISIX_TIME_DONTWAIT);
		first = a ? *a : -1;
		const auto b = co_await rd.read(ISIX_TIME_DONTWAIT);
		second_empty = !b.has_value();
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_EQUAL(11, first);
	TEST_ASSERT_TRUE(second_empty);
}

TEST(coroutines, fifo_reader_timeout)
{
	isix::co::scheduler sched;
	isix::fifo<int> fifo(4);
	isix::co::fifo_reader<int> rd(sched, fifo);
	bool timed_out { false };
	auto coro = [&]() -> isix::co::task<void> {
		const auto v = co_await rd.read(isix::ms2tick(30));
		timed_out = !v.has_value();
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_TRUE(timed_out);
}

TEST(coroutines, fifo_reader_thread_writer_burst)
{
	constexpr int c_items = 60;
	isix::co::scheduler sched;
	isix::fifo<int> fifo(8);
	isix::co::fifo_reader<int> rd(sched, fifo);
	int received { 0 };
	int out_of_order { 0 };
	bool timed_out { false };

	auto writer = isix::thread_create([&] {
		for (int i = 0; i < c_items; ++i) {
			fifo.push(i, ISIX_TIME_INFINITE);
		}
	});
	writer.start_thread(c_stack_size, c_task_prio);

	auto consumer = [&]() -> isix::co::task<void> {
		while (received < c_items) {
			const auto v = co_await rd.read(isix::ms2tick(500));
			if (!v) {
				timed_out = true;
				co_return;
			}
			if (*v != received) {
				++out_of_order;
			}
			++received;
		}
	};
	isix::co::run(sched, consumer());
	TEST_ASSERT_FALSE(timed_out);
	TEST_ASSERT_EQUAL(c_items, received);
	TEST_ASSERT_EQUAL(0, out_of_order);
	isix::wait_ms(10);
}

TEST(coroutines, fifo_reader_isr_writer)
{
	constexpr int c_items = 20;
	isix::co::scheduler sched;
	isix::fifo<int> fifo(8);
	isix::co::fifo_reader<int> rd(sched, fifo);
	volatile int next_item { 0 };
	int received { 0 };
	int out_of_order { 0 };
	bool timed_out { false };
	auto consumer = [&]() -> isix::co::task<void> {
		while (received < c_items) {
			const auto v = co_await rd.read(isix::ms2tick(200));
			if (!v) {
				timed_out = true;
				co_return;
			}
			if (*v != received) {
				++out_of_order;
			}
			++received;
		}
	};
	const bool started = tests::detail::periodic_timer_setup([&] {
		const int n = next_item;
		if (n < c_items) {
			if (fifo.push_isr(n) == ISIX_EOK) {
				next_item = n + 1;
			}
		} else {
			tests::detail::periodic_timer_stop();
		}
	}, 1000);
	TEST_ASSERT_TRUE(started);
	isix::co::run(sched, consumer());
	tests::detail::periodic_timer_stop();
	isix::wait_ms(10);
	TEST_ASSERT_FALSE(timed_out);
	TEST_ASSERT_EQUAL(c_items, received);
	TEST_ASSERT_EQUAL(0, out_of_order);
}

TEST(coroutines, fifo_reader_destroy_wakes_waiter)
{
	isix::co::scheduler sched;
	isix::fifo<int> fifo(4);
	int rc { ISIX_ESTATE };
	bool got { true };
	auto coro = [&](isix::co::fifo_reader<int>& rd) -> isix::co::task<void> {
		const auto v = co_await rd.read();
		got = v.has_value();
		rc = v ? ISIX_EOK : ISIX_EDESTROY;
	};
	{
		std::optional<isix::co::fifo_reader<int>> rd;
		rd.emplace(sched, fifo);
		auto t = coro(*rd);
		isix::co::spawn(sched, t);
		sched.step();
		rd.reset();
		sched.step();
	}
	TEST_ASSERT_FALSE(got);
	TEST_ASSERT_EQUAL(ISIX_EDESTROY, rc);
	// The fifo can be connected again after the reader is gone
	isix::co::fifo_reader<int> again(sched, fifo);
	TEST_ASSERT_TRUE(again.is_valid());
}

#endif // CONFIG_ISIX_FIFO_EVENT_NOTIFY

#if CONFIG_ISIX_SEM_EVENT_NOTIFY

TEST(coroutines, sem_adapter_thread_signal)
{
	constexpr int c_tokens = 40;
	isix::co::scheduler sched;
	ossem_t sem = isix_sem_create(nullptr, 0);
	TEST_ASSERT_NOT_NULL(sem);
	{
		isix::co::sem_adapter ad(sched, sem);
		TEST_ASSERT_TRUE(ad.is_valid());
		int taken { 0 };
		bool timed_out { false };
		auto signaller = isix::thread_create([&] {
			for (int i = 0; i < c_tokens; ++i) {
				isix_sem_signal(sem);
			}
		});
		signaller.start_thread(c_stack_size, c_task_prio);
		auto consumer = [&]() -> isix::co::task<void> {
			while (taken < c_tokens) {
				if (co_await ad.take(isix::ms2tick(500)) != ISIX_EOK) {
					timed_out = true;
					co_return;
				}
				++taken;
			}
		};
		isix::co::run(sched, consumer());
		TEST_ASSERT_FALSE(timed_out);
		TEST_ASSERT_EQUAL(c_tokens, taken);
		isix::wait_ms(10);
	}
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(sem));
}

TEST(coroutines, sem_adapter_isr_signal)
{
	constexpr int c_tokens = 15;
	isix::co::scheduler sched;
	ossem_t sem = isix_sem_create(nullptr, 0);
	TEST_ASSERT_NOT_NULL(sem);
	{
		isix::co::sem_adapter ad(sched, sem);
		volatile int sent { 0 };
		int taken { 0 };
		bool timed_out { false };
		auto consumer = [&]() -> isix::co::task<void> {
			while (taken < c_tokens) {
				if (co_await ad.take(isix::ms2tick(200)) != ISIX_EOK) {
					timed_out = true;
					co_return;
				}
				++taken;
			}
		};
		const bool started = tests::detail::periodic_timer_setup([&] {
			const int n = sent;
			if (n < c_tokens) {
				isix_sem_signal_isr(sem);
				sent = n + 1;
			} else {
				tests::detail::periodic_timer_stop();
			}
		}, 1000);
		TEST_ASSERT_TRUE(started);
		isix::co::run(sched, consumer());
		tests::detail::periodic_timer_stop();
		isix::wait_ms(10);
		TEST_ASSERT_FALSE(timed_out);
		TEST_ASSERT_EQUAL(c_tokens, taken);
	}
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(sem));
}

TEST(coroutines, sem_adapter_timeout_dontwait_and_preloaded)
{
	isix::co::scheduler sched;
	ossem_t sem = isix_sem_create(nullptr, 1);
	TEST_ASSERT_NOT_NULL(sem);
	{
		isix::co::sem_adapter ad(sched, sem);
		int rc_pre { ISIX_ESTATE };
		int rc_dontwait { ISIX_ESTATE };
		int rc_timeout { ISIX_ESTATE };
		auto coro = [&]() -> isix::co::task<void> {
			rc_pre = co_await ad.take(ISIX_TIME_DONTWAIT);
			rc_dontwait = co_await ad.take(ISIX_TIME_DONTWAIT);
			rc_timeout = co_await ad.take(isix::ms2tick(30));
		};
		isix::co::run(sched, coro());
		TEST_ASSERT_EQUAL(ISIX_EOK, rc_pre);
		TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc_dontwait);
		TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc_timeout);
	}
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(sem));
}

TEST(coroutines, sem_adapter_task_waiter_has_priority_and_busy_connect)
{
	isix::co::scheduler sched;
	ossem_t sem = isix_sem_create(nullptr, 0);
	TEST_ASSERT_NOT_NULL(sem);
	{
		isix::co::sem_adapter ad(sched, sem);
		TEST_ASSERT_TRUE(ad.is_valid());
		isix::co::sem_adapter second(sched, sem);
		TEST_ASSERT_FALSE(second.is_valid());
		// Unconnected adapter reports an invalid argument
		int rc { ISIX_ESTATE };
		auto coro = [&]() -> isix::co::task<void> { rc = co_await second.take(ISIX_TIME_DONTWAIT); };
		isix::co::run(sched, coro());
		TEST_ASSERT_EQUAL(ISIX_EINVARG, rc);
	}
	// Disconnect on destruction: the semaphore can be connected again
	isix::co::sem_adapter again(sched, sem);
	TEST_ASSERT_TRUE(again.is_valid());
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_signal(sem));
	TEST_ASSERT_TRUE(again.try_take());
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(sem));
}

TEST(coroutines, sem_adapter_destroy_wakes_waiter)
{
	isix::co::scheduler sched;
	ossem_t sem = isix_sem_create(nullptr, 0);
	TEST_ASSERT_NOT_NULL(sem);
	int rc { ISIX_ESTATE };
	{
		std::optional<isix::co::sem_adapter> ad;
		ad.emplace(sched, sem);
		auto coro = [&](isix::co::sem_adapter& a) -> isix::co::task<void> { rc = co_await a.take(); };
		auto t = coro(*ad);
		isix::co::spawn(sched, t);
		sched.step();
		ad.reset();
		sched.step();
	}
	TEST_ASSERT_EQUAL(ISIX_EDESTROY, rc);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(sem));
}

#endif // CONFIG_ISIX_SEM_EVENT_NOTIFY

#ifdef CORO_UT_EXCEPTIONS

TEST(coroutines, exception_propagates_from_body)
{
	try {
		auto coro = []() -> isix::co::task<void> { throw std::runtime_error("coro-body"); };
		isix::co::run(coro());
		TEST_FAIL_MESSAGE("expected exception");
	} catch (const std::runtime_error& e) {
		TEST_ASSERT_EQUAL_STRING("coro-body", e.what());
	}
}

TEST(coroutines, exception_after_await)
{
	try {
		auto coro = []() -> isix::co::task<void> {
			co_await isix::co::sleep_ms(2);
			throw std::runtime_error("after-await");
		};
		isix::co::run(coro());
		TEST_FAIL_MESSAGE("expected exception");
	} catch (const std::runtime_error& e) {
		TEST_ASSERT_EQUAL_STRING("after-await", e.what());
	}
}

#endif // CORO_UT_EXCEPTIONS

TEST_GROUP_RUNNER(coroutines)
{
	RUN_TEST_CASE(coroutines, returns_value);
	RUN_TEST_CASE(coroutines, event_cross_task);
	RUN_TEST_CASE(coroutines, sleep_sequence);
	RUN_TEST_CASE(coroutines, many_coroutines_interleave_on_one_thread);
	RUN_TEST_CASE(coroutines, run_returns_when_other_coroutine_still_waiting);
	RUN_TEST_CASE(coroutines, sleep_already_due_does_not_hang);
	RUN_TEST_CASE(coroutines, scheduler_teardown_with_armed_timer);
	RUN_TEST_CASE(coroutines, many_schedulers_sequential_no_leak);
	RUN_TEST_CASE(coroutines, event_set_from_isr);
	RUN_TEST_CASE(coroutines, event_set_before_wait_latched);
	RUN_TEST_CASE(coroutines, event_wait_for_timeout);
	RUN_TEST_CASE(coroutines, event_wait_for_dontwait);
	RUN_TEST_CASE(coroutines, event_timeout_then_set_no_double_resume);
	RUN_TEST_CASE(coroutines, event_wake_all_waiters);
	RUN_TEST_CASE(coroutines, task_destroy_while_waiting);
	RUN_TEST_CASE(coroutines, semaphore_and_mutex_coroutine_native);
	RUN_TEST_CASE(coroutines, semaphore_burst_from_thread);
	RUN_TEST_CASE(coroutines, semaphore_signal_from_isr);
	RUN_TEST_CASE(coroutines, semaphore_take_timeout);
	RUN_TEST_CASE(coroutines, semaphore_limit);
	RUN_TEST_CASE(coroutines, mutex_contention_fifo);
	RUN_TEST_CASE(coroutines, mutex_lock_timeout);
	RUN_TEST_CASE(coroutines, mutex_scoped_lock);
	RUN_TEST_CASE(coroutines, channel_send_recv);
	RUN_TEST_CASE(coroutines, channel_thread_producer_order);
	RUN_TEST_CASE(coroutines, channel_isr_producer);
	RUN_TEST_CASE(coroutines, channel_backpressure_coroutines);
	RUN_TEST_CASE(coroutines, channel_recv_timeout);
	RUN_TEST_CASE(coroutines, channel_send_timeout_full);
	RUN_TEST_CASE(coroutines, await_nested_task_value);
	RUN_TEST_CASE(coroutines, await_nested_void_and_event);
	RUN_TEST_CASE(coroutines, await_nested_destroy_parent_while_waiting);
	RUN_TEST_CASE(coroutines, two_schedulers_two_threads);
#if CONFIG_ISIX_FIFO_EVENT_NOTIFY
	RUN_TEST_CASE(coroutines, fifo_reader_preloaded_and_dontwait);
	RUN_TEST_CASE(coroutines, fifo_reader_timeout);
	RUN_TEST_CASE(coroutines, fifo_reader_thread_writer_burst);
	RUN_TEST_CASE(coroutines, fifo_reader_isr_writer);
	RUN_TEST_CASE(coroutines, fifo_reader_destroy_wakes_waiter);
#endif
#if CONFIG_ISIX_SEM_EVENT_NOTIFY
	RUN_TEST_CASE(coroutines, sem_adapter_thread_signal);
	RUN_TEST_CASE(coroutines, sem_adapter_isr_signal);
	RUN_TEST_CASE(coroutines, sem_adapter_timeout_dontwait_and_preloaded);
	RUN_TEST_CASE(coroutines, sem_adapter_task_waiter_has_priority_and_busy_connect);
	RUN_TEST_CASE(coroutines, sem_adapter_destroy_wakes_waiter);
#endif
#ifdef CORO_UT_EXCEPTIONS
	RUN_TEST_CASE(coroutines, exception_propagates_from_body);
	RUN_TEST_CASE(coroutines, exception_after_await);
#endif
}

#else // !CONFIG_ISIX_CPP_COROUTINES

TEST_GROUP_RUNNER(coroutines) {}

#endif
