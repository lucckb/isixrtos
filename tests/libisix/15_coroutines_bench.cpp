/*
 * Coroutine size and speed measurements.
 *
 * Author: Lucjan Bryndza
 */

/* Enable isix::co via isix.h only in this translation unit (keeps other tests lean). */
#define CONFIG_ISIX_CPP_COROUTINES 1

#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <foundation/sys/tiny_printf.h>
#include "timer_interrupt.hpp"
#include <stm32_ll_system.h>
#include <array>
#include <cstdint>

#if CONFIG_ISIX_CPP_COROUTINES

namespace {

//! CPU cycle counter; QEMU does not count, then microseconds are used
class cycle_clock {
public:
	cycle_clock()
	{
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CYCCNT = 0;
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
		const auto a = DWT->CYCCNT;
		for (int n = 0; n < 100; ++n) {
			asm volatile("nop");
		}
		m_cycles = DWT->CYCCNT != a;
	}

	[[nodiscard]] bool cycles() const noexcept { return m_cycles; }
	[[nodiscard]] const char* unit() const noexcept { return m_cycles ? "cycles" : "us"; }

	[[nodiscard]] static std::uint32_t raw_cycles() noexcept { return DWT->CYCCNT; }

	[[nodiscard]] std::uint32_t now() const noexcept
	{
		return m_cycles ? DWT->CYCCNT : static_cast<std::uint32_t>(isix::get_ujiffies());
	}

private:
	bool m_cycles { false };
};

constexpr unsigned c_iterations = 1000;
constexpr unsigned c_samples = 100;

struct latency_stats {
	std::uint32_t min { UINT32_MAX };
	std::uint32_t max {};
	std::uint64_t sum {};
	unsigned count {};

	void add(std::uint32_t v) noexcept
	{
		min = v < min ? v : min;
		max = v > max ? v : max;
		sum += v;
		++count;
	}

	[[nodiscard]] std::uint32_t avg() const noexcept
	{
		return count ? static_cast<std::uint32_t>(sum / count) : 0U;
	}
};

constexpr unsigned c_round_trips = 500;

volatile std::uint32_t g_isr_stamp {};
std::uint32_t g_switch_thread {};

//! Poll a flag set by another task, up to 5 s
void wait_flag(const volatile bool& flag)
{
	const auto t0 = isix::get_jiffies();
	while (!flag && !isix::timer_elapsed(t0, isix::ms2tick(5000))) {
		isix::wait_ms(10);
	}
}
volatile unsigned g_isr_count {};

void print_latency(const char* what, const latency_stats& st)
{
	// Cycles at the core clock; one cycle is 10 ns at 100 MHz
	tiny_printf("co bench: %s latency over %u samples: min %lu avg %lu max %lu cycles\r\n", what, st.count,
		static_cast<unsigned long>(st.min), static_cast<unsigned long>(st.avg()),
		static_cast<unsigned long>(st.max));
}

} // namespace

TEST_GROUP(coroutines_bench);
TEST_SETUP(coroutines_bench) {}
TEST_TEAR_DOWN(coroutines_bench)
{
	tests::detail::periodic_timer_stop();
}

TEST(coroutines_bench, frame_sizes_report)
{
	constexpr auto node_size = sizeof(isix::co::detail::wait_node);
	constexpr auto promise_size = sizeof(isix::co::task_promise<void>);
	constexpr auto sched_size = sizeof(isix::co::scheduler);
	tiny_printf("co sizes: wait_node %u task_promise<void> %u scheduler %u\r\n",
		static_cast<unsigned>(node_size), static_cast<unsigned>(promise_size),
		static_cast<unsigned>(sched_size));
	TEST_ASSERT_LESS_OR_EQUAL_UINT32(64U, node_size);
	TEST_ASSERT_LESS_OR_EQUAL_UINT32(512U, sched_size);
}

TEST(coroutines_bench, await_ready_event_cycles)
{
	const cycle_clock clk;
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	std::uint32_t set_only {};
	std::uint32_t set_and_await {};
	auto coro = [&]() -> isix::co::task<void> {
		auto t0 = clk.now();
		for (unsigned n = 0; n < c_iterations; ++n) {
			ev.set();
		}
		set_only = clk.now() - t0;
		t0 = clk.now();
		for (unsigned n = 0; n < c_iterations; ++n) {
			ev.set();
			co_await ev;
		}
		set_and_await = clk.now() - t0;
	};
	isix::co::run(sched, coro());
	tiny_printf("co bench: set %lu %s, set + ready co_await %lu %s per %u iterations\r\n",
		static_cast<unsigned long>(set_only), clk.unit(), static_cast<unsigned long>(set_and_await),
		clk.unit(), c_iterations);
	TEST_ASSERT_TRUE(set_and_await > 0U);
}

namespace {

//! Time of signal + step + resume of a coroutine suspended on an event
std::uint32_t signal_step_resume(const cycle_clock& clk, ostick_t timeout, unsigned& counted)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	unsigned resumes { 0 };
	bool stop { false };
	auto coro = [&]() -> isix::co::task<void> {
		while (!stop) {
			if (co_await ev.wait_for(timeout) == ISIX_EOK) {
				++resumes;
			}
		}
	};
	auto t = coro();
	static_cast<void>(isix::co::spawn(sched, t));
	sched.step();
	const auto t0 = clk.now();
	for (unsigned n = 0; n < c_iterations; ++n) {
		ev.set();
		sched.step();
	}
	const auto total = clk.now() - t0;
	counted = resumes;
	stop = true;
	ev.set();
	sched.step();
	return total;
}

} // namespace

TEST(coroutines_bench, signal_step_resume_cycles)
{
	const cycle_clock clk;
	unsigned counted_plain {};
	unsigned counted_timeout {};
	const auto plain = signal_step_resume(clk, ISIX_TIME_INFINITE, counted_plain);
	const auto with_timeout = signal_step_resume(clk, isix::ms2tick(1000), counted_timeout);
	tiny_printf("co bench: signal + step + resume %lu %s, with a timeout %lu %s per %u iterations\r\n",
		static_cast<unsigned long>(plain), clk.unit(), static_cast<unsigned long>(with_timeout), clk.unit(),
		c_iterations);
	TEST_ASSERT_EQUAL_UINT32(c_iterations, counted_plain);
	TEST_ASSERT_EQUAL_UINT32(c_iterations, counted_timeout);
	TEST_ASSERT_TRUE(plain > 0U);
}

TEST(coroutines_bench, frame_alloc_cycles)
{
	const cycle_clock clk;
	isix::co::frame_pool<128, 2> pool;
	auto heap_coro = []() -> isix::co::task<void> { co_return; };
	auto pool_coro = [](isix::co::frame_pool<128, 2>&) -> isix::co::task<void> { co_return; };
	auto t0 = clk.now();
	for (unsigned n = 0; n < c_iterations; ++n) {
		auto t = heap_coro();
	}
	const auto heap_time = clk.now() - t0;
	t0 = clk.now();
	for (unsigned n = 0; n < c_iterations; ++n) {
		auto t = pool_coro(pool);
	}
	const auto pool_time = clk.now() - t0;
	tiny_printf("co bench: create + destroy a frame, heap %lu %s, pool %lu %s per %u iterations\r\n",
		static_cast<unsigned long>(heap_time), clk.unit(), static_cast<unsigned long>(pool_time),
		clk.unit(), c_iterations);
	TEST_ASSERT_TRUE(heap_time > 0U);
	TEST_ASSERT_TRUE(pool_time > 0U);
}

TEST(coroutines_bench, isr_to_resume_latency)
{
	const cycle_clock clk;
	if (!clk.cycles()) {
		tiny_printf("co bench: isr to coroutine latency n/a (no cycle counter)\r\n");
		return;
	}
	isix::co::scheduler_thread exec;
	TEST_ASSERT_EQUAL(ISIX_EOK, exec.start(4096, 2));
	isix::co::event ev(exec.sched());
	latency_stats stats;
	volatile bool done { false };
	auto body = [&]() -> isix::co::task<void> {
		for (unsigned n = 0; n < c_samples; ++n) {
			if (co_await ev.wait_for(ISIX_TIME_INFINITE) != ISIX_EOK) {
				break;
			}
			stats.add(cycle_clock::raw_cycles() - g_isr_stamp);
		}
		done = true;
	};
	TEST_ASSERT_EQUAL(ISIX_EOK, exec.post(body()));
	g_isr_count = 0;
	const bool started = tests::detail::periodic_timer_setup([&ev] {
		g_isr_stamp = cycle_clock::raw_cycles();
		g_isr_count = g_isr_count + 1;
		ev.set_isr();
	}, 1000);
	TEST_ASSERT_TRUE(started);
	const auto t0 = isix::get_jiffies();
	while (!done && !isix::timer_elapsed(t0, isix::ms2tick(3000))) {
		isix::wait_ms(10);
	}
	tests::detail::periodic_timer_stop();
	exec.stop();
	exec.wait_for();
	print_latency("isr to coroutine", stats);
	TEST_ASSERT_EQUAL_UINT32(c_samples, stats.count);
	TEST_ASSERT_TRUE(stats.min > 0U);
}

TEST(coroutines_bench, isr_to_thread_latency)
{
	const cycle_clock clk;
	if (!clk.cycles()) {
		tiny_printf("co bench: isr to thread latency n/a (no cycle counter)\r\n");
		return;
	}
	isix::semaphore sem(0);
	latency_stats stats;
	volatile bool done { false };
	auto thr = isix::thread_create([&] {
		for (unsigned n = 0; n < c_samples; ++n) {
			if (sem.wait(isix::ms2tick(200)) != ISIX_EOK) {
				break;
			}
			stats.add(cycle_clock::raw_cycles() - g_isr_stamp);
		}
		done = true;
	});
	thr.start_thread(4096, 2);
	const bool started = tests::detail::periodic_timer_setup([&sem] {
		g_isr_stamp = cycle_clock::raw_cycles();
		sem.signal_isr();
	}, 1000);
	TEST_ASSERT_TRUE(started);
	const auto t0 = isix::get_jiffies();
	while (!done && !isix::timer_elapsed(t0, isix::ms2tick(3000))) {
		isix::wait_ms(10);
	}
	tests::detail::periodic_timer_stop();
	isix::wait_ms(20);
	print_latency("isr to thread", stats);
	TEST_ASSERT_EQUAL_UINT32(c_samples, stats.count);
	TEST_ASSERT_TRUE(stats.min > 0U);
}
TEST(coroutines_bench, switch_thread_to_thread)
{
	const cycle_clock clk;
	if (!clk.cycles()) {
		tiny_printf("co bench: thread to thread switch n/a (no cycle counter)\r\n");
		return;
	}
	isix::semaphore ping(0);
	isix::semaphore pong(0);
	volatile bool done { false };
	std::uint32_t total {};
	auto peer = isix::thread_create([&] {
		for (unsigned n = 0; n < c_round_trips; ++n) {
			if (ping.wait(ISIX_TIME_INFINITE) != ISIX_EOK) {
				break;
			}
			pong.signal();
		}
	});
	auto driver = isix::thread_create([&] {
		const auto t0 = cycle_clock::raw_cycles();
		for (unsigned n = 0; n < c_round_trips; ++n) {
			ping.signal();
			if (pong.wait(ISIX_TIME_INFINITE) != ISIX_EOK) {
				break;
			}
		}
		total = cycle_clock::raw_cycles() - t0;
		done = true;
	});
	peer.start_thread(2048, 2);
	driver.start_thread(2048, 2);
	wait_flag(done);
	isix::wait_ms(20);
	g_switch_thread = total / (2U * c_round_trips);
	tiny_printf("co bench: thread to thread switch %lu cycles per hand-off (%u round trips)\r\n",
		static_cast<unsigned long>(g_switch_thread), c_round_trips);
	TEST_ASSERT_TRUE(done);
}

TEST(coroutines_bench, switch_coroutine_to_coroutine)
{
	const cycle_clock clk;
	if (!clk.cycles()) {
		tiny_printf("co bench: coroutine to coroutine switch n/a (no cycle counter)\r\n");
		return;
	}
	isix::co::scheduler_thread exec;
	TEST_ASSERT_EQUAL(ISIX_EOK, exec.start(4096, 2));
	isix::co::event ping(exec.sched());
	isix::co::event pong(exec.sched());
	volatile bool done { false };
	std::uint32_t total {};
	auto peer = [&]() -> isix::co::task<void> {
		for (unsigned n = 0; n < c_round_trips; ++n) {
			if (co_await ping != ISIX_EOK) {
				break;
			}
			pong.set();
		}
	};
	auto driver = [&]() -> isix::co::task<void> {
		const auto t0 = cycle_clock::raw_cycles();
		for (unsigned n = 0; n < c_round_trips; ++n) {
			ping.set();
			if (co_await pong != ISIX_EOK) {
				break;
			}
		}
		total = cycle_clock::raw_cycles() - t0;
		done = true;
	};
	TEST_ASSERT_EQUAL(ISIX_EOK, exec.post(peer()));
	TEST_ASSERT_EQUAL(ISIX_EOK, exec.post(driver()));
	wait_flag(done);
	exec.stop();
	exec.wait_for();
	const auto per_handoff = total / (2U * c_round_trips);
	tiny_printf("co bench: coroutine to coroutine switch %lu cycles per hand-off (%u round trips)\r\n",
		static_cast<unsigned long>(per_handoff), c_round_trips);
	TEST_ASSERT_TRUE(done);
	// Thread figure comes from the previous test when it ran
	if (g_switch_thread != 0U) {
		TEST_ASSERT_LESS_OR_EQUAL_UINT32(2U * g_switch_thread, per_handoff);
	}
}

namespace {

//! Coroutines with a growing number of suspension points, measured with extras/scripts/coro_frames.sh
[[gnu::noinline]] isix::co::task<void> probe_awaits_0() { co_return; }
[[gnu::noinline]] isix::co::task<int> probe_awaits_0_int() { co_return 1; }
[[gnu::noinline]] isix::co::task<void> probe_awaits_1(isix::co::event& ev) { co_await ev; }
[[gnu::noinline]] isix::co::task<void> probe_awaits_2(isix::co::event& ev) { co_await ev; co_await ev; }
[[gnu::noinline]] isix::co::task<void> probe_awaits_3(isix::co::event& ev) { co_await ev; co_await ev; co_await ev; }
[[gnu::noinline]] isix::co::task<void> probe_awaits_4(isix::co::event& ev)
{
	co_await ev;
	co_await ev;
	co_await ev;
	co_await ev;
}
[[gnu::noinline]] isix::co::task<void> probe_sleep_1() { co_await isix::co::sleep_ms(1); }
[[gnu::noinline]] isix::co::task<void> probe_nested(isix::co::event& ev) { co_await probe_awaits_1(ev); }

} // namespace

TEST(coroutines_bench, frame_probes_exist)
{
	isix::co::scheduler sched;
	isix::co::event ev(sched);
	{
		auto a = probe_awaits_0();
		auto b = probe_awaits_0_int();
		auto c = probe_awaits_1(ev);
		auto d = probe_awaits_2(ev);
		auto e = probe_awaits_3(ev);
		auto f = probe_awaits_4(ev);
		auto g = probe_sleep_1();
		auto h = probe_nested(ev);
		TEST_ASSERT_TRUE(a.valid() && b.valid() && c.valid() && d.valid());
		TEST_ASSERT_TRUE(e.valid() && f.valid() && g.valid() && h.valid());
	}
}

TEST_GROUP_RUNNER(coroutines_bench)
{
	RUN_TEST_CASE(coroutines_bench, frame_sizes_report);
	RUN_TEST_CASE(coroutines_bench, frame_probes_exist);
	RUN_TEST_CASE(coroutines_bench, await_ready_event_cycles);
	RUN_TEST_CASE(coroutines_bench, signal_step_resume_cycles);
	RUN_TEST_CASE(coroutines_bench, frame_alloc_cycles);
	RUN_TEST_CASE(coroutines_bench, isr_to_resume_latency);
	RUN_TEST_CASE(coroutines_bench, isr_to_thread_latency);
	RUN_TEST_CASE(coroutines_bench, switch_thread_to_thread);
	RUN_TEST_CASE(coroutines_bench, switch_coroutine_to_coroutine);
}

#else // !CONFIG_ISIX_CPP_COROUTINES

TEST_GROUP_RUNNER(coroutines_bench) {}

#endif
