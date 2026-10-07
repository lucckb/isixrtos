#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <memory>
#include <utils/test_prio.hpp>
#include <utils/tickless_testhooks.h>


namespace
{	//Delayed execution test
	void delegated_func(void* ptr) {
		auto& cnt = *reinterpret_cast<int*>(ptr);
		++cnt;
	}
	class timer : public isix::virtual_timer {
	public:
		~timer() {
			stop_sync();
		}
		unsigned counter() const {
			return m_counter;
		}
	protected:
		virtual void handle_timer() noexcept {
			++m_counter;
		}
	private:
		unsigned m_counter {};
	};
	//! Expected number of timer expirations during one second for a period given in ms
	unsigned exp_cnt(unsigned period_ms) {
		return isix::ms2tick(1000U) / isix::ms2tick(period_ms);
	}
}

namespace {
	//! Internal structure call info for trace call
	struct call_info {
		ostick_t last_call {};
		ostick_t start_call { isix_get_jiffies() };
		int count {};
	};
	//One shoot timer function
	void one_call_timer_fun(void *ptr) {
		auto* cinfo = reinterpret_cast<call_info*>(ptr);
		++cinfo->count;
		cinfo->last_call = isix_get_jiffies();
	}
}

//Vtimer modapi test
namespace {
	constexpr auto mod_on = test_utils::ms_ticks(2000U);
	constexpr auto mod_off = test_utils::ms_ticks(800U);
	constexpr auto mod_iter = 20;
	struct mod_info 
	{
		int on_cnt {};
		int off_cnt {};
		int err_cnt {};
		int tot_cnt {};
		ostick_t last_call { isix_get_jiffies() };
		bool on {};
		osvtimer_t tmr {};
		ossem_t fin {};
	};
	//Kept in static storage so teardown can release it also after a failed assertion
	mod_info s_mod_inf;

	inline bool mod_inrange(ostick_t t, ostick_t rng) {
		//Host jitter between the worker tick and the jiffies read can shorten the interval
		return t + test_utils::ms_ticks(10U) >= rng && t<=rng+mod_off/10;
	}

	void cyclic_modapi_func(void* ptr) 
	{
		auto* mi = reinterpret_cast<mod_info*>(ptr);
		if (mi->tot_cnt >= mod_iter) {
			isix_vtimer_mod(mi->tmr,OSVTIMER_CB_CANCEL);
			isix_sem_signal(mi->fin);
			return;
		} else if (mi->on) {
			isix_vtimer_mod(mi->tmr, mod_on);
		} else if (!mi->on) {
			isix_vtimer_mod(mi->tmr, mod_off);
		}
		auto cj1 = isix_get_jiffies();
		if (mod_inrange(cj1-mi->last_call,mod_on)) {
			++mi->on_cnt;
		} else if (mod_inrange(cj1-mi->last_call,mod_off)) {
			++mi->off_cnt;
		} else {
			++mi->err_cnt;
		}
		mi->on = !mi->on;
		mi->tot_cnt++;
		mi->last_call = cj1;
	}
}


TEST_GROUP(vtimer);
TEST_SETUP(vtimer) {}
TEST_TEAR_DOWN(vtimer)
{
	if (s_mod_inf.tmr) {
		isix_vtimer_destroy(s_mod_inf.tmr);
	}
	if (s_mod_inf.fin) {
		isix_sem_destroy(s_mod_inf.fin);
	}
	s_mod_inf = mod_info{};
}

TEST(vtimer, basic)
{
	static constexpr auto wait_t = 1000U;
	static constexpr auto t1 = 100U;
	static constexpr auto t2 = 3U;
	static constexpr auto t3 = 50U;
	timer m_t1;
	timer m_t2;
	timer m_t3;
	int del_exe_cnt = 0;
	TEST_ASSERT_EQUAL(ISIX_EOK, m_t1.start_ms(t1));
	TEST_ASSERT_EQUAL(ISIX_EOK, m_t2.start_ms(t2));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_schedule_work_isr(delegated_func,&del_exe_cnt));
	TEST_ASSERT_EQUAL(ISIX_EOK, m_t3.start_ms(t3));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_schedule_work_isr(delegated_func,&del_exe_cnt));
	isix_wait_ms(wait_t);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_schedule_work_isr(delegated_func,&del_exe_cnt));
	const auto ss1 = m_t1.stop();
	const auto ss2 = m_t2.stop();
	const auto ss3 = m_t3.stop();
	isix_wait_ms(50);	//Give some time to command exec
	TEST_ASSERT_EQUAL(ISIX_EOK, ss1);
	TEST_ASSERT_EQUAL(ISIX_EOK, ss2);
	TEST_ASSERT_EQUAL(ISIX_EOK, ss3);
	TEST_ASSERT_EQUAL_UINT(exp_cnt(t1), m_t1.counter());
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(exp_cnt(t2), m_t2.counter());
	TEST_ASSERT_LESS_THAN_UINT(exp_cnt(t2)+2, m_t2.counter());
	TEST_ASSERT_EQUAL_UINT(exp_cnt(t3), m_t3.counter());
	isix_wait_ms(wait_t);
	TEST_ASSERT_EQUAL_UINT(exp_cnt(t1), m_t1.counter());
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(exp_cnt(t2), m_t2.counter());
	TEST_ASSERT_LESS_THAN_UINT(exp_cnt(t2)+2, m_t2.counter());
	TEST_ASSERT_EQUAL_UINT(exp_cnt(t3), m_t3.counter());
	TEST_ASSERT_EQUAL(3, del_exe_cnt);
}

TEST(vtimer, isr_api)
{
	static constexpr auto wait_t = 1000U;
	static constexpr auto t1 = 100U;
	static constexpr auto t2 = 3U;
	static constexpr auto t3 = 50U;
	timer m_t1;
	timer m_t2;
	timer m_t3;
	TEST_ASSERT_EQUAL(ISIX_EOK, m_t1.start_ms_isr(t1));
	TEST_ASSERT_EQUAL(ISIX_EOK, m_t2.start_ms_isr(t2));
	TEST_ASSERT_EQUAL(ISIX_EOK, m_t3.start_ms_isr(t3));
	isix_wait_ms(wait_t);
	TEST_ASSERT_EQUAL(ISIX_EOK, m_t1.stop_isr());
	TEST_ASSERT_EQUAL(ISIX_EOK, m_t2.stop_isr());
	TEST_ASSERT_EQUAL(ISIX_EOK, m_t3.stop_isr());
	isix_wait_ms(t3+2);	//Give some time to exec command
	TEST_ASSERT_EQUAL_UINT(exp_cnt(t1), m_t1.counter());
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(exp_cnt(t2), m_t2.counter());
	TEST_ASSERT_LESS_THAN_UINT(exp_cnt(t2)+2, m_t2.counter());
	TEST_ASSERT_EQUAL_UINT(exp_cnt(t3), m_t3.counter());
	isix_wait_ms(wait_t);
	TEST_ASSERT_EQUAL_UINT(exp_cnt(t1), m_t1.counter());
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(exp_cnt(t2), m_t2.counter());
	TEST_ASSERT_LESS_THAN_UINT(exp_cnt(t2)+2, m_t2.counter());
	TEST_ASSERT_EQUAL_UINT(exp_cnt(t3), m_t3.counter());
}

TEST(vtimer, one_shoot)
{
	isix::memory_stat mstat;
	isix::heap_stats(mstat);
	const auto before_create = mstat.free;
	auto* timerh = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(timerh);
	//Run one shoot timer
	call_info ci;
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::vtimer_start(timerh, one_call_timer_fun, &ci, isix::ms2tick(100), false));
	isix_wait_ms(5);
	TEST_ASSERT(isix_vtimer_is_active(timerh));
	isix_wait_ms(1000);
	TEST_ASSERT_EQUAL(1, ci.count);
	//Host jitter can delay the start timestamp read, so allow a few ticks of slack
	TEST_ASSERT_GREATER_OR_EQUAL(isix::ms2tick(100), ci.last_call - ci.start_call);
	TEST_ASSERT_LESS_OR_EQUAL(isix::ms2tick(100) + isix::ms2tick(5), ci.last_call - ci.start_call);
	isix_wait_ms(200);
	TEST_ASSERT_FALSE(isix_vtimer_is_active(timerh));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(timerh));
	isix_wait_ms(50);
	isix::heap_stats(mstat);
	TEST_ASSERT_GREATER_OR_EQUAL(before_create, mstat.free);
}

TEST(vtimer, mod_api)
{
	auto& inf = s_mod_inf;
	inf = mod_info{};
	inf.fin = isix_sem_create_limited(NULL,0,1);
	TEST_ASSERT_NOT_NULL(inf.fin);
	inf.last_call = isix_get_jiffies();
	inf.tmr = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(inf.tmr);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_start(inf.tmr, cyclic_modapi_func, &inf, mod_on, true));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::sem_wait(inf.fin, isix::ms2tick(60*1000)));
	TEST_ASSERT_EQUAL(0, inf.err_cnt);
	TEST_ASSERT_EQUAL(mod_iter/2, inf.on_cnt);
	TEST_ASSERT_EQUAL(mod_iter/2, inf.off_cnt);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(inf.fin));
	inf.fin = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(inf.tmr));
	inf.tmr = nullptr;
}

TEST(vtimer, cpp11_api)
{
	int counter = 0;
	auto fn = [&]() -> void
	{
		++counter;
	};
	auto tim = isix::vtimer_create(fn);
	TEST_ASSERT_EQUAL(ISIX_EOK, tim.start_ms(10));
	isix::wait_ms(250);
	TEST_ASSERT_EQUAL(ISIX_EOK, tim.stop());
	isix::wait_ms(25);
	TEST_ASSERT_EQUAL(25, counter);
}



namespace {
	volatile unsigned wrap_fire_count;
	volatile ostick_t wrap_fire_time;
	void wrap_cb(void*) {
		wrap_fire_time = isix_get_jiffies();
		wrap_fire_count = wrap_fire_count + 1;
	}
	volatile unsigned cpp_due_count;
}

TEST(vtimer, start_across_jiffies_wrap_not_early)
{
	static constexpr auto timeout = test_utils::ms_ticks(50U);
	wrap_fire_count = 0;
	wrap_fire_time = 0;
	const auto tmr = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(tmr);
	// Let the worker settle then start close to the jiffies wrap, worker cannot see the command yet
	isix::wait_ms(5);
	_isixp_test_set_jiffies(0xFFFFFFFFU - 3U);
	const auto t_start = isix_get_jiffies();
	const auto rc = isix_vtimer_start(tmr, wrap_cb, nullptr, timeout, false);
	test_utils::cpu_busy(8);
	isix::wait_ms(150);
	const auto cnt = wrap_fire_count;
	const auto elapsed = static_cast<ostick_t>(wrap_fire_time - t_start);
	isix_vtimer_destroy(tmr);
	isix::wait_ms(5);
	TEST_ASSERT_EQUAL(ISIX_EOK, rc);
	TEST_ASSERT_EQUAL_UINT(1U, cnt);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(timeout, elapsed);
	TEST_ASSERT_LESS_THAN_UINT(timeout + test_utils::ms_ticks(30U), elapsed);
}

TEST(vtimer, destroy_null_is_einvarg)
{
	// Creating a timer starts the worker, the queue must not be reached with a null timer
	const auto tmr = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(tmr);
	TEST_ASSERT_EQUAL(ISIX_EINVARG, isix_vtimer_destroy(nullptr));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(tmr));
}

TEST(vtimer, cpp_destroy_while_due_does_not_call_back)
{
	cpp_due_count = 0;
	auto tim = std::make_unique<isix::soft_timer>([]() { cpp_due_count = cpp_due_count + 1; });
	TEST_ASSERT_TRUE(tim->is_valid());
	TEST_ASSERT_EQUAL(ISIX_EOK, tim->start_ms(5, false));
	// Worker arms the timer, then the test keeps the CPU while the timer becomes due
	isix::wait_ms(2);
	test_utils::cpu_busy(8);
	tim.reset();
	const auto after_destroy = cpp_due_count;
	isix::wait_ms(30);
	TEST_ASSERT_EQUAL_UINT(after_destroy, cpp_due_count);
}

TEST_GROUP_RUNNER(vtimer)
{
	RUN_TEST_CASE(vtimer, start_across_jiffies_wrap_not_early);
	RUN_TEST_CASE(vtimer, destroy_null_is_einvarg);
	RUN_TEST_CASE(vtimer, cpp_destroy_while_due_does_not_call_back);
	RUN_TEST_CASE(vtimer, basic);
	RUN_TEST_CASE(vtimer, isr_api);
	RUN_TEST_CASE(vtimer, one_shoot);
	RUN_TEST_CASE(vtimer, mod_api);
	RUN_TEST_CASE(vtimer, cpp11_api);
}