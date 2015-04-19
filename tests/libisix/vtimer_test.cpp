/*
 * =====================================================================================
 *
 *       Filename:  vtimer_test.cpp
 *
 *    Description:   VTIMER test
 *
 *        Version:  1.0
 *        Created:  02.01.2014 23:02:29
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include "vtimer_test.hpp"
#include "qunit.hpp"
#include <foundation/dbglog.h>

namespace tests {

namespace {		//Delayed execution test
	
	void delegated_func( void* ptr ) {
		auto& cnt = *reinterpret_cast<int*>(ptr);
		++cnt;
	}

}

//VTIMER basic test
void vtimer::basic() 
{
	static constexpr auto wait_t = 1000U;
	static constexpr auto t1 = 100U;
	static constexpr auto t2 = 3U;
	static constexpr auto t3 = 50U;
	m_t1.clear();
	m_t2.clear();
	m_t3.clear();
	int del_exe_cnt = 0;
	QUNIT_IS_EQUAL( m_t1.start_ms(t1), ISIX_EOK );
	QUNIT_IS_EQUAL( m_t2.start_ms(t2), ISIX_EOK );
	QUNIT_IS_EQUAL( 
		isix_schedule_work_isr(delegated_func,&del_exe_cnt), 
		ISIX_EOK 
	);
	QUNIT_IS_EQUAL( m_t3.start_ms(t3), ISIX_EOK );
	QUNIT_IS_EQUAL( 
		isix_schedule_work_isr(delegated_func,&del_exe_cnt), 
		ISIX_EOK 
	);
	isix_wait_ms(wait_t);
	QUNIT_IS_EQUAL( 
		isix_schedule_work_isr(delegated_func,&del_exe_cnt), 
		ISIX_EOK 
	);
	const auto ss1 = m_t1.stop();
	const auto ss2 = m_t2.stop();
	const auto ss3 = m_t3.stop();
	isix_wait_ms(50);	//Give some time to command exec
	QUNIT_IS_EQUAL( ss1, ISIX_EOK );
	QUNIT_IS_EQUAL( ss2, ISIX_EOK );
	QUNIT_IS_EQUAL( ss3, ISIX_EOK );
	QUNIT_IS_EQUAL( m_t1.counter(), wait_t/t1 );
	QUNIT_IS_EQUAL( m_t2.counter(), wait_t/t2 );
	QUNIT_IS_EQUAL( m_t3.counter(), wait_t/t3 );
	isix_wait_ms(wait_t);
	QUNIT_IS_EQUAL( m_t1.counter(), wait_t/t1 );
	QUNIT_IS_EQUAL( m_t2.counter(), wait_t/t2 );
	QUNIT_IS_EQUAL( m_t3.counter(), wait_t/t3 );
	QUNIT_IS_EQUAL( del_exe_cnt, 3 );
}

//VTIMER basic test
void vtimer::basic_isr() 
{
	static constexpr auto wait_t = 1000U;
	static constexpr auto t1 = 100U;
	static constexpr auto t2 = 3U;
	static constexpr auto t3 = 50U;
	m_t1.clear();
	m_t2.clear();
	m_t3.clear();
	QUNIT_IS_EQUAL( m_t1.start_ms_isr(t1), ISIX_EOK );
	QUNIT_IS_EQUAL( m_t2.start_ms_isr(t2), ISIX_EOK );
	QUNIT_IS_EQUAL( m_t3.start_ms_isr(t3), ISIX_EOK );
	isix_wait_ms(wait_t);
	QUNIT_IS_EQUAL( m_t1.stop_isr(), ISIX_EOK );
	QUNIT_IS_EQUAL( m_t2.stop_isr(), ISIX_EOK );
	QUNIT_IS_EQUAL( m_t3.stop_isr(), ISIX_EOK );
	isix_wait_ms(50);	//Give some time to command exec
	QUNIT_IS_EQUAL( m_t1.counter(), wait_t/t1 );
	QUNIT_IS_EQUAL( m_t2.counter(), wait_t/t2 );
	QUNIT_IS_EQUAL( m_t3.counter(), wait_t/t3 );
	isix_wait_ms(wait_t);
	QUNIT_IS_EQUAL( m_t1.counter(), wait_t/t1 );
	QUNIT_IS_EQUAL( m_t2.counter(), wait_t/t2 );
	QUNIT_IS_EQUAL( m_t3.counter(), wait_t/t3 );
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

// VTIMER one shoot test
void vtimer::one_shoot() 
{
	auto* timerh = isix_vtimer_create();
	QUNIT_IS_TRUE( timerh != nullptr );
	//Run one shoot timer
	call_info ci;
	QUNIT_IS_EQUAL( 
			isix::vtimer_start( timerh, one_call_timer_fun, &ci, 100, false ),
			ISIX_EOK );
	isix_wait_ms( 5 );
	QUNIT_IS_EQUAL( 
			isix_vtimer_start( timerh, one_call_timer_fun, &ci, 100, false ),
			ISIX_EBUSY );
	isix_wait_ms( 1000 );
	QUNIT_IS_EQUAL( ci.count, 1 );
	QUNIT_IS_EQUAL( ci.last_call - ci.start_call, 100U );
	QUNIT_IS_EQUAL( isix_vtimer_destroy( timerh ), ISIX_EOK );
}

} //Namespace tests end

