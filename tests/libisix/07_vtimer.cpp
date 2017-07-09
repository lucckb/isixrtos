/*
 * =====================================================================================
 *
 *       Filename:  vtimer_test.cpp
 *
 *    Description:  Vtime test
 *
 *        Version:  1.0
 *        Created:  24.06.2017 16:25:09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#include <lest/lest.hpp>
#include <isix.h>


namespace
{		//Delayed execution test
	void delegated_func( void* ptr ) {
		auto& cnt = *reinterpret_cast<int*>(ptr);
		++cnt;
	}
	class timer : public isix::virtual_timer {
	public:
		//Counter get
		unsigned counter() const {
			return m_counter;
		}
		void clear() {
			m_counter = 0;
		}
		protected:
		virtual void handle_timer() noexcept
		{
			++m_counter;
		}
	private:
		unsigned m_counter {};
	};
	// Class object
}


namespace {
	//! Internal structure call info for trace call
	struct call_info {
		ostick_t last_call {};
		ostick_t start_call { isix_get_jiffies() };
		int count {};
	};
	//One shoot timer function
	void one_call_timer_fun( void *ptr ) {
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

	inline bool mod_inrange( ostick_t t, ostick_t rng ) {
		return t >= rng && t<=rng+mod_off/10;
	}

	void cyclic_modapi_func( void* ptr ) 
	{
		auto* mi = reinterpret_cast<mod_info*>(ptr);
		if( mi->tot_cnt >= mod_iter ) {
			isix_vtimer_mod( mi->tmr,OSVTIMER_CB_CANCEL );
			isix_sem_signal( mi->fin );
			return;
		} else if( mi->on ) {
			isix_vtimer_mod( mi->tmr, mod_on );
		} else if( !mi->on ) {
			isix_vtimer_mod( mi->tmr, mod_off );
		}
		auto cj1 = isix_get_jiffies();
		if( mod_inrange(cj1-mi->last_call,mod_on) ) {
			++mi->on_cnt;
		} else if( mod_inrange(cj1-mi->last_call,mod_off) ) {
			++mi->off_cnt;
		} else {
			++mi->err_cnt;
		}
		mi->on = !mi->on;
		mi->tot_cnt++;
		mi->last_call = cj1;
	}
}

static const lest::test module[] =
{
	CASE("07_vtimer_01 Vtimer basic checks")
	{
		static constexpr auto wait_t = 1000U;
		static constexpr auto t1 = 100U;
		static constexpr auto t2 = 3U;
		static constexpr auto t3 = 50U;
		timer m_t1;
		timer m_t2;
		timer m_t3;
		m_t1.clear();
		m_t2.clear();
		m_t3.clear();
		int del_exe_cnt = 0;
		EXPECT( m_t1.start_ms(t1) == ISIX_EOK );
		EXPECT( m_t2.start_ms(t2) == ISIX_EOK );
		EXPECT(
			isix_schedule_work_isr(delegated_func,&del_exe_cnt) ==
			ISIX_EOK
		);
		EXPECT( m_t3.start_ms(t3)==ISIX_EOK );
		EXPECT(
			isix_schedule_work_isr(delegated_func,&del_exe_cnt) ==
			ISIX_EOK
		);
		isix_wait_ms(wait_t);
		EXPECT(
			isix_schedule_work_isr(delegated_func,&del_exe_cnt) ==
			ISIX_EOK
		);
		const auto ss1 = m_t1.stop();
		const auto ss2 = m_t2.stop();
		const auto ss3 = m_t3.stop();
		isix_wait_ms(50);	//Give some time to command exec
		EXPECT( ss1==ISIX_EOK );
		EXPECT( ss2==ISIX_EOK );
		EXPECT( ss3==ISIX_EOK );
		EXPECT( m_t1.counter() == wait_t/t1 );
		EXPECT( m_t2.counter() >= wait_t/t2 );
		EXPECT( m_t2.counter() < wait_t/t2+2 );
		EXPECT( m_t3.counter() == wait_t/t3 );
		isix_wait_ms(wait_t);
		EXPECT( m_t1.counter() == wait_t/t1 );
		EXPECT( m_t2.counter() >= wait_t/t2 );
		EXPECT( m_t2.counter() < wait_t/t2+2 );
		EXPECT( m_t3.counter() == wait_t/t3 );
		EXPECT( del_exe_cnt==3 );
	},
	CASE("07_vtimer_02 Vtimer isr api check")
	{
		static constexpr auto wait_t = 1000U;
		static constexpr auto t1 = 100U;
		static constexpr auto t2 = 3U;
		static constexpr auto t3 = 50U;
		timer m_t1;
		timer m_t2;
		timer m_t3;
		m_t1.clear();
		m_t2.clear();
		m_t3.clear();
		EXPECT( m_t1.start_ms_isr(t1)==ISIX_EOK );
		EXPECT( m_t2.start_ms_isr(t2)==ISIX_EOK );
		EXPECT( m_t3.start_ms_isr(t3)==ISIX_EOK );
		isix_wait_ms(wait_t);
		EXPECT( m_t1.stop_isr()==ISIX_EOK );
		EXPECT( m_t2.stop_isr()==ISIX_EOK );
		EXPECT( m_t3.stop_isr()==ISIX_EOK );
		isix_wait_ms(50);	//Give some time to command exec
		EXPECT( m_t1.counter()==wait_t/t1 );
		EXPECT( m_t2.counter() >= wait_t/t2 );
		EXPECT( m_t2.counter() < wait_t/t2+2 );
		EXPECT( m_t3.counter()==wait_t/t3 );
		isix_wait_ms(wait_t);
		EXPECT( m_t1.counter()==wait_t/t1 );
		EXPECT( m_t2.counter() >= wait_t/t2 );
		EXPECT( m_t2.counter() < wait_t/t2+2 );
		EXPECT( m_t3.counter()==wait_t/t3 );
	},
	CASE("07_vtimer_03 Vtimer one shoot")
	{
		isix::memory_stat mstat;
		isix::heap_stats( mstat );
		const auto before_create = mstat.free;
		auto* timerh = isix_vtimer_create();
		EXPECT( timerh != nullptr );
		//Run one shoot timer
		call_info ci;
		EXPECT(
			isix::vtimer_start( timerh, one_call_timer_fun, &ci, 100, false )
			== ISIX_EOK
		);
		isix_wait_ms( 5 );
		EXPECT( isix_vtimer_is_active( timerh )==true );
		isix_wait_ms( 1000 );
		EXPECT( ci.count==1 );
		EXPECT( ci.last_call - ci.start_call==100U );
		isix_wait_ms( 200 );
		EXPECT( isix_vtimer_is_active( timerh )==false );
		EXPECT( isix_vtimer_destroy( timerh )==ISIX_EOK );
		isix_wait_ms(50);
		isix::heap_stats(mstat);
		EXPECT( mstat.free >= before_create );
	},
	CASE("07_vtimer_04 Vtimer mod api")
	{
		auto* timerh = isix_vtimer_create();
		EXPECT( timerh != nullptr );
		mod_info inf;
		EXPECT( inf.fin != nullptr );
		inf.tmr = timerh;
		EXPECT(
			isix_vtimer_start( timerh, cyclic_modapi_func, &inf, mod_on, true )==
			ISIX_EOK
		);
		EXPECT( isix::sem_wait( inf.fin, 60*1000 )==ISIX_EOK );
		EXPECT( inf.err_cnt == 0 );
		EXPECT( inf.on_cnt == mod_iter/2 );
		EXPECT( inf.off_cnt == mod_iter/2 );
		EXPECT( isix_sem_destroy( inf.fin ) == ISIX_EOK );
		EXPECT( isix_vtimer_destroy( timerh ) == ISIX_EOK );
		inf.fin = nullptr;
	},
	CASE("07_vtimer_05 C++11 vtimer api")
	{
		int counter = 0;
		auto fn = [&]() -> void
		{
			++counter;
		};
		auto tim = isix::vtimer_create( fn );
		EXPECT( tim.start_ms(10) == ISIX_EOK );
		isix::wait_ms(250);
		EXPECT( tim.stop() == ISIX_EOK );
		isix::wait_ms(25);
		EXPECT( counter == 25 );
	}
};

extern lest::tests & specification();
MODULE( specification(), module )
