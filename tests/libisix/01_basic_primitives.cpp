/*
 * =====================================================================================
 *
 *       Filename:  heap_test.cpp
 *
 *    Description:  BASIC heap test
 *
 *        Version:  1.0
 *        Created:  22.06.2017 19:04:45
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
#include <isix/arch/sem_atomic.h>

static const lest::test module[] =
{
	CASE( "01_base_01 Basic heap allocator" )
	{
		EXPECT( reinterpret_cast<long>(isix::task_self())%4 == 0 );
		auto ptr1 = isix_alloc( 1 );
		auto ptr2 = isix_alloc( 1 );
		EXPECT_NOT( ptr1==nullptr );
		EXPECT( reinterpret_cast<long>(ptr1)%ISIX_BYTE_ALIGNMENT_SIZE == 0U );
		EXPECT_NOT( ptr2 == nullptr );
		EXPECT( reinterpret_cast<long>(ptr2)%ISIX_BYTE_ALIGNMENT_SIZE == 0U );
		if(ptr1) isix_free( ptr1 );
		if(ptr2) isix_free( ptr2 );
		isix::memory_stat mstat;
		isix::heap_stats( mstat );
		EXPECT( mstat.free > 0U );
	},
	CASE( "01_base_02 Atomic semaphore primitives test" )
	{
	_isix_port_atomic_sem_t sem;
	_isix_port_atomic_sem_init( &sem , 1, sys_atomic_unlimited_value );
	//Basic aritmetic tests
	EXPECT( sem.value == 1 );
	EXPECT( _isix_port_atomic_sem_inc( &sem ) == 2 );
	EXPECT( sem.value == 2 );
	EXPECT( _isix_port_atomic_sem_inc( &sem ) == 3 );
	EXPECT( sem.value == 3 );
	EXPECT( _isix_port_atomic_sem_dec( &sem ) == 2 );
	EXPECT( sem.value == 2 );
	_isix_port_atomic_sem_dec( &sem );
	EXPECT( sem.value == 1 );
	EXPECT( _isix_port_atomic_sem_dec( &sem ) == 0 );
	EXPECT( _isix_port_atomic_sem_dec( &sem ) == -1 );
	EXPECT( sem.value == -1 );
	EXPECT( _isix_port_atomic_sem_dec( &sem ) == -2 );
	EXPECT( sem.value == -2 );
	// Testing try wait
	EXPECT( _isix_port_atomic_sem_trydec( &sem ) == -2 );
	EXPECT( sem.value == -2 );
	//UP to 0
	EXPECT( _isix_port_atomic_sem_inc( &sem ) == -1 );
	EXPECT( _isix_port_atomic_sem_inc( &sem ) == 0 );
	//Testing on 0 level
	EXPECT( _isix_port_atomic_sem_trydec( &sem ) == 0);
	EXPECT( sem.value == 0 );
	//Testing on 1 level should be changed to 0
	EXPECT( _isix_port_atomic_sem_inc( &sem ) == 1 );
	//Testing on 1 level should be 0 again
	EXPECT( _isix_port_atomic_sem_trydec( &sem ) == 1 );
	EXPECT( sem.value == 0 );
	//Testing for upper limit value
	_isix_port_atomic_sem_t lsem;
	_isix_port_atomic_sem_init( &lsem, 0, 3 );
	EXPECT( _isix_port_atomic_sem_inc( &lsem ) == 1 );
	EXPECT( lsem.value == 1 );
	EXPECT( _isix_port_atomic_sem_inc( &lsem ) == 2 );
	EXPECT( lsem.value == 2 );
	EXPECT( _isix_port_atomic_sem_inc( &lsem ) == 3 );
	EXPECT( lsem.value == 3 );
	EXPECT( _isix_port_atomic_sem_inc( &lsem ) == 3 );
	EXPECT( lsem.value == 3 );
	EXPECT( _isix_port_atomic_sem_inc( &lsem ) == 3 );
	EXPECT( lsem.value == 3 );
	EXPECT( _isix_port_atomic_sem_dec( &lsem ) == 2 );
	EXPECT( lsem.value == 2 );
	}
};


extern lest::tests & specification();
MODULE( specification(), module )




