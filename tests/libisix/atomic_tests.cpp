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
#include <asm/atomic_sem.h>
#include <asm/atomic.h>
#include "atomic_tests.hpp"
#include "qunit.hpp"
#include "timer_interrupt.hpp"

namespace tests {


//Atomic semaphore primitives test
void atomic_tests::atomic_sem_test( )  
{
	using namespace sys;
	sys_atomic_sem_lock_t sem;
	sys_atomic_sem_init( &sem , 1, sys_atomic_unlimited_value );
	//Basic aritmetic tests
	QUNIT_IS_EQUAL( sem.value, 1 );
	QUNIT_IS_EQUAL( sys_atomic_sem_inc( &sem ), 2 );
	QUNIT_IS_EQUAL( sem.value, 2 );
	QUNIT_IS_EQUAL( sys_atomic_sem_inc( &sem ), 3 );
	QUNIT_IS_EQUAL( sem.value, 3 );
	QUNIT_IS_EQUAL( sys_atomic_sem_dec( &sem ), 3 );
	QUNIT_IS_EQUAL( sem.value, 2 );
	sys_atomic_sem_dec( &sem );
	QUNIT_IS_EQUAL( sem.value, 1 );
	QUNIT_IS_EQUAL( sys_atomic_sem_dec( &sem ), 1 );
	QUNIT_IS_EQUAL( sys_atomic_sem_dec( &sem ), 0 );
	QUNIT_IS_EQUAL( sem.value, 0 );
	QUNIT_IS_EQUAL( sys_atomic_sem_dec( &sem ), 0 );
	QUNIT_IS_EQUAL( sem.value, 0 );
	//Testing for upper limit value
	sys_atomic_sem_lock_t lsem;
	sys_atomic_sem_init( &lsem, 0, 3 );
	QUNIT_IS_EQUAL( sys_atomic_sem_inc( &lsem ), 1 );
	QUNIT_IS_EQUAL( lsem.value, 1 );
	QUNIT_IS_EQUAL( sys_atomic_sem_inc( &lsem ), 2 );
	QUNIT_IS_EQUAL( lsem.value, 2 );
	QUNIT_IS_EQUAL( sys_atomic_sem_inc( &lsem ), 3 );
	QUNIT_IS_EQUAL( lsem.value, 3 );
	QUNIT_IS_EQUAL( sys_atomic_sem_inc( &lsem ), 3 );
	QUNIT_IS_EQUAL( lsem.value, 3 );
	QUNIT_IS_EQUAL( sys_atomic_sem_inc( &lsem ), 3 );
	QUNIT_IS_EQUAL( lsem.value, 3 );
	QUNIT_IS_EQUAL( sys_atomic_sem_dec( &lsem ), 3 );
	QUNIT_IS_EQUAL( lsem.value, 2 );
}

//Atomic access 8bit test
void atomic_tests::atomic_access_8() 
{
	using namespace sys;
	int8_t s = -100;
	QUNIT_IS_EQUAL( sys_atomic_read_int8_t(&s), -100 );
	QUNIT_IS_EQUAL( sys_atomic_write_int8_t(&s, -5 ) , -100 );
	QUNIT_IS_EQUAL( sys_atomic_read_int8_t(&s), -5 );
	QUNIT_IS_EQUAL( sys_atomic_try_write_int8_t(&s, -30), 0 );
	QUNIT_IS_EQUAL( sys_atomic_read_int8_t(&s), -30 );
	uint8_t u = 5;
	QUNIT_IS_EQUAL( sys_atomic_read_uint8_t(&u), 5 );
	QUNIT_IS_EQUAL( sys_atomic_write_uint8_t(&u, 150 ) , 5 );
	QUNIT_IS_EQUAL( sys_atomic_read_uint8_t(&u), 150 );
	QUNIT_IS_EQUAL( sys_atomic_try_write_uint8_t(&u, 220), 0 );
	QUNIT_IS_EQUAL( sys_atomic_read_uint8_t(&u), 220 );
}

//Atomic access 8bit test
void atomic_tests::atomic_access_16() 
{
	using namespace sys;
	int16_t s = -1000;
	QUNIT_IS_EQUAL( sys_atomic_read_int16_t(&s), -1000 );
	QUNIT_IS_EQUAL( sys_atomic_write_int16_t(&s, -500 ) , -1000 );
	QUNIT_IS_EQUAL( sys_atomic_read_int16_t(&s), -500 );
	QUNIT_IS_EQUAL( sys_atomic_try_write_int16_t(&s, -300), 0 );
	QUNIT_IS_EQUAL( sys_atomic_read_int16_t(&s), -300 );
	uint16_t u = 5000;
	QUNIT_IS_EQUAL( sys_atomic_read_uint16_t(&u), 5000 );
	QUNIT_IS_EQUAL( sys_atomic_write_uint16_t(&u, 1500 ) , 5000 );
	QUNIT_IS_EQUAL( sys_atomic_read_uint16_t(&u), 1500 );
	QUNIT_IS_EQUAL( sys_atomic_try_write_uint16_t(&u, 22000), 0 );
	QUNIT_IS_EQUAL( sys_atomic_read_uint16_t(&u), 22000 );
}

void atomic_tests::atomic_access_32() 
{
	using namespace sys;
	int32_t s = -100000;
	QUNIT_IS_EQUAL( sys_atomic_read_int32_t(&s), -100000 );
	QUNIT_IS_EQUAL( sys_atomic_write_int32_t(&s, -500000 ) , -100000 );
	QUNIT_IS_EQUAL( sys_atomic_read_int32_t(&s), -500000);
	QUNIT_IS_EQUAL( sys_atomic_try_write_int32_t(&s, -3000000), 0 );
	QUNIT_IS_EQUAL( sys_atomic_read_int32_t(&s), -3000000 );
	uint32_t u = 0xEEEE1000;
	QUNIT_IS_EQUAL( sys_atomic_read_uint32_t(&u), uint32_t(0xEEEE1000) );
	QUNIT_IS_EQUAL( sys_atomic_write_uint32_t(&u, 0xABCDABCD ) , uint32_t(0xEEEE1000) );
	QUNIT_IS_EQUAL( sys_atomic_read_uint32_t(&u), uint32_t(0xABCDABCD) );
	QUNIT_IS_EQUAL( sys_atomic_try_write_uint32_t(&u, 0x19790822), 0 );
	QUNIT_IS_EQUAL( sys_atomic_read_uint32_t(&u), 0x19790822);
}

} // Namespace tests end
