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
/*
 * No native support for _Atomic(). Place object in structure to prevent
 * most forms of direct non-atomic access.
 */


#include "mutex_test.hpp"
#include "qunit.hpp"
#include <foundation/dbglog.h>
#include <isix/prv/list.h>
#include <isix/prv/mutex.h>




//Temporary private data access mutex stuff
// Structure of isix mutex
//TODO condvars

struct mtx_hacker {
	osmtx_t mtx;
};

namespace {
	std::string test_buf;
	isix::mutex mtx1;
	isix::mutex mtx2;
	isix::condvar mcv;
	constexpr auto STK_SIZ = 512;
}

#define bug(code) do { if(code) { dbprintf("ABORT FILE %s LINE %i CODE %i",__FILE__,__LINE__,code); std::abort(); } } while(0)

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
		bug( mtx1.lock() );
		bug( mtx1.unlock() );
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}

	void thread4b( void* ptr ) {
		isix::wait_ms(150);
		//isix::enter_critical();
		bug( mtx2.lock() );
		bug( mtx2.unlock() );
		isix::yield();
		//isix::exit_critical();
		*reinterpret_cast<ostick_t*>(ptr)=isix::get_jiffies();
	}
}

namespace test06 {
namespace {
	void thread( void* p) {
		if( mtx1.lock() ) std::abort();
		test_buf.push_back( *reinterpret_cast<const char*>(p) );
		if( mtx1.unlock() ) std::abort();
	}
}}

namespace test07 {
namespace {

	void thread( void* p ) {
		*reinterpret_cast<int*>(p) = mtx1.lock();
		for(;;) isix_wait_ms(100);
	}

}}

namespace test08 {
namespace {

	struct params {
		osmtx_t mtx;
		volatile int ret;
	};
	void thread( void* ptr ) {
		params& p = *reinterpret_cast<params*>(ptr);
		p.ret = isix::mutex_lock( p.mtx );
		for(;;) {
			isix_wait_ms(100);
		}
	}

}}

namespace test09 {
namespace {

	void thread( void* ptr ) {
		if( mtx1.lock() ) std::abort();
		if( mcv.wait() ) std::abort();
		test_buf.push_back( *reinterpret_cast<const char*>(ptr) );
		if( mtx1.unlock() ) std::abort();
	}

}}

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

	//Restore org prio
	QUNIT_IS_EQUAL( isix::task_change_prio( nullptr, old_prio ), isix::get_min_priority() );

}



/* The behavior of multiple mutex locks from the same thread is tested
  * Getting current thread priority for later checks.
 * Locking the mutex first time, it must be possible because it is not owned.
 * Locking the mutex second time, it must be possible because it is recursive.
 * Unlocking the mutex then it must be still owned because recursivity.
 * Unlocking the mutex then it must not be owned anymore and the queue must be empty.
 * Testing that priority has not changed after operations.
 * Testing consecutive try_lock calls and a final unlock_all()
 * Testing consecutive lock/unlock calls and a  final unlock_all().
 * Testing that priority has not changed after operations.
 */

void mutexes::test05() {
	const auto prio = isix::get_task_inherited_priority();
	QUNIT_IS_EQUAL( mtx1.try_lock(), ISIX_EOK );
	//! Locking recursive mutex should be possible
	QUNIT_IS_EQUAL( mtx1.try_lock(), ISIX_EOK );

	QUNIT_IS_EQUAL( mtx1.unlock(), ISIX_EOK );
	//After single unlock it must be still owned
	QUNIT_IS_NOT_EQUAL( ((mtx_hacker*)&mtx1)->mtx->owner, nullptr );

	QUNIT_IS_EQUAL( mtx1.unlock(), ISIX_EOK );
	//After second unlock it should be owned
	QUNIT_IS_EQUAL( ((mtx_hacker*)&mtx1)->mtx->owner, nullptr );

	// Testing that priority is not changed
	QUNIT_IS_EQUAL( isix::get_task_inherited_priority(), prio );

	// Test consecutive lock unlock and unlock all
	QUNIT_IS_EQUAL( mtx1.try_lock(), ISIX_EOK );
	//isix_enter_critical();
	int ret = mtx1.try_lock();
	//isix_exit_critical();
	QUNIT_IS_EQUAL( ret, ISIX_EOK );
	//Check recursion counter
	QUNIT_IS_EQUAL( ((mtx_hacker*)&mtx1)->mtx->count, 2 );
	//isix_enter_critical();
	isix::mutex_unlock_all();
	//isix_exit_critical();
	QUNIT_IS_EQUAL( ((mtx_hacker*)&mtx1)->mtx->owner, nullptr );
	QUNIT_IS_EQUAL( ((mtx_hacker*)&mtx1)->mtx->count, 0 );
	QUNIT_IS_TRUE( list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
	// Final priority testing
	QUNIT_IS_EQUAL( isix::get_task_inherited_priority(), prio );
}

/** Main hiph priority tasks lock mutex. Five tasks are created when mutex is released
 *  other created tasks should get mutex in the order */
void mutexes::test06() {

	using namespace test06;
	test_buf.clear();
	ostask_t t1,t2,t3,t4,t5;
	t1 = isix::task_create( thread, (char*)"E", STK_SIZ, 5 );
	if( t1 ==nullptr) std::abort();
	t2 = isix::task_create( thread, (char*)"D", STK_SIZ, 4 );
	if( t2 ==nullptr) std::abort();
	t3 = isix::task_create( thread, (char*)"C", STK_SIZ, 3 );
	if( t3 ==nullptr) std::abort();
	t4 = isix::task_create( thread, (char*)"B", STK_SIZ, 2 );
	if( t4 ==nullptr) std::abort();
	t5 = isix::task_create( thread, (char*)"A", STK_SIZ, 1 );
	if( t5 ==nullptr) std::abort();
	isix::wait_ms(500);
	QUNIT_IS_EQUAL( test_buf, "ABCDE" );
}


/* Five tasks take a mutex and wait for task abandon when the owner task is destroyed */
void mutexes::test07()  {

	using namespace test07;
	int fin[5] { -9999, -9999, -9999, -9999, -9999 };
	ostask_t t[5];
	if( (t[0]=isix::task_create( thread,&fin[0],STK_SIZ, 4 ))==nullptr) std::abort();
	isix::wait_ms(10);
	if( (t[1]=isix::task_create( thread,&fin[1],STK_SIZ, 5 ))==nullptr) std::abort();
	if( (t[2]=isix::task_create( thread,&fin[2],STK_SIZ, 5 ))==nullptr) std::abort();
	if( (t[3]=isix::task_create( thread,&fin[3],STK_SIZ, 5 ))==nullptr) std::abort();
	if( (t[4]=isix::task_create( thread,&fin[4],STK_SIZ, 5 ))==nullptr) std::abort();
	isix::wait_ms(20);
	int match[6] { -9999, -9999, -9999, -9999, -9999 };
	for( int i=0;i<5;i++ ) {
		dbprintf(">>>>>> Test loop %i <<<<<<< ", i );
		isix::task_kill( t[i] );
		isix::wait_ms(20);
		match[i] = ISIX_EOK;
		match[i+1] = ISIX_EOK;
		for( int j=0; j<5; j++ ) {
			QUNIT_IS_EQUAL( fin[j], match[j] );
		}
	}
	//Final test for the mutex state
	QUNIT_IS_EQUAL( ((mtx_hacker*)&mtx1)->mtx->owner, nullptr );
	QUNIT_IS_EQUAL( ((mtx_hacker*)&mtx1)->mtx->count, 0 );
	QUNIT_IS_TRUE( list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list));
	isix::wait_ms(10);
}


/* Mutex create and destroy test
 * When tasks wait for mutexes which is destroyed it should be awaked with edestroyed state*/
void mutexes::test08() {
	using namespace test08;
	osmtx_t mloc = isix::mutex_create();
	QUNIT_IS_NOT_EQUAL( mloc, nullptr );
	params par[4]{ {mloc,-9999},{mloc,-9999}, {mloc,-9999}, {mloc,-9999} };
	ostask_t t1,t2,t3,t4;
	if( (t1=isix::task_create( thread ,&par[0],STK_SIZ, 4 ))==nullptr) std::abort();
	if( (t2=isix::task_create( thread ,&par[1],STK_SIZ, 4 ))==nullptr) std::abort();
	if( (t3=isix::task_create( thread ,&par[2],STK_SIZ, 4 ))==nullptr) std::abort();
	if( (t4=isix::task_create( thread ,&par[3],STK_SIZ, 4 ))==nullptr) std::abort();
	isix::wait_ms(20);
	QUNIT_IS_EQUAL( isix::mutex_destroy( mloc ), ISIX_EOK );
	isix::wait_ms(10);
	QUNIT_IS_EQUAL( par[0].ret, ISIX_EOK );
	QUNIT_IS_EQUAL( par[1].ret, ISIX_EDESTROY );
	QUNIT_IS_EQUAL( par[2].ret, ISIX_EDESTROY );
	QUNIT_IS_EQUAL( par[3].ret, ISIX_EDESTROY );
	isix::task_kill(t1);
	isix::task_kill(t2);
	isix::task_kill(t3);
	isix::task_kill(t4);
	isix::wait_ms(10);
}

/** Condvar basic test  */
void mutexes::test09() {
	using namespace test09;
	dbprintf("Test09 begin");
	test_buf.clear();
	ostask_t t1,t2,t3,t4,t5;
	t1 = isix::task_create( thread, (char*)"E", STK_SIZ, 5 );
	if( t1 ==nullptr) std::abort();
	t2 = isix::task_create( thread, (char*)"D", STK_SIZ, 4 );
	if( t2 ==nullptr) std::abort();
	t3 = isix::task_create( thread, (char*)"C", STK_SIZ, 3 );
	if( t3 ==nullptr) std::abort();
	t4 = isix::task_create( thread, (char*)"B", STK_SIZ, 2 );
	if( t4 ==nullptr) std::abort();
	t5 = isix::task_create( thread, (char*)"A", STK_SIZ, 1 );
	if( t5 ==nullptr) std::abort();
	isix::wait_ms(500);
	QUNIT_IS_EQUAL( test_buf, "ABCDE" );
}

}
