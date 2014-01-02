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
	isix::isix_wait_ms(100);
	m_t1.stop();
	m_t2.stop();
	m_t3.stop();
	QUNIT_IS_EQUAL(m_t1.counter(), 100/10 );
	QUNIT_IS_EQUAL(m_t2.counter(), 100/1 );
	QUNIT_IS_EQUAL(m_t3.counter(), 100/5 );
	isix::isix_wait_ms(100);
	QUNIT_IS_EQUAL(m_t1.counter(), 100/10 );
	QUNIT_IS_EQUAL(m_t2.counter(), 100/1 );
	QUNIT_IS_EQUAL(m_t3.counter(), 100/5 );
}

} //Namespace tests end

