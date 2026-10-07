/*
 * Extended coroutine tests: scale, allocation, cancellation, composition.
 *
 * Author: Lucjan Bryndza
 */

/* Enable isix::co via isix.h only in this translation unit (keeps other tests lean). */
#define CONFIG_ISIX_CPP_COROUTINES 1

#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include "timer_interrupt.hpp"
#include "utils/tickless_testhooks.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <memory>
#include <optional>

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

//! Heap in use after pending deferred task cleanup has finished
size_t heap_used()
{
	isix::wait_ms(30);
	isix::memory_stat st {};
	isix::heap_stats(st);
	return st.used;
}

//! Wait for deferred cleanup (zombie tasks, timer worker) until the heap is back at the baseline
bool heap_back_to(size_t baseline)
{
	for (int n = 0; n < 50; ++n) {
		if (heap_used() <= baseline) {
			return true;
		}
	}
	return false;
}

//! Frame allocator that counts requests and optionally refuses them
struct counting_allocator {
	static inline unsigned allocs {};
	static inline unsigned frees {};
	static inline bool refuse {};

	static void* alloc(void*, std::size_t size) noexcept
	{
		++allocs;
		return refuse ? nullptr : isix_alloc(size);
	}
	static void release(void*, void* ptr, std::size_t) noexcept
	{
		++frees;
		isix_free(ptr);
	}
	static void install(bool refuse_all) noexcept
	{
		allocs = frees = 0;
		refuse = refuse_all;
		isix::co::set_frame_allocator({ &alloc, &release, nullptr });
	}
};

//! Awaiter whose timeout and cancellation are reported through on_abort instead of a wake-up
class abortable_wait final {
public:
	static inline isix::co::detail::wait_node* node {};
	static inline int abort_rc { ISIX_EOK };
	static inline unsigned abort_calls {};

	explicit abortable_wait(ostick_t timeout) noexcept : m_timeout(timeout) {}

	bool await_ready() const noexcept { return false; }

	template<typename P>
	bool await_suspend(std::coroutine_handle<P> h) noexcept
	{
		m_node.on_abort = [](isix::co::detail::wait_node& n, int rc) {
			node = &n;
			abort_rc = rc;
			++abort_calls;
		};
		h.promise().sched->suspend(m_node, h, m_timeout);
		return true;
	}

	int await_resume() const noexcept { return m_node.result; }

private:
	ostick_t m_timeout {};
	isix::co::detail::wait_node m_node {};
};

//! Awaiter whose abort handler signals an event while the scheduler walks its timer list
class signalling_abort_wait final {
public:
	static inline isix::co::event* target {};
	static inline isix::co::detail::wait_node* node {};
	static inline unsigned abort_calls {};

	explicit signalling_abort_wait(ostick_t timeout) noexcept : m_timeout(timeout) {}

	bool await_ready() const noexcept { return false; }

	template<typename P>
	bool await_suspend(std::coroutine_handle<P> h) noexcept
	{
		m_node.on_abort = [](isix::co::detail::wait_node& n, int) {
			node = &n;
			++abort_calls;
			target->set(ISIX_EOK);
		};
		h.promise().sched->suspend(m_node, h, m_timeout);
		return true;
	}

	int await_resume() const noexcept { return m_node.result; }

private:
	ostick_t m_timeout {};
	isix::co::detail::wait_node m_node {};
};

//! A signal sent by another thread while the scheduler thread resumes a coroutine must not be lost
void signal_during_resume(osprio_t helper_prio, bool block_scheduler)
{
	constexpr int c_rounds = 50;
	isix::co::scheduler_thread st;
	TEST_ASSERT_EQUAL(ISIX_EOK, st.start(c_stack_size, 4));
	isix::co::event ev(st.sched());
	isix::co::event ack(st.sched());
	isix::semaphore kick(0);
	volatile int ok { 0 };
	volatile int timeouts { 0 };
	volatile int ack_failed { 0 };
	volatile bool w_done { false };
	volatile bool k_done { false };
	volatile bool h_done { false };
	auto helper = isix::thread_create([&] {
		for (int n = 0; n < c_rounds; ++n) {
			if (kick.wait(isix::ms2tick(1000)) != ISIX_EOK) {
				break;
			}
			ev.set();
		}
		h_done = true;
	});
	helper.start_thread(c_stack_size, helper_prio);
	auto waiter = [&]() -> isix::co::task<void> {
		for (int n = 0; n < c_rounds; ++n) {
			if (co_await ev.wait_for(isix::ms2tick(200)) == ISIX_EOK) {
				ok = ok + 1;
			} else {
				timeouts = timeouts + 1;
			}
			ack.set();
		}
		w_done = true;
	};
	auto kicker = [&]() -> isix::co::task<void> {
		for (int n = 0; n < c_rounds; ++n) {
			kick.signal();
			if (block_scheduler) {
				// Lets a lower priority thread run while the coroutine is being resumed
				isix::wait_ms(2);
			}
			if (co_await ack.wait_for(isix::ms2tick(500)) != ISIX_EOK) {
				ack_failed = ack_failed + 1;
			}
		}
		k_done = true;
	};
	TEST_ASSERT_EQUAL(ISIX_EOK, st.post(waiter()));
	TEST_ASSERT_EQUAL(ISIX_EOK, st.post(kicker()));
	const auto t0 = isix::get_jiffies();
	while (!(w_done && k_done && h_done) && !isix::timer_elapsed(t0, isix::ms2tick(8000))) {
		isix::wait_ms(5);
	}
	helper.wait_for();
	TEST_ASSERT_TRUE(w_done && k_done && h_done);
	TEST_ASSERT_EQUAL(c_rounds, ok);
	TEST_ASSERT_EQUAL(0, timeouts);
	TEST_ASSERT_EQUAL(0, ack_failed);
}

} // namespace

TEST_GROUP(coroutines_ext);
TEST_SETUP(coroutines_ext)
{
	// The first armed timer creates the vtimer worker, keep it out of heap comparisons
	static bool warmed { false };
	if (!warmed) {
		auto warm = []() -> isix::co::task<void> { co_await isix::co::sleep_ms(20); };
		isix::co::run(warm());
		warmed = true;
	}
}
TEST_TEAR_DOWN(coroutines_ext)
{
#if CONFIG_ISIX_CO_TEST_SHUFFLE
	isix::co::test::enable(false);
#endif
	tests::detail::periodic_timer_stop();
	isix::co::reset_frame_allocator();
}

TEST(coroutines_ext, hundred_events_on_one_scheduler)
{
	constexpr int c_count = 100;
	const auto heap_before = heap_used();
	{
		isix::co::scheduler sched;
		std::array<std::optional<isix::co::event>, c_count> events {};
		std::array<int, c_count> rc {};
		int done { 0 };
		bool all_valid { true };
		for (auto& ev : events) {
			ev.emplace(sched);
			all_valid = all_valid && ev->is_valid();
		}
		auto waiter = [&](int idx) -> isix::co::task<void> {
			rc[idx] = co_await events[idx]->wait_for(isix::ms2tick(1000));
			++done;
		};
		std::array<isix::co::task<void>, c_count> tasks {};
		for (int n = 0; n < c_count; ++n) {
			tasks[n] = waiter(n);
			isix::co::spawn(sched, tasks[n]);
		}
		auto thr = isix::thread_create([&events] {
			isix::wait_ms(20);
			for (auto& ev : events) {
				ev->set();
			}
		});
		thr.start_thread(c_stack_size, c_task_prio);
		const bool finished = drive(sched, [&done] { return done == c_count; });
		int ok { 0 };
		for (const auto v : rc) {
			ok += v == ISIX_EOK ? 1 : 0;
		}
		isix::wait_ms(30);
		TEST_ASSERT_TRUE(all_valid);
		TEST_ASSERT_TRUE(finished);
		TEST_ASSERT_EQUAL(c_count, ok);
	}
	TEST_ASSERT_TRUE(heap_back_to(heap_before));
}

TEST(coroutines_ext, frame_storage_runs_without_heap)
{
	counting_allocator::install(true);
	isix::co::frame_storage<256> storage;
	auto coro = [](isix::co::frame_storage<256>&) -> isix::co::task<int> {
		co_await isix::co::sleep_ms(5);
		co_return 5;
	};
	const auto heap_before = heap_used();
	int v {};
	{
		isix::co::scheduler sched;
		v = isix::co::run(sched, coro(storage));
	}
	TEST_ASSERT_EQUAL(5, v);
	TEST_ASSERT_EQUAL_UINT32(0, counting_allocator::allocs);
	TEST_ASSERT_FALSE(storage.in_use());
	TEST_ASSERT_TRUE(heap_back_to(heap_before));
}

TEST(coroutines_ext, frame_storage_too_small_reports_enomem)
{
	isix::co::scheduler sched;
	isix::co::frame_storage<32> storage;
	auto coro = [](isix::co::frame_storage<32>&) -> isix::co::task<void> { co_return; };
	auto t = coro(storage);
	TEST_ASSERT_FALSE(t.valid());
	TEST_ASSERT_EQUAL(ISIX_ENOMEM, isix::co::spawn(sched, t));
	TEST_ASSERT_FALSE(storage.in_use());
}

TEST(coroutines_ext, frame_pool_exhaustion_and_reuse)
{
	isix::co::scheduler sched;
	isix::co::frame_pool<128, 2> pool;
	auto coro = [](isix::co::frame_pool<128, 2>&) -> isix::co::task<int> { co_return 9; };
	auto a = coro(pool);
	auto b = coro(pool);
	auto c = coro(pool);
	TEST_ASSERT_TRUE(a.valid());
	TEST_ASSERT_TRUE(b.valid());
	TEST_ASSERT_FALSE(c.valid());
	TEST_ASSERT_EQUAL_UINT32(2, pool.used());
	TEST_ASSERT_EQUAL(ISIX_ENOMEM, isix::co::spawn(sched, c));
	TEST_ASSERT_EQUAL(9, isix::co::run(sched, std::move(a)));
	TEST_ASSERT_EQUAL_UINT32(1, pool.used());
	auto d = coro(pool);
	TEST_ASSERT_TRUE(d.valid());
	TEST_ASSERT_EQUAL(9, isix::co::run(sched, std::move(d)));
	b.destroy();
	TEST_ASSERT_EQUAL_UINT32(0, pool.used());
}

TEST(coroutines_ext, global_allocator_failure_is_not_fatal)
{
	isix::co::scheduler sched;
	counting_allocator::install(true);
	auto coro = []() -> isix::co::task<int> { co_return 1; };
	auto t = coro();
	TEST_ASSERT_FALSE(t.valid());
	TEST_ASSERT_EQUAL(ISIX_ENOMEM, isix::co::spawn(sched, t));
	TEST_ASSERT_EQUAL_UINT32(1, counting_allocator::allocs);
	counting_allocator::install(false);
	auto ok = coro();
	TEST_ASSERT_TRUE(ok.valid());
	TEST_ASSERT_EQUAL(1, isix::co::run(sched, std::move(ok)));
	TEST_ASSERT_EQUAL_UINT32(1, counting_allocator::allocs);
	TEST_ASSERT_EQUAL_UINT32(1, counting_allocator::frees);
}

TEST(coroutines_ext, cancel_wakes_event_wait_with_ecanceled)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc { ISIX_ESTATE };
	bool done { false };
	auto coro = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(1000));
		done = true;
	};
	auto t = coro();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	TEST_ASSERT_FALSE(done);
	const auto u0 = isix::get_ujiffies();
	t.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_UINT_WITHIN(50ULL * 1000ULL, 0ULL, isix::get_ujiffies() - u0);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc);
}

TEST(coroutines_ext, cancel_propagates_to_nested_leaf)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int leaf_rc { ISIX_ESTATE };
	bool mid_cancelled { false };
	bool outer_cancelled { false };
	bool done { false };
	auto leaf = [&]() -> isix::co::task<void> {
		leaf_rc = co_await ev.wait_for(isix::ms2tick(1000));
	};
	auto mid = [&]() -> isix::co::task<void> {
		co_await leaf();
		mid_cancelled = co_await isix::co::cancelled();
	};
	auto outer = [&]() -> isix::co::task<void> {
		co_await mid();
		outer_cancelled = co_await isix::co::cancelled();
		done = true;
	};
	auto t = outer();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	sched.step();
	TEST_ASSERT_FALSE(done);
	const auto u0 = isix::get_ujiffies();
	t.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_UINT_WITHIN(50ULL * 1000ULL, 0ULL, isix::get_ujiffies() - u0);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, leaf_rc);
	TEST_ASSERT_TRUE(mid_cancelled);
	TEST_ASSERT_TRUE(outer_cancelled);
}

TEST(coroutines_ext, cancel_before_start)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc { ISIX_ESTATE };
	auto coro = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(1000));
	};
	auto t = coro();
	t.cancel();
	const auto u0 = isix::get_ujiffies();
	isix::co::run(sched, std::move(t));
	TEST_ASSERT_UINT_WITHIN(50ULL * 1000ULL, 0ULL, isix::get_ujiffies() - u0);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc);
}

TEST(coroutines_ext, cancelled_task_next_await_returns_immediately)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc { ISIX_ESTATE };
	osutick_t after_wait {};
	osutick_t after_sleep {};
	bool done { false };
	auto coro = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(1000));
		after_wait = isix::get_ujiffies();
		co_await isix::co::sleep_ms(500);
		after_sleep = isix::get_ujiffies();
		done = true;
	};
	auto t = coro();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	t.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc);
	TEST_ASSERT_UINT_WITHIN(50ULL * 1000ULL, 0ULL, after_sleep - after_wait);
}

TEST(coroutines_ext, cancel_while_in_ready_queue)
{
	isix::co::scheduler sched;
	isix::co::event ev_b(sched);
	isix::co::event ev_a(sched);
	isix::co::event ev_a2(sched);
	int first_rc { ISIX_ESTATE };
	int second_rc { ISIX_ESTATE };
	bool a_done { false };
	bool b_done { false };
	isix::co::task<void> ta;
	auto a = [&]() -> isix::co::task<void> {
		first_rc = co_await ev_a.wait_for(isix::ms2tick(1000));
		second_rc = co_await ev_a2.wait_for(isix::ms2tick(1000));
		a_done = true;
	};
	auto b = [&]() -> isix::co::task<void> {
		co_await ev_b.wait_for(isix::ms2tick(1000));
		ta.cancel();
		b_done = true;
	};
	ta = a();
	auto tb = b();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, tb));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, ta));
	sched.step();
	// B is woken before A, so it cancels A while A sits on the ready queue
	ev_b.set();
	ev_a.set();
	TEST_ASSERT_TRUE(drive(sched, [&] { return a_done && b_done; }));
	TEST_ASSERT_EQUAL(ISIX_EOK, first_rc);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, second_rc);
}

TEST(coroutines_ext, cancel_releases_scoped_mutex)
{
	isix::co::scheduler sched;
	isix::co::mutex mtx(sched);
	TEST_ASSERT_TRUE(mtx.try_lock());
	int rc { ISIX_ESTATE };
	bool owns { true };
	bool done { false };
	auto coro = [&]() -> isix::co::task<void> {
		auto lk = co_await mtx.scoped();
		rc = lk.error();
		owns = lk.owns_lock();
		done = true;
	};
	auto t = coro();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	t.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc);
	TEST_ASSERT_FALSE(owns);
	TEST_ASSERT_TRUE(mtx.is_locked());
	TEST_ASSERT_EQUAL(ISIX_EOK, mtx.unlock());
	TEST_ASSERT_FALSE(mtx.is_locked());
}

TEST(coroutines_ext, stop_source_from_thread)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	isix::co::stop_source stop(sched);
	int rc { ISIX_ESTATE };
	bool done { false };
	auto coro = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(1000));
		done = true;
	};
	auto t = coro();
	stop.bind(t);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	auto thr = isix::thread_create([&stop] {
		isix::wait_ms(10);
		stop.request_stop();
	});
	thr.start_thread(c_stack_size, c_task_prio);
	const auto u0 = isix::get_ujiffies();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_UINT_WITHIN(60ULL * 1000ULL, 30ULL * 1000ULL, isix::get_ujiffies() - u0);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc);
	stop.unbind();
	isix::wait_ms(20);
}

TEST(coroutines_ext, stop_source_from_isr)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	isix::co::stop_source stop(sched);
	int rc { ISIX_ESTATE };
	bool done { false };
	volatile int isr_count { 0 };
	auto coro = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(500));
		done = true;
	};
	auto t = coro();
	stop.bind(t);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	const bool started = tests::detail::periodic_timer_setup([&] {
		isr_count = isr_count + 1;
		if (isr_count == 5) {
			stop.request_stop_isr();
		} else if (isr_count > 5) {
			tests::detail::periodic_timer_stop();
		}
	}, 1000);
	TEST_ASSERT_TRUE(started);
	const auto u0 = isix::get_ujiffies();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	tests::detail::periodic_timer_stop();
	TEST_ASSERT_UINT_WITHIN(50ULL * 1000ULL, 0ULL, isix::get_ujiffies() - u0);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc);
	stop.unbind();
	isix::wait_ms(10);
}

TEST(coroutines_ext, on_abort_replaces_timeout_wakeup)
{
	isix::co::scheduler sched;
	abortable_wait::node = nullptr;
	abortable_wait::abort_calls = 0;
	int rc { ISIX_ESTATE };
	bool done { false };
	auto coro = [&]() -> isix::co::task<void> {
		rc = co_await abortable_wait(isix::ms2tick(20));
		done = true;
	};
	auto t = coro();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	TEST_ASSERT_TRUE(drive(sched, [] { return abortable_wait::abort_calls > 0; }));
	TEST_ASSERT_FALSE(done);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, abortable_wait::abort_rc);
	TEST_ASSERT_NOT_NULL(abortable_wait::node);
	sched.wake(*abortable_wait::node, ISIX_ETIMEOUT);
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc);
}

TEST(coroutines_ext, on_abort_receives_cancellation)
{
	isix::co::scheduler sched;
	abortable_wait::node = nullptr;
	abortable_wait::abort_calls = 0;
	int rc { ISIX_ESTATE };
	bool done { false };
	auto coro = [&]() -> isix::co::task<void> {
		rc = co_await abortable_wait(isix::ms2tick(1000));
		done = true;
	};
	auto t = coro();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	t.cancel();
	TEST_ASSERT_EQUAL_UINT32(1, abortable_wait::abort_calls);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, abortable_wait::abort_rc);
	TEST_ASSERT_FALSE(done);
	sched.wake(*abortable_wait::node, ISIX_ECANCELED);
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc);
}

TEST(coroutines_ext, mutex_unlock_by_non_owner_rejected)
{
	isix::co::scheduler sched;
	isix::co::mutex mtx(sched);
	isix::co::event ev(sched);
	int stranger_rc { ISIX_ESTATE };
	int owner_rc { ISIX_ESTATE };
	bool owner_locked { false };
	bool done { false };
	auto owner = [&]() -> isix::co::task<void> {
		owner_locked = co_await mtx.lock() == ISIX_EOK;
		co_await ev.wait_for(isix::ms2tick(1000));
		owner_rc = mtx.unlock();
		done = true;
	};
	auto stranger = [&]() -> isix::co::task<void> {
		stranger_rc = mtx.unlock();
		co_return;
	};
	auto t1 = owner();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t1));
	sched.step();
	TEST_ASSERT_TRUE(owner_locked);
	isix::co::run(sched, stranger());
	TEST_ASSERT_EQUAL(ISIX_EPERM, stranger_rc);
	TEST_ASSERT_TRUE(mtx.is_locked());
	ev.set();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_EQUAL(ISIX_EOK, owner_rc);
	TEST_ASSERT_FALSE(mtx.is_locked());
}

#if CONFIG_ISIX_FIFO_EVENT_NOTIFY
TEST(coroutines_ext, fifo_reader_second_consumer_does_not_block)
{
	constexpr int c_items = 40;
	constexpr int c_thief_items = 10;
	isix::co::scheduler sched;
	isix::fifo<int> fifo(4);
	isix::co::fifo_reader<int> rd(sched, fifo);
	volatile int stolen { 0 };
	volatile bool thief_done { false };
	volatile bool writer_done { false };
	int received { 0 };
	bool timed_out { false };
	auto thief = isix::thread_create([&] {
		int v {};
		while (stolen < c_thief_items && fifo.pop(v, isix::ms2tick(2000)) == ISIX_EOK) {
			stolen = stolen + 1;
		}
		thief_done = true;
	});
	thief.start_thread(c_stack_size, c_task_prio);
	auto writer = isix::thread_create([&] {
		for (int i = 0; i < c_items; ++i) {
			static_cast<void>(fifo.push(i, ISIX_TIME_INFINITE));
		}
		writer_done = true;
	});
	writer.start_thread(c_stack_size, c_task_prio + 1);
	auto consumer = [&]() -> isix::co::task<void> {
		for (;;) {
			const auto v = co_await rd.read(isix::ms2tick(300));
			if (!v) {
				timed_out = true;
				co_return;
			}
			++received;
		}
	};
	isix::co::run(sched, consumer());
	const auto t0 = isix::get_jiffies();
	while (!(thief_done && writer_done) && !isix::timer_elapsed(t0, isix::ms2tick(2000))) {
		isix::wait_ms(10);
	}
	isix::wait_ms(30);
	TEST_ASSERT_TRUE(timed_out);
	TEST_ASSERT_TRUE(thief_done && writer_done);
	TEST_ASSERT_EQUAL(c_thief_items, stolen);
	TEST_ASSERT_EQUAL(c_items, received + stolen);
}
#endif

TEST(coroutines_ext, when_all_two_values)
{
	isix::co::scheduler sched;
	int sum { 0 };
	int err_a { ISIX_ESTATE };
	int err_b { ISIX_ESTATE };
	auto make = [](int v) -> isix::co::task<int> {
		co_await isix::co::sleep_ms(5);
		co_return v;
	};
	auto parent = [&]() -> isix::co::task<void> {
		auto a = make(3);
		auto b = make(4);
		co_await isix::co::when_all(a, b);
		sum = a.value() + b.value();
		err_a = a.error();
		err_b = b.error();
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(7, sum);
	TEST_ASSERT_EQUAL(ISIX_EOK, err_a);
	TEST_ASSERT_EQUAL(ISIX_EOK, err_b);
}

TEST(coroutines_ext, when_all_with_sleeps_runs_concurrently)
{
	isix::co::scheduler sched;
	osutick_t elapsed {};
	auto nap = []() -> isix::co::task<void> { co_await isix::co::sleep_ms(30); };
	auto parent = [&]() -> isix::co::task<void> {
		auto a = nap();
		auto b = nap();
		const auto u0 = isix::get_ujiffies();
		co_await isix::co::when_all(a, b);
		elapsed = isix::get_ujiffies() - u0;
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_UINT_WITHIN(25ULL * 1000ULL, 30ULL * 1000ULL, elapsed);
}

TEST(coroutines_ext, when_all_cancel_propagates_to_all)
{
	isix::co::scheduler sched;
	isix::co::event ev_a(sched);
	isix::co::event ev_b(sched);
	int rc_a { ISIX_ESTATE };
	int rc_b { ISIX_ESTATE };
	int err_a { ISIX_ESTATE };
	int err_b { ISIX_ESTATE };
	bool done { false };
	auto wait_on = [](isix::co::event& ev, int& rc) -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(1000));
	};
	auto parent = [&]() -> isix::co::task<void> {
		auto a = wait_on(ev_a, rc_a);
		auto b = wait_on(ev_b, rc_b);
		co_await isix::co::when_all(a, b);
		err_a = a.error();
		err_b = b.error();
		done = true;
	};
	auto t = parent();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	sched.step();
	const auto u0 = isix::get_ujiffies();
	t.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_UINT_WITHIN(50ULL * 1000ULL, 0ULL, isix::get_ujiffies() - u0);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc_a);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc_b);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, err_a);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, err_b);
}

TEST(coroutines_ext, when_any_returns_winner_index)
{
	isix::co::scheduler sched;
	std::size_t winner { 99 };
	int err_fast { ISIX_ESTATE };
	int err_slow { ISIX_ESTATE };
	osutick_t elapsed {};
	auto sleeper = [](unsigned ms) -> isix::co::task<void> { co_await isix::co::sleep_ms(ms); };
	auto parent = [&]() -> isix::co::task<void> {
		auto fast = sleeper(10);
		auto slow = sleeper(500);
		const auto u0 = isix::get_ujiffies();
		winner = co_await isix::co::when_any(fast, slow);
		elapsed = isix::get_ujiffies() - u0;
		err_fast = fast.error();
		err_slow = slow.error();
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL_UINT32(0, winner);
	TEST_ASSERT_UINT_WITHIN(40ULL * 1000ULL, 10ULL * 1000ULL, elapsed);
	TEST_ASSERT_EQUAL(ISIX_EOK, err_fast);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, err_slow);
}

TEST(coroutines_ext, when_any_losers_finish_before_resume)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	bool loser_finished { false };
	bool seen_at_resume { false };
	std::size_t winner { 99 };
	auto quick = []() -> isix::co::task<void> { co_await isix::co::sleep_ms(5); };
	auto loser = [&]() -> isix::co::task<void> {
		co_await ev.wait_for(isix::ms2tick(1000));
		co_await isix::co::sleep_ms(1);
		loser_finished = true;
	};
	auto parent = [&]() -> isix::co::task<void> {
		auto a = quick();
		auto b = loser();
		winner = co_await isix::co::when_any(a, b);
		seen_at_resume = loser_finished;
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL_UINT32(0, winner);
	TEST_ASSERT_TRUE(seen_at_resume);
}

TEST(coroutines_ext, when_any_event_vs_timeout_from_isr)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	std::size_t winner { 99 };
	int rc_event { ISIX_ESTATE };
	volatile int isr_count { 0 };
	auto wait_ev = [&]() -> isix::co::task<void> {
		rc_event = co_await ev.wait_for(isix::ms2tick(1000));
	};
	auto timeout = []() -> isix::co::task<void> { co_await isix::co::sleep_ms(300); };
	auto parent = [&]() -> isix::co::task<void> {
		auto a = wait_ev();
		auto b = timeout();
		winner = co_await isix::co::when_any(a, b);
	};
	const bool started = tests::detail::periodic_timer_setup([&] {
		isr_count = isr_count + 1;
		if (isr_count == 5) {
			ev.set_isr();
		} else if (isr_count > 5) {
			tests::detail::periodic_timer_stop();
		}
	}, 1000);
	TEST_ASSERT_TRUE(started);
	const auto u0 = isix::get_ujiffies();
	isix::co::run(sched, parent());
	tests::detail::periodic_timer_stop();
	TEST_ASSERT_UINT_WITHIN(100ULL * 1000ULL, 0ULL, isix::get_ujiffies() - u0);
	TEST_ASSERT_EQUAL_UINT32(0, winner);
	TEST_ASSERT_EQUAL(ISIX_EOK, rc_event);
	isix::wait_ms(10);
}

TEST(coroutines_ext, with_timeout_expires)
{
	isix::co::scheduler sched;
	int rc { ISIX_ESTATE };
	int child_err { ISIX_ESTATE };
	int child_finishes { 0 };
	osutick_t elapsed {};
	auto sleeper = [&]() -> isix::co::task<void> {
		co_await isix::co::sleep_ms(500);
		++child_finishes;
	};
	auto parent = [&]() -> isix::co::task<void> {
		auto child = sleeper();
		const auto u0 = isix::get_ujiffies();
		const auto r = co_await isix::co::with_timeout(child, isix::ms2tick(20));
		elapsed = isix::get_ujiffies() - u0;
		rc = r.has_value() ? ISIX_EOK : r.error();
		child_err = child.error();
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc);
	TEST_ASSERT_UINT_WITHIN(40ULL * 1000ULL, 20ULL * 1000ULL, elapsed);
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, child_err);
	TEST_ASSERT_EQUAL(1, child_finishes);
}

TEST(coroutines_ext, with_timeout_completes_in_time)
{
	isix::co::scheduler sched;
	int value { 0 };
	int rc { ISIX_ESTATE };
	auto compute = []() -> isix::co::task<int> {
		co_await isix::co::sleep_ms(5);
		co_return 21;
	};
	auto parent = [&]() -> isix::co::task<void> {
		const auto r = co_await isix::co::with_timeout(compute(), isix::ms2tick(500));
		rc = r.has_value() ? ISIX_EOK : r.error();
		value = r.value_or(0);
	};
	isix::co::run(sched, parent());
	TEST_ASSERT_EQUAL(ISIX_EOK, rc);
	TEST_ASSERT_EQUAL(21, value);
}

TEST(coroutines_ext, with_timeout_nested_in_when_any)
{
	isix::co::scheduler sched;
	int inner_rc { ISIX_ESTATE };
	std::size_t winner { 99 };
	auto slow = []() -> isix::co::task<void> { co_await isix::co::sleep_ms(500); };
	auto wrapped = [&]() -> isix::co::task<void> {
		const auto r = co_await isix::co::with_timeout(slow(), isix::ms2tick(20));
		inner_rc = r.has_value() ? ISIX_EOK : r.error();
	};
	auto parent = [&]() -> isix::co::task<void> {
		auto a = wrapped();
		auto b = slow();
		winner = co_await isix::co::when_any(a, b);
	};
	const auto u0 = isix::get_ujiffies();
	isix::co::run(sched, parent());
	TEST_ASSERT_UINT_WITHIN(60ULL * 1000ULL, 20ULL * 1000ULL, isix::get_ujiffies() - u0);
	TEST_ASSERT_EQUAL_UINT32(0, winner);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, inner_rc);
}

TEST(coroutines_ext, scope_spawn_and_join)
{
	constexpr int c_count = 10;
	int finished { 0 };
	bool join_ok { false };
	bool done { false };
	const auto heap_before = heap_used();
	{
		isix::co::scheduler sched;
		isix::co::async_scope scope(sched);
		auto worker = [&](unsigned ms) -> isix::co::task<void> {
			co_await isix::co::sleep_ms(ms);
			++finished;
		};
		auto parent = [&]() -> isix::co::task<void> {
			for (int n = 0; n < c_count; ++n) {
				if (scope.spawn(worker(2U + static_cast<unsigned>(n) * 2U)) != ISIX_EOK) {
					co_return;
				}
			}
			join_ok = co_await scope.join() == ISIX_EOK;
			done = true;
		};
		isix::co::run(sched, parent());
		TEST_ASSERT_TRUE(done);
		TEST_ASSERT_TRUE(join_ok);
		TEST_ASSERT_EQUAL(c_count, finished);
		TEST_ASSERT_EQUAL_UINT32(0, scope.size());
	}
	TEST_ASSERT_TRUE(heap_back_to(heap_before));
}

TEST(coroutines_ext, scope_cancel_all)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	isix::co::async_scope scope(sched);
	std::array<int, 3> rc {};
	rc.fill(ISIX_ESTATE);
	auto waiter = [&](std::size_t idx) -> isix::co::task<void> {
		rc[idx] = co_await ev.wait_for(isix::ms2tick(1000));
	};
	for (std::size_t n = 0; n < rc.size(); ++n) {
		TEST_ASSERT_EQUAL(ISIX_EOK, scope.spawn(waiter(n)));
	}
	sched.step();
	TEST_ASSERT_EQUAL_UINT32(3, scope.size());
	scope.cancel_all();
	const auto u0 = isix::get_ujiffies();
	TEST_ASSERT_TRUE(drive(sched, [&scope] { return scope.size() == 0; }));
	TEST_ASSERT_UINT_WITHIN(50ULL * 1000ULL, 0ULL, isix::get_ujiffies() - u0);
	for (const auto v : rc) {
		TEST_ASSERT_EQUAL(ISIX_ECANCELED, v);
	}
}

TEST(coroutines_ext, post_from_thread_runs_on_scheduler)
{
	constexpr int c_count = 20;
	isix::co::scheduler sched;
	std::array<ostask_t, c_count> ran_on {};
	int finished { 0 };
	int post_failures { 0 };
	auto body = [&](int idx) -> isix::co::task<void> {
		co_await isix::co::sleep_ms(1);
		ran_on[static_cast<std::size_t>(idx)] = isix_task_self();
		++finished;
	};
	auto thr = isix::thread_create([&] {
		for (int n = 0; n < c_count; ++n) {
			if (sched.post(body(n)) != ISIX_EOK) {
				++post_failures;
			}
		}
	});
	// Keeps the scheduler blocked in its event wait while the thread posts
	auto keeper = []() -> isix::co::task<void> { co_await isix::co::sleep_ms(1500); };
	auto tk = keeper();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, tk));
	thr.start_thread(c_stack_size, c_task_prio);
	const auto self = isix_task_self();
	const bool ok = drive(sched, [&finished] { return finished == c_count; });
	tk.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&tk] { return tk.done(); }));
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_EQUAL(0, post_failures);
	for (const auto t : ran_on) {
		TEST_ASSERT_TRUE(t == self);
	}
	TEST_ASSERT_EQUAL_UINT32(0, sched.detached_count());
	isix::wait_ms(10);
}

TEST(coroutines_ext, scope_destructor_cleans_up)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	auto waiter = [&]() -> isix::co::task<void> { co_await ev.wait_for(isix::ms2tick(1000)); };
	const auto heap_before = heap_used();
	{
		isix::co::async_scope scope(sched);
		for (int n = 0; n < 3; ++n) {
			TEST_ASSERT_EQUAL(ISIX_EOK, scope.spawn(waiter()));
		}
		sched.step();
		TEST_ASSERT_EQUAL_UINT32(3, scope.size());
	}
	TEST_ASSERT_TRUE(heap_back_to(heap_before));
	ev.set();
	sched.step();
}

TEST(coroutines_ext, scheduler_thread_two_priorities)
{
	isix::co::scheduler_thread high;
	isix::co::scheduler_thread low;
	TEST_ASSERT_EQUAL(ISIX_EOK, high.start(c_stack_size, 2));
	TEST_ASSERT_EQUAL(ISIX_EOK, low.start(c_stack_size, 5));
	isix::co::event ev_high(high.sched());
	isix::co::event ev_low(low.sched());
	volatile int order[2] { 0, 0 };
	volatile int next { 0 };
	auto waiter = [&](isix::co::event& ev, int id) -> isix::co::task<void> {
		co_await ev.wait_for(isix::ms2tick(1000));
		isix::enter_critical();
		order[next] = id;
		next = next + 1;
		isix::exit_critical();
	};
	TEST_ASSERT_EQUAL(ISIX_EOK, high.post(waiter(ev_high, 1)));
	TEST_ASSERT_EQUAL(ISIX_EOK, low.post(waiter(ev_low, 2)));
	isix::wait_ms(30);
	// Both are signaled before either executor can run, the higher priority one goes first
	isix::enter_critical();
	ev_low.set();
	ev_high.set();
	isix::exit_critical();
	isix::wait_ms(50);
	TEST_ASSERT_EQUAL(2, next);
	TEST_ASSERT_EQUAL(1, order[0]);
	TEST_ASSERT_EQUAL(2, order[1]);
	high.stop();
	low.stop();
}

TEST(coroutines_ext, signal_during_resume_from_higher_prio_thread)
{
	signal_during_resume(2, false);
}

TEST(coroutines_ext, signal_during_resume_from_lower_prio_thread)
{
	signal_during_resume(8, true);
}

TEST(coroutines_ext, event_set_from_scheduler_thread_wakes_directly)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc_outside { ISIX_ESTATE };
	int rc_inside { ISIX_ESTATE };
	auto waiter = [&](int& rc) -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(500));
	};
	auto setter = [&]() -> isix::co::task<void> {
		ev.set();
		co_return;
	};
	// Outside of a step the waiter becomes ready at once and runs in the next step
	auto w1 = waiter(rc_outside);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, w1));
	sched.step();
	TEST_ASSERT_FALSE(w1.done());
	ev.set();
	sched.step();
	TEST_ASSERT_TRUE(w1.done());
	TEST_ASSERT_EQUAL(ISIX_EOK, rc_outside);
	// Inside a coroutine the waiter runs in the step after the one that signalled
	auto w2 = waiter(rc_inside);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, w2));
	sched.step();
	auto s = setter();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, s));
	sched.step();
	TEST_ASSERT_TRUE(s.done());
	TEST_ASSERT_FALSE(w2.done());
	sched.step();
	TEST_ASSERT_TRUE(w2.done());
	TEST_ASSERT_EQUAL(ISIX_EOK, rc_inside);
	// Without a waiter the signal stays latched exactly once
	ev.set();
	int first { ISIX_ESTATE };
	int second { ISIX_ESTATE };
	auto probe = [&]() -> isix::co::task<void> {
		first = co_await ev.wait_for(isix::ms2tick(100));
		second = co_await ev.wait_for(ISIX_TIME_DONTWAIT);
	};
	isix::co::run(sched, probe());
	TEST_ASSERT_EQUAL(ISIX_EOK, first);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, second);
}

TEST(coroutines_ext, event_set_behind_queued_isr_signal_keeps_order)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc { ISIX_ESTATE };
	int leftover { ISIX_ESTATE };
	volatile bool fired { false };
	auto waiter = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(500));
		leftover = co_await ev.wait_for(ISIX_TIME_DONTWAIT);
	};
	auto w = waiter();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, w));
	sched.step();
	const bool started = tests::detail::periodic_timer_setup([&] {
		if (!fired) {
			fired = true;
			ev.set_isr(ISIX_EFIFOFULL);
		}
		tests::detail::periodic_timer_stop();
	}, 1000);
	TEST_ASSERT_TRUE(started);
	const auto t0 = isix::get_jiffies();
	while (!fired && !isix::timer_elapsed(t0, isix::ms2tick(500))) {
		isix::wait_ms(1);
	}
	TEST_ASSERT_TRUE(fired);
	// The ISR signal is still queued, the later signal must follow it and not overtake it
	ev.set(ISIX_EOK);
	TEST_ASSERT_TRUE(drive(sched, [&w] { return w.done(); }));
	TEST_ASSERT_EQUAL(ISIX_EOK, rc);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, leftover);
}

TEST(coroutines_ext, event_set_from_inbox_handler_of_queued_event)
{
	struct probe {
		isix::co::event* ev {};
		isix::co::detail::signal_link link;
		probe(isix::co::event& e) : ev(&e), link(&probe::cb, this) {}
		static void cb(void* ctx) { static_cast<probe*>(ctx)->ev->set(ISIX_EOK); }
	};
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	probe p(ev);
	int rc { ISIX_ESTATE };
	int leftover { ISIX_ESTATE };
	volatile bool fired { false };
	auto waiter = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(500));
		leftover = co_await ev.wait_for(ISIX_TIME_DONTWAIT);
	};
	auto w = waiter();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, w));
	sched.step();
	// Inbox order: the probe, then the event. The probe signals the event that is queued behind it.
	sched.signal(p.link);
	const bool started = tests::detail::periodic_timer_setup([&] {
		if (!fired) {
			fired = true;
			ev.set_isr(ISIX_EFIFOFULL);
		}
		tests::detail::periodic_timer_stop();
	}, 1000);
	TEST_ASSERT_TRUE(started);
	const auto t0 = isix::get_jiffies();
	while (!fired && !isix::timer_elapsed(t0, isix::ms2tick(500))) {
		isix::wait_ms(1);
	}
	TEST_ASSERT_TRUE(fired);
	TEST_ASSERT_TRUE(drive(sched, [&w] { return w.done(); }));
	TEST_ASSERT_EQUAL(ISIX_EOK, rc);
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, leftover);
}

TEST(coroutines_ext, event_set_from_abort_handler_while_timers_expire)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	signalling_abort_wait::target = &ev;
	signalling_abort_wait::node = nullptr;
	signalling_abort_wait::abort_calls = 0;
	int rc_x { ISIX_ESTATE };
	int rc_a { ISIX_ESTATE };
	int rc_b { ISIX_ESTATE };
	auto aborting = [&]() -> isix::co::task<void> {
		rc_x = co_await signalling_abort_wait(isix::ms2tick(10));
	};
	auto waiter = [&](int& rc, ostick_t timeout) -> isix::co::task<void> {
		rc = co_await ev.wait_for(timeout);
	};
	// Same deadline as the aborting node, so it is the next entry of the timer list being processed
	auto x = aborting();
	auto a = waiter(rc_a, isix::ms2tick(10));
	auto b = waiter(rc_b, isix::ms2tick(300));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, x));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, a));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, b));
	TEST_ASSERT_TRUE(drive(sched, [&] { return a.done() && b.done(); }));
	TEST_ASSERT_EQUAL_UINT32(1, signalling_abort_wait::abort_calls);
	TEST_ASSERT_EQUAL(ISIX_EOK, rc_a);
	TEST_ASSERT_EQUAL(ISIX_EOK, rc_b);
	TEST_ASSERT_FALSE(x.done());
	sched.wake(*signalling_abort_wait::node, ISIX_ETIMEOUT);
	TEST_ASSERT_TRUE(drive(sched, [&x] { return x.done(); }));
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc_x);
}

TEST(coroutines_ext, driver_thread_check_excludes_isr_and_other_threads)
{
	isix::co::scheduler sched;
	using access = isix::co::detail::scheduler_access;
	volatile bool in_isr { true };
	volatile bool fired { false };
	volatile bool in_other { true };
	volatile bool other_done { false };
	TEST_ASSERT_FALSE(access::in_driver_thread(sched));
	sched.step();
	TEST_ASSERT_TRUE(access::in_driver_thread(sched));
	auto other = isix::thread_create([&] {
		in_other = access::in_driver_thread(sched);
		other_done = true;
	});
	other.start_thread(c_stack_size, c_task_prio);
	// The ISR interrupts the driver thread, the check must still say no
	const bool started = tests::detail::periodic_timer_setup([&] {
		if (!fired) {
			in_isr = access::in_driver_thread(sched);
			fired = true;
		}
		tests::detail::periodic_timer_stop();
	}, 1000);
	TEST_ASSERT_TRUE(started);
	const auto t0 = isix::get_jiffies();
	while (!fired && !isix::timer_elapsed(t0, isix::ms2tick(500))) {
	}
	const auto t1 = isix::get_jiffies();
	while (!other_done && !isix::timer_elapsed(t1, isix::ms2tick(500))) {
		isix::wait_ms(1);
	}
	TEST_ASSERT_TRUE(fired);
	TEST_ASSERT_TRUE(other_done);
	TEST_ASSERT_FALSE(in_isr);
	TEST_ASSERT_FALSE(in_other);
}

TEST(coroutines_ext, yield_round_robin)
{
	isix::co::scheduler sched;
	std::array<int, 9> seen {};
	std::size_t n { 0 };
	auto worker = [&](int id) -> isix::co::task<void> {
		for (int round = 0; round < 3; ++round) {
			seen[n++] = id;
			co_await isix::co::yield();
		}
	};
	auto a = worker(1);
	auto b = worker(2);
	auto c = worker(3);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, a));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, b));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, c));
	TEST_ASSERT_TRUE(drive(sched, [&] { return a.done() && b.done() && c.done(); }));
	const std::array<int, 9> expected { 1, 2, 3, 1, 2, 3, 1, 2, 3 };
	TEST_ASSERT_EQUAL_INT_ARRAY(expected.data(), seen.data(), 9);
}

TEST(coroutines_ext, sleep_until_past_deadline_returns_now)
{
	isix::co::scheduler sched;
	osutick_t past_elapsed {};
	osutick_t future_elapsed {};
	auto coro = [&]() -> isix::co::task<void> {
		auto u0 = isix::get_ujiffies();
		co_await isix::co::sleep_until(isix::get_jiffies() - 50);
		past_elapsed = isix::get_ujiffies() - u0;
		u0 = isix::get_ujiffies();
		co_await isix::co::sleep_until(isix::get_jiffies() + isix::ms2tick(30));
		future_elapsed = isix::get_ujiffies() - u0;
	};
	isix::co::run(sched, coro());
	TEST_ASSERT_UINT_WITHIN(10ULL * 1000ULL, 0ULL, past_elapsed);
	TEST_ASSERT_UINT_WITHIN(25ULL * 1000ULL, 30ULL * 1000ULL, future_elapsed);
}

TEST(coroutines_ext, event_status_from_isr)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc { ISIX_ESTATE };
	volatile int isr_count { 0 };
	auto coro = [&]() -> isix::co::task<void> {
		rc = co_await ev.wait_for(isix::ms2tick(500));
	};
	const bool started = tests::detail::periodic_timer_setup([&] {
		isr_count = isr_count + 1;
		if (isr_count == 3) {
			ev.set_isr(ISIX_EFIFOFULL);
		} else if (isr_count > 3) {
			tests::detail::periodic_timer_stop();
		}
	}, 1000);
	TEST_ASSERT_TRUE(started);
	isix::co::run(sched, coro());
	tests::detail::periodic_timer_stop();
	TEST_ASSERT_EQUAL(ISIX_EFIFOFULL, rc);
	isix::wait_ms(10);
}

TEST(coroutines_ext, isr_resignal_during_dispatch)
{
	struct probe {
		isix::co::scheduler* sched {};
		isix::co::detail::signal_link first;
		isix::co::detail::signal_link second;
		unsigned first_runs {};
		unsigned second_runs {};
		bool resignal_first { true };

		probe(isix::co::scheduler& s)
			: sched(&s), first(&probe::first_cb, this), second(&probe::second_cb, this) {}

		static void first_cb(void* ctx)
		{
			auto& p = *static_cast<probe*>(ctx);
			++p.first_runs;
			if (p.resignal_first) {
				p.resignal_first = false;
				// Own link is free again, the other one is still waiting in the batch
				p.sched->signal(p.first);
				p.sched->signal(p.second);
			}
		}
		static void second_cb(void* ctx) { ++static_cast<probe*>(ctx)->second_runs; }
	};

	isix::co::scheduler sched;
	probe p(sched);
	sched.signal(p.first);
	sched.signal(p.second);
	sched.step();
	TEST_ASSERT_EQUAL_UINT(1, p.first_runs);
	TEST_ASSERT_EQUAL_UINT(1, p.second_runs);
	sched.step();
	TEST_ASSERT_EQUAL_UINT(2, p.first_runs);
	TEST_ASSERT_EQUAL_UINT(1, p.second_runs);
	sched.step();
	TEST_ASSERT_EQUAL_UINT(2, p.first_runs);
	sched.unsignal(p.first);
	sched.unsignal(p.second);
}

TEST(coroutines_ext, channel_recv_reports_reason)
{
	isix::co::scheduler sched;
	int timeout_rc { ISIX_ESTATE };
	int destroy_rc { ISIX_ESTATE };
	int cancel_rc { ISIX_ESTATE };
	int value { 0 };
	bool sent { false };
	bool done { false };
	isix::co::channel<int, 2> ch(sched);
	auto run_first = [&]() -> isix::co::task<void> {
		const auto v = co_await ch.recv(isix::ms2tick(10));
		timeout_rc = v.has_value() ? ISIX_EOK : v.error();
		sent = ch.try_send(5);
		const auto w = co_await ch.recv(isix::ms2tick(10));
		value = w.value_or(-1);
	};
	isix::co::run(sched, run_first());
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, timeout_rc);
	TEST_ASSERT_TRUE(sent);
	TEST_ASSERT_EQUAL(5, value);

	auto waiter = [&]() -> isix::co::task<void> {
		const auto v = co_await ch.recv();
		cancel_rc = v.has_value() ? ISIX_EOK : v.error();
		done = true;
	};
	done = false;
	auto t = waiter();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	t.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, cancel_rc);

	{
		auto ch2 = std::make_unique<isix::co::channel<int, 2>>(sched);
		auto waiter2 = [&]() -> isix::co::task<void> {
			const auto v = co_await ch2->recv();
			destroy_rc = v.has_value() ? ISIX_EOK : v.error();
		};
		auto t2 = waiter2();
		TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t2));
		sched.step();
		ch2.reset();
		TEST_ASSERT_TRUE(drive(sched, [&t2] { return t2.done(); }));
	}
	TEST_ASSERT_EQUAL(ISIX_EDESTROY, destroy_rc);
}

TEST(coroutines_ext, generator_range_for)
{
	auto count_to = [](int n) -> isix::co::generator<int> {
		for (int i = 1; i <= n; ++i) {
			co_yield i;
		}
	};
	int sum { 0 };
	int items { 0 };
	for (const auto v : count_to(5)) {
		sum += v;
		++items;
	}
	TEST_ASSERT_EQUAL(15, sum);
	TEST_ASSERT_EQUAL(5, items);
	int none { 0 };
	for (const auto v : count_to(0)) {
		none += v;
	}
	TEST_ASSERT_EQUAL(0, none);
}

TEST(coroutines_ext, stream_isr_writer_bytes_in_order)
{
	constexpr int c_chunks = 4;
	isix::co::scheduler sched;
	isix::co::stream_buffer<64> stream(sched);
	std::array<std::uint8_t, 16 * c_chunks> got {};
	std::size_t total { 0 };
	int bad_rc { 0 };
	volatile std::uint8_t next_byte { 0 };
	auto reader = [&]() -> isix::co::task<void> {
		while (total < got.size()) {
			std::array<std::uint8_t, 16> chunk {};
			const auto r = co_await stream.read(std::span<std::uint8_t>(chunk), 16, isix::ms2tick(500));
			if (!r || *r != 16U) {
				++bad_rc;
				co_return;
			}
			std::copy(chunk.begin(), chunk.end(), got.begin() + static_cast<std::ptrdiff_t>(total));
			total += *r;
		}
	};
	const bool started = tests::detail::periodic_timer_setup([&] {
		if (next_byte < got.size()) {
			std::array<std::byte, 4> part {};
			for (auto& b : part) {
				b = static_cast<std::byte>(next_byte);
				next_byte = static_cast<std::uint8_t>(next_byte + 1U);
			}
			stream.write_isr(part);
		} else {
			tests::detail::periodic_timer_stop();
		}
	}, 1000);
	TEST_ASSERT_TRUE(started);
	isix::co::run(sched, reader());
	tests::detail::periodic_timer_stop();
	TEST_ASSERT_EQUAL(0, bad_rc);
	TEST_ASSERT_EQUAL_UINT32(got.size(), total);
	for (std::size_t n = 0; n < got.size(); ++n) {
		TEST_ASSERT_EQUAL_UINT8(n, got[n]);
	}
	TEST_ASSERT_EQUAL_UINT32(0, stream.dropped());
	isix::wait_ms(10);
}

TEST(coroutines_ext, stream_read_timeout)
{
	isix::co::scheduler sched;
	isix::co::stream_buffer<16> stream(sched);
	int rc { ISIX_ESTATE };
	osutick_t elapsed {};
	auto reader = [&]() -> isix::co::task<void> {
		std::array<std::byte, 8> buf {};
		const auto u0 = isix::get_ujiffies();
		const auto r = co_await stream.read(buf, 4, isix::ms2tick(20));
		elapsed = isix::get_ujiffies() - u0;
		rc = r ? ISIX_EOK : r.error();
	};
	isix::co::run(sched, reader());
	TEST_ASSERT_EQUAL(ISIX_ETIMEOUT, rc);
	TEST_ASSERT_UINT_WITHIN(40ULL * 1000ULL, 20ULL * 1000ULL, elapsed);
}

TEST(coroutines_ext, stream_overflow_counts_dropped)
{
	isix::co::scheduler sched;
	isix::co::stream_buffer<8> stream(sched);
	std::array<std::byte, 20> data {};
	for (std::size_t n = 0; n < data.size(); ++n) {
		data[n] = static_cast<std::byte>(n);
	}
	TEST_ASSERT_EQUAL_UINT32(8, stream.write(data));
	TEST_ASSERT_EQUAL_UINT32(12, stream.dropped());
	TEST_ASSERT_EQUAL_UINT32(8, stream.available());
	std::size_t got { 0 };
	std::array<std::byte, 16> out {};
	auto reader = [&]() -> isix::co::task<void> {
		const auto r = co_await stream.read(out);
		got = r.value_or(0);
	};
	isix::co::run(sched, reader());
	TEST_ASSERT_EQUAL_UINT32(8, got);
	for (std::size_t n = 0; n < got; ++n) {
		TEST_ASSERT_EQUAL_UINT8(n, static_cast<std::uint8_t>(out[n]));
	}
	TEST_ASSERT_EQUAL_UINT32(0, stream.available());
}

TEST(coroutines_ext, offload_blocking_call)
{
	isix::co::scheduler sched;
	isix::co::worker_thread worker(sched);
	TEST_ASSERT_EQUAL(ISIX_EOK, worker.start(c_stack_size, c_task_prio));
	int rc { ISIX_ESTATE };
	int ticks { 0 };
	bool job_ran { false };
	bool done { false };
	osutick_t elapsed {};
	auto job = [&job_ran] {
		isix::wait_ms(40);
		job_ran = true;
	};
	auto ticker = [&]() -> isix::co::task<void> {
		while (!done) {
			co_await isix::co::sleep_ms(5);
			++ticks;
		}
	};
	auto caller = [&]() -> isix::co::task<void> {
		const auto u0 = isix::get_ujiffies();
		rc = co_await worker.run(job);
		elapsed = isix::get_ujiffies() - u0;
		done = true;
	};
	auto t1 = ticker();
	auto t2 = caller();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t1));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t2));
	TEST_ASSERT_TRUE(drive(sched, [&] { return t1.done() && t2.done(); }));
	TEST_ASSERT_EQUAL(ISIX_EOK, rc);
	TEST_ASSERT_TRUE(job_ran);
	TEST_ASSERT_TRUE(ticks >= 3);
	TEST_ASSERT_UINT_WITHIN(50ULL * 1000ULL, 40ULL * 1000ULL, elapsed);
}

TEST(coroutines_ext, offload_cancel_waits_for_completion)
{
	isix::co::scheduler sched;
	isix::co::worker_thread worker(sched);
	TEST_ASSERT_EQUAL(ISIX_EOK, worker.start(c_stack_size, c_task_prio));
	volatile bool job_finished { false };
	int rc { ISIX_ESTATE };
	bool finished_at_resume { false };
	bool done { false };
	osutick_t after_cancel {};
	auto caller = [&]() -> isix::co::task<void> {
		int local { 0 };
		auto job = [&local, &job_finished] {
			isix::wait_ms(60);
			local = 7;
			job_finished = true;
		};
		rc = co_await worker.run(job);
		finished_at_resume = job_finished && local == 7;
		done = true;
	};
	auto t = caller();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	isix::wait_ms(10);
	TEST_ASSERT_FALSE(job_finished);
	after_cancel = isix::get_ujiffies();
	t.cancel();
	TEST_ASSERT_TRUE(drive(sched, [&done] { return done; }));
	after_cancel = isix::get_ujiffies() - after_cancel;
	TEST_ASSERT_EQUAL(ISIX_ECANCELED, rc);
	TEST_ASSERT_TRUE(finished_at_resume);
	TEST_ASSERT_TRUE(after_cancel >= 30ULL * 1000ULL);
}

#if CONFIG_ISIX_TICKLESS
TEST(coroutines_ext, idle_scheduler_is_tickless)
{
	_isixp_test_tickless_force_wfi(true);
	_isixp_test_tickless_stats_reset();
	isix::co::scheduler sched;
	osutick_t elapsed {};
	auto coro = [&]() -> isix::co::task<void> {
		const auto u0 = isix::get_ujiffies();
		co_await isix::co::sleep_ms(300);
		elapsed = isix::get_ujiffies() - u0;
	};
	isix::co::run(sched, coro());
	isix_tickless_stats st {};
	_isixp_test_tickless_stats(&st);
	_isixp_test_tickless_force_wfi(false);
	tiny_printf("co tickless: sleeps %u expired %u restarts %u\r\n", static_cast<unsigned>(st.sleeps),
		static_cast<unsigned>(st.expired), static_cast<unsigned>(st.restarts));
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1U, st.sleeps);
	TEST_ASSERT_UINT_WITHIN(55ULL * 1000ULL, 300ULL * 1000ULL, elapsed);
}

TEST(coroutines_ext, scheduler_wakes_only_on_events)
{
	_isixp_test_tickless_force_wfi(true);
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	int rc { ISIX_ESTATE };
	auto coro = [&]() -> isix::co::task<void> { rc = co_await ev.wait_for(isix::ms2tick(2000)); };
	auto thr = isix::thread_create([&ev] {
		isix::wait_ms(200);
		ev.set();
	});
	thr.start_thread(c_stack_size, c_task_prio);
	auto t = coro();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, t));
	sched.step();
	_isixp_test_tickless_stats_reset();
	TEST_ASSERT_TRUE(drive(sched, [&t] { return t.done(); }));
	isix_tickless_stats st {};
	_isixp_test_tickless_stats(&st);
	_isixp_test_tickless_force_wfi(false);
	tiny_printf("co tickless events: sleeps %u expired %u restarts %u\r\n", static_cast<unsigned>(st.sleeps),
		static_cast<unsigned>(st.expired), static_cast<unsigned>(st.restarts));
	TEST_ASSERT_EQUAL(ISIX_EOK, rc);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1U, st.sleeps);
	TEST_ASSERT_LESS_OR_EQUAL_UINT32(8U, st.sleeps);
	TEST_ASSERT_LESS_OR_EQUAL_UINT32(2U, st.restarts);
	isix::wait_ms(10);
}
#endif

#if CONFIG_ISIX_CO_DEBUG
TEST(coroutines_ext, stats_count_frames_and_resumes)
{
	isix::co::scheduler sched;
	const auto before = sched.stats();
	auto worker = []() -> isix::co::task<void> {
		co_await isix::co::sleep_ms(3);
		co_await isix::co::sleep_ms(3);
	};
	auto a = worker();
	auto b = worker();
	auto c = worker();
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, a, "a"));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, b, "b"));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, c, "c"));
	const auto created = sched.stats();
	TEST_ASSERT_EQUAL_UINT32(before.live_frames + 3U, created.live_frames);
	TEST_ASSERT_TRUE(created.frame_bytes > before.frame_bytes);
	sched.step();
	unsigned listed { 0 };
	unsigned waiting { 0 };
	bool names_ok { true };
	sched.for_each_task([&](const isix::co::task_info& info) {
		++listed;
		waiting += info.state == isix::co::task_state::waiting ? 1U : 0U;
		names_ok = names_ok && info.name && info.name[0] >= 'a' && info.name[0] <= 'c';
	});
	TEST_ASSERT_EQUAL_UINT32(3, listed);
	TEST_ASSERT_EQUAL_UINT32(3, waiting);
	TEST_ASSERT_TRUE(names_ok);
	TEST_ASSERT_TRUE(drive(sched, [&] { return a.done() && b.done() && c.done(); }));
	const auto after = sched.stats();
	TEST_ASSERT_TRUE(after.resumes >= before.resumes + 9U);
	TEST_ASSERT_TRUE(after.peak_frame_bytes >= created.frame_bytes);
	a.destroy();
	b.destroy();
	c.destroy();
	const auto freed = sched.stats();
	TEST_ASSERT_EQUAL_UINT32(before.live_frames, freed.live_frames);
	TEST_ASSERT_EQUAL_UINT32(before.frame_bytes, freed.frame_bytes);
}

TEST(coroutines_ext, stats_detect_leak_after_release)
{
	isix::co::scheduler sched;
	const auto before = sched.stats();
	auto coro = []() -> isix::co::task<void> { co_return; };
	auto t = coro();
	TEST_ASSERT_EQUAL_UINT32(before.live_frames + 1U, sched.stats().live_frames);
	auto h = t.release();
	TEST_ASSERT_EQUAL_UINT32(before.live_frames + 1U, sched.stats().live_frames);
	h.destroy();
	TEST_ASSERT_EQUAL_UINT32(before.live_frames, sched.stats().live_frames);
}
#endif

#if CONFIG_ISIX_CO_TRACE
namespace {
std::array<unsigned, 7> s_trace_counts {};
void count_trace(isix::co::trace_event e, const char*, const void*)
{
	++s_trace_counts[static_cast<std::size_t>(e)];
}
} // namespace

TEST(coroutines_ext, trace_hook_sees_lifecycle)
{
	s_trace_counts.fill(0);
	isix::co::set_trace_hook(&count_trace);
	{
		isix::co::scheduler sched;
		auto worker = []() -> isix::co::task<void> { co_await isix::co::sleep_ms(5); };
		auto a = worker();
		auto b = worker();
		auto c = worker();
		isix::co::event ev(sched);
		auto waiter = [&]() -> isix::co::task<void> { co_await ev.wait_for(isix::ms2tick(1000)); };
		auto d = waiter();
		TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, a));
		TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, b));
		TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, c));
		TEST_ASSERT_EQUAL(ISIX_EOK, isix::co::spawn(sched, d));
		TEST_ASSERT_TRUE(drive(sched, [&] { return a.done() && b.done() && c.done(); }));
		d.cancel();
		TEST_ASSERT_TRUE(drive(sched, [&d] { return d.done(); }));
	}
	isix::co::set_trace_hook(nullptr);
	const auto count = [](isix::co::trace_event e) { return s_trace_counts[static_cast<std::size_t>(e)]; };
	TEST_ASSERT_EQUAL_UINT32(4, count(isix::co::trace_event::spawn));
	TEST_ASSERT_EQUAL_UINT32(4, count(isix::co::trace_event::finish));
	TEST_ASSERT_EQUAL_UINT32(4, count(isix::co::trace_event::destroy));
	TEST_ASSERT_EQUAL_UINT32(1, count(isix::co::trace_event::cancel));
	TEST_ASSERT_EQUAL_UINT32(3, count(isix::co::trace_event::timeout));
	TEST_ASSERT_TRUE(count(isix::co::trace_event::suspend) >= 4U);
	TEST_ASSERT_TRUE(count(isix::co::trace_event::resume) >= 8U);
}
#endif

#if CONFIG_ISIX_CO_TEST_SHUFFLE
namespace {

//! Two coroutines whose combined effect depends on the order they resume in
int run_order_probe()
{
	isix::co::scheduler sched;
	int value { 0 };
	auto doubler = [&]() -> isix::co::task<void> {
		value *= 2;
		co_return;
	};
	auto adder = [&]() -> isix::co::task<void> {
		value += 1;
		co_return;
	};
	auto a = doubler();
	auto b = adder();
	static_cast<void>(isix::co::spawn(sched, a));
	static_cast<void>(isix::co::spawn(sched, b));
	sched.step();
	return value;
}

} // namespace

TEST(coroutines_ext, shuffle_finds_order_dependence)
{
	isix::co::test::enable(false);
	for (int n = 0; n < 8; ++n) {
		TEST_ASSERT_EQUAL(1, run_order_probe());
	}
	isix::co::test::enable(true);
	std::uint32_t failing_seed { 0 };
	for (std::uint32_t seed = 1; seed <= 32 && failing_seed == 0U; ++seed) {
		isix::co::test::set_seed(seed);
		if (run_order_probe() != 1) {
			failing_seed = isix::co::test::get_seed();
		}
	}
	isix::co::test::enable(false);
	tiny_printf("co shuffle: order dependent result with seed %u\r\n", static_cast<unsigned>(failing_seed));
	TEST_ASSERT_TRUE(failing_seed != 0U);
	isix::co::test::set_seed(failing_seed);
	isix::co::test::enable(true);
	const int first = run_order_probe();
	isix::co::test::set_seed(failing_seed);
	const int second = run_order_probe();
	isix::co::test::enable(false);
	TEST_ASSERT_EQUAL(first, second);
}
#endif

TEST(coroutines_ext, stress_200_coroutines_100_channels)
{
	constexpr std::size_t c_pipes = 2;
	constexpr std::size_t c_stages = 50;
	constexpr int c_messages = 200;
	constexpr std::size_t c_tickers = 98;
	using channel_t = isix::co::channel<int, 2>;
	const auto heap_before = heap_used();
	const auto t0 = isix::get_jiffies();
	bool finished { false };
	{
		isix::co::scheduler sched;
		isix::co::event tick(sched);
		std::array<std::array<std::optional<channel_t>, c_stages>, c_pipes> chans {};
		for (auto& pipe : chans) {
			for (auto& ch : pipe) {
				ch.emplace(sched);
			}
		}
		std::array<int, c_pipes> sunk {};
		std::array<int, c_pipes> bad {};
		std::array<unsigned, c_tickers> ticks {};
		volatile bool stop { false };
		bool storm_done { false };
		auto source = [&](std::size_t p) -> isix::co::task<void> {
			for (int n = 0; n < c_messages; ++n) {
				if (co_await chans[p][0]->send(n, isix::ms2tick(2000)) != ISIX_EOK) {
					++bad[p];
					co_return;
				}
			}
		};
		auto forward = [&](std::size_t p, std::size_t stage) -> isix::co::task<void> {
			for (int n = 0; n < c_messages; ++n) {
				const auto v = co_await chans[p][stage]->recv(isix::ms2tick(2000));
				if (!v || co_await chans[p][stage + 1]->send(*v, isix::ms2tick(2000)) != ISIX_EOK) {
					++bad[p];
					co_return;
				}
			}
		};
		auto sink = [&](std::size_t p) -> isix::co::task<void> {
			for (int n = 0; n < c_messages; ++n) {
				const auto v = co_await chans[p][c_stages - 1]->recv(isix::ms2tick(2000));
				if (!v || *v != n) {
					++bad[p];
					co_return;
				}
				++sunk[p];
			}
		};
		auto ticker = [&](std::size_t idx) -> isix::co::task<void> {
			while (!stop) {
				if (co_await tick.wait_for(isix::ms2tick(100)) == ISIX_EOK) {
					++ticks[idx];
				}
			}
		};
		isix::co::async_scope scope(sched);
		bool spawn_ok { true };
		for (std::size_t p = 0; p < c_pipes; ++p) {
			spawn_ok = spawn_ok && scope.spawn(source(p)) == ISIX_EOK;
			for (std::size_t st = 0; st + 1 < c_stages; ++st) {
				spawn_ok = spawn_ok && scope.spawn(forward(p, st)) == ISIX_EOK;
			}
			spawn_ok = spawn_ok && scope.spawn(sink(p)) == ISIX_EOK;
		}
		for (std::size_t n = 0; n < c_tickers; ++n) {
			spawn_ok = spawn_ok && scope.spawn(ticker(n)) == ISIX_EOK;
		}
		const bool started = tests::detail::periodic_timer_setup([&] { tick.set_isr(); }, 1000);
		TEST_ASSERT_TRUE(started);
		TEST_ASSERT_TRUE(spawn_ok);
		TEST_ASSERT_TRUE(drive(sched, [&] { return sunk[0] == c_messages && sunk[1] == c_messages; }, 20000));
		stop = true;
		tick.set();
		storm_done = drive(sched, [&scope] { return scope.size() == 0; }, 5000);
		tests::detail::periodic_timer_stop();
		finished = storm_done;
		TEST_ASSERT_EQUAL(0, bad[0] + bad[1]);
		unsigned total_ticks { 0 };
		for (const auto v : ticks) {
			total_ticks += v;
		}
		TEST_ASSERT_TRUE(total_ticks > 0U);
	}
	TEST_ASSERT_TRUE(finished);
	TEST_ASSERT_TRUE(isix::get_jiffies() - t0 < isix::ms2tick(15000));
	TEST_ASSERT_TRUE(heap_back_to(heap_before));
}

TEST(coroutines_ext, stress_cancel_storm)
{
	constexpr std::size_t c_slots = 8;
	constexpr int c_rounds_per_slot = 125;
	const auto heap_before = heap_used();
	{
		isix::co::scheduler sched;
		isix::co::event never(sched);
		std::array<std::optional<isix::co::stop_source>, c_slots> sources {};
		for (auto& s : sources) {
			s.emplace(sched);
		}
		std::array<int, c_slots> rounds {};
		std::array<int, c_slots> cancelled {};
		volatile bool storm_run { true };
		auto victim = [&]() -> isix::co::task<void> {
			co_await never.wait_for(isix::ms2tick(30));
			co_await isix::co::sleep_ms(1);
		};
		auto slot = [&](std::size_t idx) -> isix::co::task<void> {
			for (int r = 0; r < c_rounds_per_slot; ++r) {
				auto t = victim();
				sources[idx]->reset();
				sources[idx]->bind(t);
				co_await std::move(t);
				sources[idx]->unbind();
				++rounds[idx];
			}
		};
		auto storm = isix::thread_create([&] {
			std::uint32_t x = 2463534242U;
			while (storm_run) {
				x ^= x << 13;
				x ^= x >> 17;
				x ^= x << 5;
				sources[x % c_slots]->request_stop();
				isix::wait_ms(1);
			}
		});
		storm.start_thread(c_stack_size, c_task_prio);
		isix::co::async_scope scope(sched);
		bool spawn_ok { true };
		for (std::size_t n = 0; n < c_slots; ++n) {
			spawn_ok = spawn_ok && scope.spawn(slot(n)) == ISIX_EOK;
		}
		TEST_ASSERT_TRUE(spawn_ok);
		const bool ok = drive(sched, [&scope] { return scope.size() == 0; }, 30000);
		storm_run = false;
		isix::wait_ms(20);
		TEST_ASSERT_TRUE(ok);
		int total { 0 };
		for (const auto r : rounds) {
			total += r;
		}
		static_cast<void>(cancelled);
		TEST_ASSERT_EQUAL(static_cast<int>(c_slots) * c_rounds_per_slot, total);
	}
	TEST_ASSERT_TRUE(heap_back_to(heap_before));
}

TEST_GROUP_RUNNER(coroutines_ext)
{
	RUN_TEST_CASE(coroutines_ext, hundred_events_on_one_scheduler);
	RUN_TEST_CASE(coroutines_ext, isr_resignal_during_dispatch);
	RUN_TEST_CASE(coroutines_ext, frame_storage_runs_without_heap);
	RUN_TEST_CASE(coroutines_ext, frame_storage_too_small_reports_enomem);
	RUN_TEST_CASE(coroutines_ext, frame_pool_exhaustion_and_reuse);
	RUN_TEST_CASE(coroutines_ext, global_allocator_failure_is_not_fatal);
	RUN_TEST_CASE(coroutines_ext, cancel_wakes_event_wait_with_ecanceled);
	RUN_TEST_CASE(coroutines_ext, cancel_propagates_to_nested_leaf);
	RUN_TEST_CASE(coroutines_ext, cancel_before_start);
	RUN_TEST_CASE(coroutines_ext, cancelled_task_next_await_returns_immediately);
	RUN_TEST_CASE(coroutines_ext, cancel_while_in_ready_queue);
	RUN_TEST_CASE(coroutines_ext, cancel_releases_scoped_mutex);
	RUN_TEST_CASE(coroutines_ext, stop_source_from_thread);
	RUN_TEST_CASE(coroutines_ext, stop_source_from_isr);
	RUN_TEST_CASE(coroutines_ext, on_abort_replaces_timeout_wakeup);
	RUN_TEST_CASE(coroutines_ext, on_abort_receives_cancellation);
	RUN_TEST_CASE(coroutines_ext, mutex_unlock_by_non_owner_rejected);
	RUN_TEST_CASE(coroutines_ext, when_all_two_values);
	RUN_TEST_CASE(coroutines_ext, when_all_with_sleeps_runs_concurrently);
	RUN_TEST_CASE(coroutines_ext, when_all_cancel_propagates_to_all);
	RUN_TEST_CASE(coroutines_ext, when_any_returns_winner_index);
	RUN_TEST_CASE(coroutines_ext, when_any_losers_finish_before_resume);
	RUN_TEST_CASE(coroutines_ext, when_any_event_vs_timeout_from_isr);
	RUN_TEST_CASE(coroutines_ext, with_timeout_expires);
	RUN_TEST_CASE(coroutines_ext, with_timeout_completes_in_time);
	RUN_TEST_CASE(coroutines_ext, with_timeout_nested_in_when_any);
	RUN_TEST_CASE(coroutines_ext, scope_spawn_and_join);
	RUN_TEST_CASE(coroutines_ext, scope_cancel_all);
	RUN_TEST_CASE(coroutines_ext, post_from_thread_runs_on_scheduler);
	RUN_TEST_CASE(coroutines_ext, scope_destructor_cleans_up);
	RUN_TEST_CASE(coroutines_ext, scheduler_thread_two_priorities);
	RUN_TEST_CASE(coroutines_ext, yield_round_robin);
	RUN_TEST_CASE(coroutines_ext, signal_during_resume_from_higher_prio_thread);
	RUN_TEST_CASE(coroutines_ext, signal_during_resume_from_lower_prio_thread);
	RUN_TEST_CASE(coroutines_ext, event_set_from_scheduler_thread_wakes_directly);
	RUN_TEST_CASE(coroutines_ext, event_set_behind_queued_isr_signal_keeps_order);
	RUN_TEST_CASE(coroutines_ext, event_set_from_inbox_handler_of_queued_event);
	RUN_TEST_CASE(coroutines_ext, event_set_from_abort_handler_while_timers_expire);
	RUN_TEST_CASE(coroutines_ext, driver_thread_check_excludes_isr_and_other_threads);
	RUN_TEST_CASE(coroutines_ext, sleep_until_past_deadline_returns_now);
	RUN_TEST_CASE(coroutines_ext, event_status_from_isr);
	RUN_TEST_CASE(coroutines_ext, channel_recv_reports_reason);
	RUN_TEST_CASE(coroutines_ext, generator_range_for);
	RUN_TEST_CASE(coroutines_ext, stream_isr_writer_bytes_in_order);
	RUN_TEST_CASE(coroutines_ext, stream_read_timeout);
	RUN_TEST_CASE(coroutines_ext, stream_overflow_counts_dropped);
	RUN_TEST_CASE(coroutines_ext, offload_blocking_call);
	RUN_TEST_CASE(coroutines_ext, stress_200_coroutines_100_channels);
	RUN_TEST_CASE(coroutines_ext, stress_cancel_storm);
	RUN_TEST_CASE(coroutines_ext, offload_cancel_waits_for_completion);
#if CONFIG_ISIX_TICKLESS
	RUN_TEST_CASE(coroutines_ext, idle_scheduler_is_tickless);
	RUN_TEST_CASE(coroutines_ext, scheduler_wakes_only_on_events);
#endif
#if CONFIG_ISIX_CO_DEBUG
	RUN_TEST_CASE(coroutines_ext, stats_count_frames_and_resumes);
	RUN_TEST_CASE(coroutines_ext, stats_detect_leak_after_release);
#endif
#if CONFIG_ISIX_CO_TRACE
	RUN_TEST_CASE(coroutines_ext, trace_hook_sees_lifecycle);
#endif
#if CONFIG_ISIX_CO_TEST_SHUFFLE
	RUN_TEST_CASE(coroutines_ext, shuffle_finds_order_dependence);
#endif
#if CONFIG_ISIX_FIFO_EVENT_NOTIFY
	RUN_TEST_CASE(coroutines_ext, fifo_reader_second_consumer_does_not_block);
#endif
}

#else // !CONFIG_ISIX_CPP_COROUTINES

TEST_GROUP_RUNNER(coroutines_ext) {}

#endif
