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

namespace tests {

//VTIMER basic test
void vtimer::basic() 
{
	m_t1.start_ms(10);
	m_t2.start_ms(1);
	m_t3.start_ms(5);
	isix_wait_ms(100);
	m_t1.stop();
	m_t2.stop();
	m_t3.stop();
	QUNIT_IS_EQUAL(m_t1.counter(), 100U/10U );
	QUNIT_IS_EQUAL(m_t2.counter(), 100U/1U );
	QUNIT_IS_EQUAL(m_t3.counter(), 100U/5U );
	isix_wait_ms(100);
	QUNIT_IS_EQUAL(m_t1.counter(), 100U/10U );
	QUNIT_IS_EQUAL(m_t2.counter(), 100U );
	QUNIT_IS_EQUAL(m_t3.counter(), 100U/5U );
}

namespace {
	//! Internal structure call info for trace call
	struct call_info {
		tick_t last_call {};
		tick_t start_call { isix_get_jiffies() };
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
	auto* timerh = isix_vtimer_create_oneshoot();
	QUNIT_IS_TRUE( timerh != nullptr );
	//Check negative to use normal timer here
	QUNIT_IS_EQUAL( isix_vtimer_start(timerh, 100 ), ISIX_EINVARG );
	//Run one shoot timer
	call_info ci;
	QUNIT_IS_EQUAL( 
			isix_vtimer_one_shoot( timerh, one_call_timer_fun, &ci, 100 ),
			ISIX_EOK );
	isix_wait_ms( 5 );
	QUNIT_IS_EQUAL( 
			isix_vtimer_one_shoot( timerh, one_call_timer_fun, &ci, 100 ),
			ISIX_EBUSY );
	isix_wait_ms( 200 );
	QUNIT_IS_EQUAL( ci.count, 1 );
	QUNIT_IS_EQUAL( ci.last_call - ci.start_call, 100U );
	QUNIT_IS_EQUAL( isix_vtimer_destroy( timerh ), ISIX_EOK );
}

} //Namespace tests end

