/*
 * =====================================================================================
 *
 *       Filename:  atomic_tests.cpp
 *
 *    Description:  Atomics operation tests
 *
 *        Version:  1.0
 *        Created:  16.12.2013 18:13:48
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <asm/atomic_ops.h>
#include "atomic_tests.hpp"
#include "qunit.hpp"
#include "timer_interrupt.hpp"

namespace tests {

//From interrupt context test
namespace {

}

//Atomic semaphore primitives test
void atomic_tests::atomic_sem_test( )  
{
	using namespace sys;
	sys_atomic_sem_lock_t sem;
	sys_atomic_sem_init( &sem , 1 );
	//Basic aritmetic tests
	QUNIT_IS_EQUAL( sem.value, 1 );
	QUNIT_IS_EQUAL( sys_atomic_try_sem_inc( &sem ), 2 );
	QUNIT_IS_EQUAL( sem.value, 2 );
	QUNIT_IS_EQUAL( sys_atomic_try_sem_inc( &sem ), 3 );
	QUNIT_IS_EQUAL( sem.value, 3 );
	QUNIT_IS_EQUAL( sys_atomic_try_sem_dec( &sem ), 2 );
	QUNIT_IS_EQUAL( sem.value, 2 );
	sys_atomic_try_sem_dec( &sem );
	QUNIT_IS_EQUAL( sem.value, 1 );
	QUNIT_IS_EQUAL( sys_atomic_try_sem_dec( &sem ), 0 );
	QUNIT_IS_EQUAL( sys_atomic_try_sem_dec( &sem ), 0 );
	QUNIT_IS_EQUAL( sem.value, 0 );
	QUNIT_IS_EQUAL( sys_atomic_try_sem_dec( &sem ), 0 );
	QUNIT_IS_EQUAL( sem.value, 0 );
}

} // Namespace tests end
