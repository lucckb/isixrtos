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
	isix::mutex mtx2;
	constexpr auto STK_SIZ = 256;
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

	void cpu_busy( unsigned ms_duration ) {
		const auto t1 = isix::get_jiffies();
		do {
			asm volatile("nop\n");
		} while( !isix::timer_elapsed(t1, ms_duration) );
	}

	void thread2l(void*ptr) {
		if( mtx1.lock() ) {
			std::abort();
		}
		cpu_busy(40);
		if( mtx1.unlock() ) {
			std::abort();
		}
		cpu_busy(10);
		test_buf.push_back('C');
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}
	void thread2m(void*ptr) {
		isix::wait_ms(20);
		cpu_busy(40);
		test_buf.push_back('B');
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}
	void thread2h(void*ptr) {
		isix::wait_ms(40);
		if( mtx1.lock() ) {
			std::abort();
		}
		cpu_busy(10);
		if( mtx1.unlock() ) {
			std::abort();
		}
		test_buf.push_back('A');
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}
}

namespace {

	void thread3ll( void *ptr ) {
		if( mtx1.lock() ) std::abort();
		cpu_busy(30);
		if( mtx1.unlock() ) std::abort();
		test_buf.push_back( 'E' );
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}

	void thread3l( void* ptr ) {
		isix::wait_ms(10);
		if( mtx2.lock() ) std::abort();
		cpu_busy(20);
		if( mtx1.lock() ) std::abort();
		cpu_busy(10);
		if( mtx1.unlock() ) std::abort();
		cpu_busy(10);
		if( mtx2.unlock() ) std::abort();
		test_buf.push_back( 'D' );
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}
	void thread3m( void* ptr ) {
		isix::wait_ms(20);
		if( mtx2.lock() ) std::abort();
		cpu_busy(10);
		if( mtx2.unlock() ) std::abort();
		test_buf.push_back( 'C' );
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}
	void thread3h( void* ptr ) {
		isix::wait_ms(40);
		cpu_busy(20);
		test_buf.push_back( 'B' );
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}

	void thread3hh( void* ptr ) {
		isix::wait_ms(50);
		if( mtx2.lock() ) std::abort();
		cpu_busy(10);
		if( mtx2.unlock() ) std::abort();
		test_buf.push_back( 'A' );
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}

}

namespace {

	void thread4a( void* ptr ) {
		isix::wait_ms(50);
		if( mtx1.lock() ) std::abort();
		if( mtx1.unlock() ) std::abort();
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}

	void thread4b( void* ptr ) {
		isix::wait_ms(150);
		isix::enter_critical();
		if( mtx2.lock() ) std::abort();
		if( mtx2.unlock() ) std::abort();
		isix::yield();
		isix::exit_critical();
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}
}

namespace tests {

//! Test step1
void mutexes::test01() {
	QUNIT_IS_EQUAL( mtx1.lock(), ISIX_EOK );
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread1,const_cast<char*>("A"), STK_SIZ, 5 ),nullptr );
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread1,const_cast<char*>("B"), STK_SIZ, 4 ),nullptr);
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread1,const_cast<char*>("C"), STK_SIZ, 3 ),nullptr);
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread1,const_cast<char*>("D"), STK_SIZ, 2 ),nullptr);
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread1,const_cast<char*>("E"), STK_SIZ, 1 ),nullptr);
	isix::wait_ms(100);
	QUNIT_IS_TRUE( test_buf.empty() );
	QUNIT_IS_EQUAL( mtx1.unlock(), ISIX_EOK );
	isix::wait_ms(200);
	QUNIT_IS_EQUAL( test_buf, "EDCBA" );
	test_buf.clear();
}

//! Test step 02
//  Piority inheritance basic conditions
void mutexes::test02() {
	ostick_t fin[3] {};
	const auto t1 = isix::get_jiffies();
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread2h,&fin[0],STK_SIZ, 1 ),nullptr);
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread2m,&fin[1],STK_SIZ, 2 ),nullptr);
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread2l,&fin[2],STK_SIZ, 3 ),nullptr);
	isix::wait_ms(350);
	QUNIT_IS_EQUAL( test_buf, "ABC" );
	ostick_t max {};
	for( auto v: fin ) {
		 max = std::max( max, v - t1 );
	}
	QUNIT_IS_TRUE( max >= 100 && max<110 );
	test_buf.clear();
}
/** Priority inheritance complex case
 Five threads are involved in the complex priority inversion
 * scenario, the priority inheritance algorithm is tested for depths
 * greater than one. The test expects the threads to perform their
 * operations in increasing priority order by rearranging their
 * priorities in order to avoid the priority inversion trap.
 */

void mutexes::test03() {
	ostick_t fin[5] {};
	const auto t1 = isix::get_jiffies();
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread3ll,&fin[0],STK_SIZ, 5 ),nullptr);
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread3l, &fin[1],STK_SIZ, 4 ),nullptr);
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread3m, &fin[2],STK_SIZ, 3 ),nullptr);
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread3h, &fin[3],STK_SIZ, 2 ),nullptr);
	QUNIT_IS_NOT_EQUAL( isix::task_create( thread3hh,&fin[4],STK_SIZ, 1 ),nullptr);

	isix::wait_ms(350);
	QUNIT_IS_EQUAL( test_buf, "ABCDE" );
	ostick_t max {};
	for( auto v: fin ) {
		 max = std::max( max, v - t1 );
	}
	QUNIT_IS_TRUE( max >= 100 && max<149 );
	test_buf.clear();
}

/* Two threads are spawned that try to lock the mutexes already locked
 * by the tester thread with precise timing. The test expects that the
 * priority changes caused by the priority inheritance algorithm happen
 * at the right moment and with the right values.<br> Thread A performs
 * wait(50), lock(m1), unlock(m1), exit. Thread B performs wait(150),
 * lock(m2), unlock(m2), exit.
 */
void mutexes::test04() {
	// Change current priority to minimum
	const auto old_prio = isix::task_change_prio( nullptr, isix::get_min_priority() );
	QUNIT_IS_TRUE( old_prio != isix::get_min_priority() );
	ostick_t fin[2] {};
	const auto p = isix::get_min_priority();
	const auto pa =  p - 1;
	const auto pb =  p - 2;
	if( isix::task_create( thread4a,&fin[0],STK_SIZ, pa )==nullptr) std::abort();
	if( isix::task_create( thread4b,&fin[1],STK_SIZ, pb )==nullptr) std::abort();
	/*   Locking the mutex M1 before thread A has a chance to lock
     it. The priority must not change because A has not yet reached 
     mtx1 so the mutex is not locked.*/
	QUNIT_IS_EQUAL( mtx1.lock(), ISIX_EOK );
	
	//Real prirority should be p
	QUNIT_IS_EQUAL( isix::get_task_inherited_priority(), p );
	
	//Thread A should reach the mtx1 after 100ms
	isix::wait_ms(100);
	QUNIT_IS_EQUAL( isix::get_task_inherited_priority(), pa );

	/*   Locking the mutex M2 before thread B has a chance to lock
     it. The priority must not change because B has not yet reached
     MTX2 */
	QUNIT_IS_EQUAL( mtx2.lock(), ISIX_EOK );
	QUNIT_IS_EQUAL( isix::get_task_inherited_priority(), pa );

	/* Waiting 100mS, this makes thread B reach mtx2 and
     get the mutex. This must boost the priority of the current thread
     at the same level of thread B */
	isix::wait_ms(100);
	QUNIT_IS_EQUAL( isix::get_task_inherited_priority(), pb );

	/*  Unlocking M2, the priority should fall back to P(A).*/
	QUNIT_IS_EQUAL( mtx2.unlock(), ISIX_EOK );
	QUNIT_IS_EQUAL( isix::get_task_inherited_priority(), pa );


	/*  Unlocking M1, the priority should fall back to P(0).*/
	QUNIT_IS_EQUAL( mtx1.unlock(), ISIX_EOK );
	QUNIT_IS_EQUAL( isix::get_task_inherited_priority(), p );

}

}
