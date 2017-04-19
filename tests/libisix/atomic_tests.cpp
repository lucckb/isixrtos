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
#include <isix/arch/sem_atomic.h>
#include "atomic_tests.hpp"
#include "qunit.hpp"
#include "timer_interrupt.hpp"

namespace tests {


//Atomic semaphore primitives test
void atomic_tests::atomic_sem_test( )  
{
	_isix_port_atomic_sem_t sem;
	_isix_port_atomic_sem_init( &sem , 1, sys_atomic_unlimited_value );
	//Basic aritmetic tests
	QUNIT_IS_EQUAL( sem.value, 1 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &sem ), 2 );
	QUNIT_IS_EQUAL( sem.value, 2 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &sem ), 3 );
	QUNIT_IS_EQUAL( sem.value, 3 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_dec( &sem ), 2 );
	QUNIT_IS_EQUAL( sem.value, 2 );
	_isix_port_atomic_sem_dec( &sem );
	QUNIT_IS_EQUAL( sem.value, 1 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_dec( &sem ), 0 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_dec( &sem ), -1 );
	QUNIT_IS_EQUAL( sem.value, -1 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_dec( &sem ), -2 );
	QUNIT_IS_EQUAL( sem.value, -2 );
	// Testing try wait
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_trydec( &sem ), -2 );
	QUNIT_IS_EQUAL( sem.value, -2 );
	//UP to 0
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &sem ), -1 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &sem ), 0 );
	//Testing on 0 level
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_trydec( &sem ), 0);
	QUNIT_IS_EQUAL( sem.value, 0 );
	//Testing on 1 level should be changed to 0
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &sem ), 1 );
	//Testing on 1 level should be 0 again
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_trydec( &sem ), 1 );
	QUNIT_IS_EQUAL( sem.value, 0 );
	
	//Testing for upper limit value
	_isix_port_atomic_sem_t lsem;
	_isix_port_atomic_sem_init( &lsem, 0, 3 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &lsem ), 1 );
	QUNIT_IS_EQUAL( lsem.value, 1 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &lsem ), 2 );
	QUNIT_IS_EQUAL( lsem.value, 2 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &lsem ), 3 );
	QUNIT_IS_EQUAL( lsem.value, 3 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &lsem ), 3 );
	QUNIT_IS_EQUAL( lsem.value, 3 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_inc( &lsem ), 3 );
	QUNIT_IS_EQUAL( lsem.value, 3 );
	QUNIT_IS_EQUAL( _isix_port_atomic_sem_dec( &lsem ), 2 );
	QUNIT_IS_EQUAL( lsem.value, 2 );
}


} // Namespace tests end
