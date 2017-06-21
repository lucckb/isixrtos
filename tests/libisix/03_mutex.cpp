/*
 * =====================================================================================
 *
 *       Filename:  mutex_test.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  23.06.2017 18:50:38
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
#include <stm32system.h>
#include <isix/prv/list.h>
#include <isix/prv/mutex.h>


#pragma GCC diagnostic ignored "-Wstrict-aliasing"


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
	constexpr auto STK_SIZ = 2048;
}

namespace {

	void cpu_busy( unsigned ms_duration ) {
		const auto t1 = isix::get_jiffies();
		do {
			asm volatile("nop\n");
		} while( !isix::timer_elapsed(t1, ms_duration) );
	}

}

namespace test09 {
namespace {

	void thread( void* ptr ) {
		if( (mtx1.lock()) ) std::abort();
		if( mcv.wait() ) std::abort();
		test_buf.push_back( *reinterpret_cast<const char*>(ptr) );
		if( mtx1.unlock() ) std::abort();
		isix::wait_ms(100);
	}

}}

namespace test13 {
namespace {
	struct arg {
		oscondvar_t cv;
		char ch;
	};
	void thread( void* ptr ) {
		arg& a = *reinterpret_cast<arg*>(ptr);
		if( (mtx1.lock()) ) {
			std::abort();
		}
		auto ret = isix::condvar_wait(a.cv, ISIX_TIME_INFINITE);
		if( ret == ISIX_EDESTROY ) {
			test_buf.push_back( a.ch );
			return;
		}
		//NOTE: Don't unlock mtx1 it should be unlocked automaticaly
		//bug( mtx1.unlock() );
	}

}}

namespace test11 {
namespace {

	void thread_a( void* ptr ) {
		if( mtx2.lock() ) {
			std::abort();
		}
		if( mtx1.lock() ) {
			std::abort();
		}
		if( mcv.wait() ) {
			std::abort();
		}
		test_buf.push_back( *reinterpret_cast<const char*>(ptr) );
		if( mtx1.unlock() ) {
			std::abort();
		}
		if( mtx2.unlock() ) {
			std::abort();
		}
	}
	void thread_b( void* ptr ) {
		if( mtx2.lock() ) {
			std::abort();
		}
		test_buf.push_back( *reinterpret_cast<const char*>(ptr) );
		if( mtx2.unlock() ) {
			std::abort();
		}
	}
}
}

static const lest::test module[] =
{
	//T1
	CASE( "03_mutex_01 Mutex delivery order" )
	{
		constexpr auto thr1 = []( char ch )
		{
			if( mtx1.lock() ) { test_buf.push_back('Z'); return; }
			test_buf.push_back( ch );
			if( mtx1.unlock() ) { test_buf.push_back('Z'); return; }
		};
		EXPECT( mtx1.lock()== ISIX_EOK );
		auto tsk1 = isix::thread_create_and_run( STK_SIZ, 5, 0, thr1, 'A' );
		auto tsk2 = isix::thread_create_and_run( STK_SIZ, 4, 0, thr1, 'B' );
		auto tsk3 = isix::thread_create_and_run( STK_SIZ, 3, 0, thr1, 'C' );
		auto tsk4 = isix::thread_create_and_run( STK_SIZ, 2, 0, thr1, 'D' );
		auto tsk5 = isix::thread_create_and_run( STK_SIZ, 1, 0, thr1, 'E' );
		EXPECT( tsk1.is_valid() );
		EXPECT( tsk2.is_valid() );
		EXPECT( tsk3.is_valid() );
		EXPECT( tsk4.is_valid() );
		EXPECT( tsk5.is_valid() );
		isix::wait_ms(100);
		EXPECT( test_buf.empty()==true );
		EXPECT( mtx1.unlock()== ISIX_EOK );
		isix::wait_ms(200);
		EXPECT( test_buf == "EDCBA" );
		test_buf.clear();
	},
	//T2
	CASE( "03_mutex_02 Priority inheritance basic conditions" )
	{
		ostick_t fin[3] {};
		const auto thr2l = [&]() 
		{
			if( mtx1.lock() ) { test_buf.push_back('Z'); return; }
			cpu_busy(40);
			if( mtx1.unlock() ) { test_buf.push_back('Z'); return; }
			cpu_busy(10);
			test_buf.push_back('C');
			fin[2] = isix::get_jiffies();
		};
		const auto thr2m = [&]() {
			isix::wait_ms(20);
			cpu_busy(40);
			test_buf.push_back('B');
			fin[1]=isix::get_jiffies();
		};
		const auto thr2h = [&]() {
			isix::wait_ms(40);
			if( mtx1.lock() ) { test_buf.push_back('Z'); return; }
			cpu_busy(10);
			if( mtx1.unlock() ) { test_buf.push_back('Z'); return; }
			test_buf.push_back('A');
			fin[0]=isix::get_jiffies();
		};
		EXPECT( test_buf.empty() );
		const auto t1 = isix::get_jiffies();
		auto tsk1 = isix::thread_create_and_run( STK_SIZ,1,0,thr2h);
		auto tsk2 = isix::thread_create_and_run( STK_SIZ,2,0,thr2m);
		auto tsk3 = isix::thread_create_and_run( STK_SIZ,3,0,thr2l);
	    EXPECT( tsk1.is_valid() );
		EXPECT( tsk2.is_valid() );
		EXPECT( tsk3.is_valid() );
		isix::wait_ms(350);
		EXPECT( test_buf == "ABC" );
		ostick_t max {};
		for( auto v: fin ) {
			max = std::max( max, v - t1 );
		}
		EXPECT( max>=70U );
		EXPECT( max<105U );
		test_buf.clear();
		isix::wait_ms(5);
	},
	/** Priority inheritance complex case
	Five threads are involved in the complex priority inversion
	* scenario, the priority inheritance algorithm is tested for depths
	* greater than one. The test expects the threads to perform their
	* operations in increasing priority order by rearranging their
	* priorities in order to avoid the priority inversion trap.
	*/
	//T3
	CASE( "03_mutex_03 Priority inheritance complex test" )
	{
		ostick_t fin[5] {};
		const auto thr3ll = [&]()
		{
			if( mtx1.lock() ) { test_buf.push_back('Z'); return; }
			cpu_busy(30);
			if( mtx1.unlock() ) { test_buf.push_back('Z'); return; }
			test_buf.push_back( 'E' );
			fin[0]=isix::get_jiffies();
		};
		const auto thr3l = [&]()
		{
			isix::wait_ms(10);
			if( mtx2.lock() ) { test_buf.push_back('Z'); return; }
			cpu_busy(20);
			if( mtx1.lock() ) { test_buf.push_back('Z'); return; }
			cpu_busy(10);
			if( mtx1.unlock() ) { test_buf.push_back('Z'); return; }
			cpu_busy(10);
			if( mtx2.unlock() ) { test_buf.push_back('Z'); return; }
			test_buf.push_back( 'D' );
			fin[1]=isix::get_jiffies();
		};
		const auto thr3m = [&]()
		{
			isix::wait_ms(20);
			if( mtx2.lock() ) { test_buf.push_back('Z'); return; }
			cpu_busy(10);
			if( mtx2.unlock() ) { test_buf.push_back('Z'); return; }
			test_buf.push_back( 'C' );
			fin[2]=isix::get_jiffies();
		};
		const auto thr3h = [&]()
		{
			isix::wait_ms(40);
			cpu_busy(20);
			test_buf.push_back( 'B' );
			fin[3]=isix::get_jiffies();
		};
		const auto thr3hh = [&]()
		{
			isix::wait_ms(50);
			if( mtx2.lock() ) { test_buf.push_back('Z'); return; }
			cpu_busy(10);
			if( mtx2.unlock() ) { test_buf.push_back('Z'); return; }
			test_buf.push_back( 'A' );
			fin[4]=isix::get_jiffies();
		};
		const auto t1 = isix::get_jiffies();
		auto tsk1 = isix::thread_create_and_run( STK_SIZ,5,0,thr3ll );
		auto tsk2 = isix::thread_create_and_run( STK_SIZ,4,0,thr3l );
		auto tsk3 = isix::thread_create_and_run( STK_SIZ,3,0,thr3m );
		auto tsk4 = isix::thread_create_and_run( STK_SIZ,2,0,thr3h );
		auto tsk5 = isix::thread_create_and_run( STK_SIZ,1,0,thr3hh );
	    EXPECT( tsk1.is_valid() );
	    EXPECT( tsk2.is_valid() );
	    EXPECT( tsk3.is_valid() );
	    EXPECT( tsk4.is_valid() );
	    EXPECT( tsk5.is_valid() );

		isix::wait_ms(350);
		EXPECT( test_buf == "ABCDE" );
		ostick_t max {};
		for( auto v: fin ) {
			max = std::max( max, v - t1 );
		}
		EXPECT( max >= 65U );
		EXPECT( max < 110U );
		test_buf.clear();
	} ,
	/* Two threads are spawned that try to lock the mutexes already locked
	* by the tester thread with precise timing. The test expects that the
	* priority changes caused by the priority inheritance algorithm happen
	* at the right moment and with the right values.<br> Thread A performs
	* wait(50), lock(m1), unlock(m1), exit. Thread B performs wait(150),
	* lock(m2), unlock(m2), exit.
	*/
	//T4
	CASE( "03_mutex_04 Priority inheritance mutex priority values check" )
	{
		ostick_t fin[2] {};
		const auto thr4a = [&]()
		{
			isix::wait_ms(50);
			if( mtx1.lock() ) {
				return;
			}
			if( mtx1.unlock() ) {
				return;
			}
			fin[0]=isix::get_jiffies();
		};
		const auto thr4b = [&]()
		{
			isix::wait_ms(150);
			//isix::enter_critical();
			if( mtx2.lock() ) {
				return;
			}
			if( mtx2.unlock() ) {
				return;
			}
			isix::yield();
			//isix::exit_critical();
			fin[1]=isix::get_jiffies();
		};
		// Change current priority to minimum
		const auto old_prio = isix::task_change_prio( nullptr, isix::get_min_priority() );
		EXPECT( old_prio != isix::get_min_priority() );
		const auto p = isix::get_min_priority();
		const auto pa =  p - 1;
		const auto pb =  p - 2;
		auto tsk1 = isix::thread_create_and_run( STK_SIZ,pa,0,thr4a );
		auto tsk2 = isix::thread_create_and_run( STK_SIZ,pb,0,thr4b );
	    EXPECT( tsk1.is_valid() );
	    EXPECT( tsk2.is_valid() );
		/*   Locking the mutex M1 before thread A has a chance to lock
			 it. The priority must not change because A has not yet reached
			 mtx1 so the mutex is not locked.*/
		EXPECT( mtx1.lock()==ISIX_EOK );
		//Real prirority should be p
		EXPECT( isix::get_task_inherited_priority()==p );
		//Thread A should reach the mtx1 after 100ms
		isix::wait_ms(100);
		EXPECT( isix::get_task_inherited_priority()==pa );
		/*   Locking the mutex M2 before thread B has a chance to lock
			 it. The priority must not change because B has not yet reached
			 MTX2 */
		EXPECT( mtx2.lock()==ISIX_EOK );
		EXPECT( isix::get_task_inherited_priority()==pa );
		/* Waiting 100mS, this makes thread B reach mtx2 and
		   get the mutex. This must boost the priority of the current thread
		   at the same level of thread B */
		isix::wait_ms(100);
		EXPECT( isix::get_task_inherited_priority()==pb );
		/*  Unlocking M2, the priority should fall back to P(A).*/
		EXPECT( mtx2.unlock()==ISIX_EOK );
		EXPECT( isix::get_task_inherited_priority()==pa );
		/*  Unlocking M1, the priority should fall back to P(0).*/
		EXPECT( mtx1.unlock()==ISIX_EOK );
		EXPECT( isix::get_task_inherited_priority()==p );
		//Restore org prio
		EXPECT( isix::task_change_prio( nullptr, old_prio )==isix::get_min_priority() );
		//Compare priorities
		EXPECT( fin[0] >= 50U );
		EXPECT( fin[1] >= 150U );
	},
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
	//T5
	CASE( "03_mutex_05 Multiple mutex lock from same thread" )
	{
		const auto prio = isix::get_task_inherited_priority();
		EXPECT( mtx1.try_lock()==ISIX_EOK );
		//! Locking recursive mutex should be possible
		EXPECT( mtx1.try_lock()==ISIX_EOK );
		EXPECT( mtx1.unlock()==ISIX_EOK );
		//After single unlock it must be still owned
		EXPECT_NOT( ((mtx_hacker*)&mtx1)->mtx->owner==nullptr );
		EXPECT( mtx1.unlock()==ISIX_EOK );
		//After second unlock it should be owned
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->owner==nullptr );
		// Testing that priority is not changed
		EXPECT( isix::get_task_inherited_priority()==prio );
		// Test consecutive lock unlock and unlock all
		EXPECT( mtx1.try_lock()==ISIX_EOK );
		//isix_enter_critical();
		int ret = mtx1.try_lock();
		//isix_exit_critical();
		EXPECT( ret==ISIX_EOK );
		//Check recursion counter
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->count==2 );
		//isix_enter_critical();
		isix::mutex_unlock_all();
		//isix_exit_critical();
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->owner==nullptr );
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->count==0 );
		EXPECT( list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list)==true);
		// Final priority testing
		EXPECT( isix::get_task_inherited_priority()==prio );
	},
	/** Main hiph priority tasks lock mutex. Five tasks are
	 * created when mutex is released
	 *  other created tasks should get mutex in the order */
	//T6
	CASE( "03_mutex_06 High priority mutex order check" )
	{
		const auto thread = [](char ch)
		{
			if( mtx1.lock() ) { test_buf.push_back('Z'); return; }
			test_buf.push_back( ch );
			if( mtx1.unlock() ) { test_buf.push_back('Z'); return; }
		};
		test_buf.clear();
		auto thr1 = isix::thread_create_and_run( STK_SIZ,5,0,thread,'E' );
		EXPECT( thr1.is_valid() );
		auto thr2 = isix::thread_create_and_run( STK_SIZ,4,0,thread,'D' );
		EXPECT( thr2.is_valid() );
		auto thr3 = isix::thread_create_and_run( STK_SIZ,3,0,thread,'C' );
		EXPECT( thr3.is_valid() );
		auto thr4 = isix::thread_create_and_run( STK_SIZ,2,0,thread,'B' );
		EXPECT( thr3.is_valid() );
		auto thr5 = isix::thread_create_and_run( STK_SIZ,1,0,thread,'A' );
		EXPECT( thr3.is_valid() );
		isix::wait_ms(500);
		EXPECT( test_buf=="ABCDE" );
	},
	/* Five tasks take a mutex and wait for
	 * task abandon when the owner task is destroyed */
	//T7
	CASE( "03_mutex_07 Abandoned mutex when task is destroyed" )
	{
		int fin[5] { -9999, -9999, -9999, -9999, -9999 };
		const auto thread = [](int& res)
		{
			res = mtx1.lock();
			for(;;) isix_wait_ms(100);
		};
		auto thr1 = isix::thread_create_and_run( STK_SIZ,4,0,thread,std::ref(fin[0]) );
		EXPECT( thr1.is_valid() );
		isix::wait_ms(10);
		auto thr2 = isix::thread_create_and_run( STK_SIZ,5,0,thread,std::ref(fin[1]) );
		auto thr3 = isix::thread_create_and_run( STK_SIZ,5,0,thread,std::ref(fin[2]) );
		auto thr4 = isix::thread_create_and_run( STK_SIZ,5,0,thread,std::ref(fin[3]) );
		auto thr5 = isix::thread_create_and_run( STK_SIZ,5,0,thread,std::ref(fin[4]) );
		EXPECT( thr2.is_valid() );
		EXPECT( thr3.is_valid() );
		EXPECT( thr4.is_valid() );
		EXPECT( thr5.is_valid() );
		thr1.kill();
		isix::wait_ms(20);
		EXPECT( fin[0] == ISIX_EOK );
		thr2.kill();
		isix::wait_ms(20);
		EXPECT( fin[1] == ISIX_EOK );
		thr3.kill();
		isix::wait_ms(20);
		EXPECT( fin[2] == ISIX_EOK );
		thr4.kill();
		isix::wait_ms(20);
		EXPECT( fin[3] == ISIX_EOK );
		thr5.kill();
		isix::wait_ms(20);
		EXPECT( fin[4] == ISIX_EOK );
		//Final test for the mutex state
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->owner==nullptr );
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->count==0 );
		EXPECT( list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list)==true);
		isix::wait_ms(10);
	},
	/* Mutex create and destroy test
	* When tasks wait for mutexes which is
	* destroyed it should be awaked with destroyed state*/
	//T8
	CASE( "03_mutex_08 Awake tasks when mutex is destroyed" )
	{
		osmtx_t mloc = isix::mutex_create();
		EXPECT_NOT( mloc==nullptr );
		const auto thr = [&]( int& ret ) -> void
		{
			ret = isix::mutex_lock( mloc );
			for(;;) {
				isix_wait_ms(100);
			}
		};
		int retp[4] { -9999, -9999, -9999, -9999 };
		auto thr1 = isix::thread_create_and_run( STK_SIZ,4,0,thr,std::ref(retp[0]) );
		auto thr2 = isix::thread_create_and_run( STK_SIZ,4,0,thr,std::ref(retp[1]) );
		auto thr3 = isix::thread_create_and_run( STK_SIZ,4,0,thr,std::ref(retp[2]) );
		auto thr4 = isix::thread_create_and_run( STK_SIZ,4,0,thr,std::ref(retp[3]) );
		EXPECT( thr1.is_valid() );
		EXPECT( thr2.is_valid() );
		EXPECT( thr3.is_valid() );
		EXPECT( thr4.is_valid() );
		isix::wait_ms(20);
		EXPECT( isix::mutex_destroy( mloc )==ISIX_EOK );
		isix::wait_ms(10);
		EXPECT( retp[0]==ISIX_EOK );
		EXPECT( retp[1]==ISIX_EDESTROY );
		EXPECT( retp[2]==ISIX_EDESTROY );
		EXPECT( retp[3]==ISIX_EDESTROY );
		isix::wait_ms(10);
	},
	// Condition variable order test
	//T9
	CASE( "03_mutex_09 Condition variable order tests " )
	{
		using namespace test09;
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
		isix::wait_ms(200);
		EXPECT( test_buf.empty()==true );
		EXPECT( mcv.broadcast()==ISIX_EOK );
		isix::wait_ms(500);
		EXPECT( test_buf=="ABCDE" );
		// Owned mutexes should be in locked state
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->owner==nullptr );
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->count==0 );
		EXPECT( list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list)==true);
	},
	//T10
	CASE( "03_mutex_10 Condvar mutex signaling" )
	{
		using namespace test09;
		test_buf.clear();
		ostask_t t1,t2,t3,t4,t5;
		t1 = isix::task_create( thread, (char*)"E", STK_SIZ, 5 );
		if( t1 == nullptr) std::abort();
		t2 = isix::task_create( thread, (char*)"D", STK_SIZ, 4 );
		if( t2 == nullptr) std::abort();
		t3 = isix::task_create( thread, (char*)"C", STK_SIZ, 3 );
		if( t3 == nullptr) std::abort();
		t4 = isix::task_create( thread, (char*)"B", STK_SIZ, 2 );
		if( t4 == nullptr) std::abort();
		t5 = isix::task_create( thread, (char*)"A", STK_SIZ, 1 );
		if( t5 == nullptr) std::abort();
		isix::wait_ms(200);
		EXPECT( test_buf.empty()==true );
		EXPECT( mcv.signal()==ISIX_EOK );
		EXPECT( mcv.signal()==ISIX_EOK );
		EXPECT( mcv.signal()==ISIX_EOK );
		EXPECT( mcv.signal()==ISIX_EOK );
		EXPECT( mcv.signal()==ISIX_EOK );
		isix::wait_ms(500);
		EXPECT( test_buf=="ABCDE" );
	},
	//T11
	CASE( "03_mutex_11 Condtion wait priority boost" )
	{
		using namespace test11;
		test_buf.clear();
		//const auto old_prio = isix::task_change_prio( nullptr, isix::get_min_priority() );
		ostask_t t1,t2,t3;
		EXPECT(isix::get_task_inherited_priority()==isix::get_task_priority() );
		t1 = isix::task_create( thread_a, (char*)"A", STK_SIZ, 5  );
		if( t1 == nullptr) std::abort();
		t2 = isix::task_create( test09::thread, (char*)"C", STK_SIZ, 4 );
		if( t2 == nullptr) std::abort();
		t3 = isix::task_create( thread_b, (char*)"B", STK_SIZ, 3 );
		if( t3 == nullptr) std::abort();
		isix::wait_ms(100);
		mcv.signal();
		mcv.signal();
		isix::wait_ms(200);
		EXPECT( test_buf=="BAC" );
		//Restore org prio
		//EXPECT( isix::task_change_prio( nullptr, old_prio )==isix::get_min_priority() );
	},
	//T12
	CASE( "03_mutex_12 Condvar wait for timeout and not owning mutex" )
	{
		//! Mutex 1 shouldnt be aquired
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->owner==nullptr );
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->count==0 );
		EXPECT( list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list)==true);
		//! Lock the mutex and wait for timeout
		EXPECT( mtx1.lock()==ISIX_EOK );
		EXPECT( mcv.wait( 100 )==ISIX_ETIMEOUT );
		//! Mutex should be
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->owner==nullptr );
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->count==0 );
		EXPECT( list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list)==true);
		//! Locking and unlocking should be possible
		EXPECT( mtx1.try_lock()==ISIX_EOK );
		EXPECT( mtx1.unlock()==ISIX_EOK );
		//! Mutex should be empty
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->owner==nullptr );
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->count==0 );
		EXPECT( list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list)==true);
		// Wait without owning mutexes should casue error
		EXPECT( mcv.wait()==ISIX_EINVARG );
	},
	//T13
	CASE( "03_mutex_13 Condvar task destroy API" )
	{
		using namespace test13;
		test_buf.clear();
		ostask_t t1,t2,t3,t4,t5;
		oscondvar_t cv = isix::condvar_create();
		EXPECT( cv != nullptr );
		const arg a1 { cv, 'E' };
		const arg a2 { cv, 'D' };
		const arg a3 { cv, 'C' };
		const arg a4 { cv, 'B' };
		const arg a5 { cv, 'A' };
		t1 = isix::task_create( thread, (void*)&a1, STK_SIZ, 5 );
		if( t1 ==nullptr) std::abort();
		t2 = isix::task_create( thread, (void*)&a2, STK_SIZ, 4 );
		if( t2 ==nullptr) std::abort();
		t3 = isix::task_create( thread, (void*)&a3, STK_SIZ, 3 );
		if( t3 ==nullptr) std::abort();
		t4 = isix::task_create( thread, (void*)&a4, STK_SIZ, 2 );
		if( t4 ==nullptr) std::abort();
		t5 = isix::task_create( thread, (void*)&a5, STK_SIZ, 1 );
		if( t5 ==nullptr) std::abort();
		isix::wait_ms(200);
		EXPECT( test_buf.empty()==true );
		EXPECT( isix::condvar_destroy(cv)==ISIX_EOK );
		isix::wait_ms(500);
		EXPECT( test_buf=="ABCDE" );
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->owner==nullptr );
		EXPECT( ((mtx_hacker*)&mtx1)->mtx->count==0 );
		EXPECT( list_isempty(&((mtx_hacker*)&mtx1)->mtx->wait_list)==true);
	}
};


extern lest::tests & specification();
MODULE( specification(), module )
