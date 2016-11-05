/*
 * =====================================================================================
 *
 *       Filename:  mutex_test.cpp
 *
 *    Description:  Mutex test
 *
 *        Version:  1.0
 *        Created:  05.11.2016 15:04:08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include "mutex_test.hpp"
#include "qunit.hpp"
#include <foundation/dbglog.h>


namespace {
	std::string test_buf;
	isix::mutex mtx1;
}

namespace {

	void thread1( void* p ) {
		if( mtx1.lock() ) {
			dbprintf("Fatal! " );
			std::abort();
		}
		test_buf.push_back( *reinterpret_cast<const char*>(p) );
		if( mtx1.unlock() ) {
			dbprintf("Fatal! " );
			std::abort();
		}
	}
}

namespace tests {

//! Test step1
void mutexes::test01() {
	QUNIT_IS_EQUAL( mtx1.lock(), ISIX_EOK );
	threads.push_back( isix::task_create( thread1, const_cast<char*>("A"), 256, 3 ) );
	threads.push_back( isix::task_create( thread1, const_cast<char*>("B"), 256, 2 ) );
	threads.push_back( isix::task_create( thread1, const_cast<char*>("C"), 256, 1 ) );
	threads.push_back( isix::task_create( thread1, const_cast<char*>("D"), 256, 0 ) );
	dbprintf("TICK1");
	isix::wait_ms(100);
	dbprintf("TICK2");
	QUNIT_IS_TRUE( test_buf.empty() );
	dbprintf("TICK3");
	QUNIT_IS_EQUAL( mtx1.unlock(), ISIX_EOK );
	dbprintf("TICK4");
	isix::wait_ms(200);
	dbprintf("TICK5");
	QUNIT_IS_EQUAL( test_buf, "DCBA" );
	dbprintf("TICK6");
}


}
