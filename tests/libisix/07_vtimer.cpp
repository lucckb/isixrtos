#define UNITY_FIXTURE_NO_EXTRAS
#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>


namespace
{	//Delayed execution test
	void delegated_func(void* ptr) {
		auto& cnt = *reinterpret_cast<int*>(ptr);
		++cnt;
	}
	class timer : public isix::virtual_timer {
	public:
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
	constexpr auto mod_on = 2000U;
	constexpr auto mod_off = 800U;
	constexpr auto mod_iter = 20;
	struct mod_info 
	{
		int on_cnt {};
		int off_cnt {};
		int err_cnt {};
		int tot_cnt {};
		ostick_t last_call { isix_get_jiffies() };
		bool on {};
		osvtimer_t tmr;
		ossem_t fin { isix_sem_create_limited(NULL,0,1) };
	};

	inline bool mod_inrange(ostick_t t, ostick_t rng) {
		return t >= rng && t<=rng+mod_off/10;
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
TEST_TEAR_DOWN(vtimer) {}

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
	TEST_ASSERT_EQUAL_UINT(wait_t/t1, m_t1.counter());
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(wait_t/t2, m_t2.counter());
	TEST_ASSERT_LESS_THAN_UINT(wait_t/t2+2, m_t2.counter());
	TEST_ASSERT_EQUAL_UINT(wait_t/t3, m_t3.counter());
	isix_wait_ms(wait_t);
	TEST_ASSERT_EQUAL_UINT(wait_t/t1, m_t1.counter());
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(wait_t/t2, m_t2.counter());
	TEST_ASSERT_LESS_THAN_UINT(wait_t/t2+2, m_t2.counter());
	TEST_ASSERT_EQUAL_UINT(wait_t/t3, m_t3.counter());
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
	TEST_ASSERT_EQUAL_UINT(wait_t/t1, m_t1.counter());
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(wait_t/t2, m_t2.counter());
	TEST_ASSERT_LESS_THAN_UINT(wait_t/t2+2, m_t2.counter());
	TEST_ASSERT_EQUAL_UINT(wait_t/t3, m_t3.counter());
	isix_wait_ms(wait_t);
	TEST_ASSERT_EQUAL_UINT(wait_t/t1, m_t1.counter());
	TEST_ASSERT_GREATER_OR_EQUAL_UINT(wait_t/t2, m_t2.counter());
	TEST_ASSERT_LESS_THAN_UINT(wait_t/t2+2, m_t2.counter());
	TEST_ASSERT_EQUAL_UINT(wait_t/t3, m_t3.counter());
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
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::vtimer_start(timerh, one_call_timer_fun, &ci, 100, false));
	isix_wait_ms(5);
	TEST_ASSERT(isix_vtimer_is_active(timerh));
	isix_wait_ms(1000);
	TEST_ASSERT_EQUAL(1, ci.count);
	TEST_ASSERT_EQUAL(100, ci.last_call - ci.start_call);
	isix_wait_ms(200);
	TEST_ASSERT_FALSE(isix_vtimer_is_active(timerh));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(timerh));
	isix_wait_ms(50);
	isix::heap_stats(mstat);
	TEST_ASSERT_GREATER_OR_EQUAL(before_create, mstat.free);
}

TEST(vtimer, mod_api)
{
	auto* timerh = isix_vtimer_create();
	TEST_ASSERT_NOT_NULL(timerh);
	mod_info inf;
	TEST_ASSERT_NOT_NULL(inf.fin);
	inf.tmr = timerh;
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_start(timerh, cyclic_modapi_func, &inf, mod_on, true));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix::sem_wait(inf.fin, 60*1000));
	TEST_ASSERT_EQUAL(0, inf.err_cnt);
	TEST_ASSERT_EQUAL(mod_iter/2, inf.on_cnt);
	TEST_ASSERT_EQUAL(mod_iter/2, inf.off_cnt);
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_sem_destroy(inf.fin));
	TEST_ASSERT_EQUAL(ISIX_EOK, isix_vtimer_destroy(timerh));
	inf.fin = nullptr;
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


TEST_GROUP_RUNNER(vtimer)
{
	RUN_TEST_CASE(vtimer, basic);
	RUN_TEST_CASE(vtimer, isr_api);
	RUN_TEST_CASE(vtimer, one_shoot);
	RUN_TEST_CASE(vtimer, mod_api);
	RUN_TEST_CASE(vtimer, cpp11_api);
}