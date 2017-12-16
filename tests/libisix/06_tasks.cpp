/*
 * =====================================================================================
 *
 *       Filename:  task_tests.cpp
 *
 *    Description:  Tasks API test
 *
 *        Version:  1.0
 *        Created:  23.06.2017 20:03:36
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
#include "task_test_helper.h"
#include "utils/fpu_test_and_set.h"
#include "utils/timer_interrupt.hpp"
#include <foundation/sys/dbglog.h>

namespace
{
	static const auto BASE_TASK_PRIO = 1;
	//Test basic task functionality
	class base_task_tests {
		static constexpr auto STACK_SIZE = 1024;
		volatile unsigned m_exec_count {};
		volatile bool m_req_selfsusp {};
		//Main function
		void thread() noexcept
		{
			for(;;) {
				++m_exec_count;
				if( m_req_selfsusp ) {
					m_req_selfsusp = false;
					isix::task_suspend(nullptr);
				}
			}
		}
	public:
		base_task_tests()
			: m_thr( isix::thread_create( std::bind(&base_task_tests::thread,std::ref(*this))))
		{
		}
		base_task_tests( base_task_tests& ) = delete;
		base_task_tests& operator=( base_task_tests& ) = delete;
		void start() {
			m_thr.start_thread(STACK_SIZE, BASE_TASK_PRIO);
		}
		void selfsuspend() {
			m_req_selfsusp = true;
		}
		unsigned exec_count() const {
			return m_exec_count;
		}
		void exec_count( unsigned v ) {
			m_exec_count = v;
		}
		auto tid() const noexcept {
			return m_thr.tid();
		}
	private:
		isix::thread m_thr;
	};

	namespace thr11 {
		bool fin2 = false;
		void thread2_func()
		{
			fin2 = false;
			for( int i=0;i<5;++i ) {
				isix::wait_ms( 100 );
			}
			fin2 = true;
		}
	}
	namespace waitref  {
		void task_ref(void* ) {
			for( int i=0;i<5;++i ) {
				isix::wait_ms( 100 );
			}
		}
		void task_ref2(void* arg )  {
			ostask_t t = reinterpret_cast<ostask_t>(arg);
			if( isix::task_wait_for( t ) != ISIX_EOK ) std::abort();
			isix::wait_ms(50);
		}
	}
}

namespace
{
	constexpr auto c_stack_size = ISIX_MIN_STACK_SIZE*2;
	constexpr auto c_task_prio = 3;
	constexpr auto c_stack_margin = 100;
}

const lest::test module[] =
{
	CASE( "06_tasks_01 Basic api for tasks" )
	{
		static constexpr auto MIN_STACK_FREE = 64U;
		//Check if scheduler is running
		EXPECT( isix_is_scheduler_active()==true);
		auto t1 = new base_task_tests;
		auto t2 = new base_task_tests;
		auto t3 = new base_task_tests;
		auto t4 = new base_task_tests;
		t1->start(); t2->start(); t3->start(); t4->start();
		//Active wait tasks shouldnt run
		for(auto tc = isix_get_jiffies(); isix_get_jiffies()<tc+5000; ) {
			asm volatile("nop\n");
		}
		EXPECT( t1->exec_count()==0U);
		EXPECT( t2->exec_count()==0U);
		EXPECT( t3->exec_count()==0U);
		EXPECT( t4->exec_count()==0U);
		//Now goto sleep
		isix_wait_ms( 5000 );
		//TASK should run now
		EXPECT( t1->exec_count()>0U );
		EXPECT( t2->exec_count()>0U );
		EXPECT( t3->exec_count()>0U );
		EXPECT( t4->exec_count()>0U );
		//Zero task count.. change prio and go active wait
		t1->exec_count(0);
		t2->exec_count(0);
		t3->exec_count(0);
		t4->exec_count(0);
		EXPECT( t1->exec_count()==0U );
		EXPECT( t2->exec_count()==0U);
		EXPECT( t3->exec_count()==0U );
		EXPECT( t4->exec_count()==0U );
		EXPECT( isix_task_change_prio(t1->tid(),0)==BASE_TASK_PRIO );
		EXPECT( isix_task_change_prio(t2->tid(),0)==BASE_TASK_PRIO );
		EXPECT( isix_task_change_prio(t3->tid(),0)==BASE_TASK_PRIO );
		EXPECT( isix_task_change_prio(t4->tid(),0)==BASE_TASK_PRIO );
		//Active wait tasks should doesn't run
		for( auto tc = isix_get_jiffies(); isix_get_jiffies()<tc+5000; ) {
			asm volatile("nop\n");
		}
		//TASK should run now
		EXPECT( t1->exec_count()>0U );
		EXPECT( t4->exec_count()>0U );
		//Validate stack space functionality
		EXPECT( isix_free_stack_space(t1->tid()) > ssize_t(MIN_STACK_FREE)  );
		EXPECT( isix_free_stack_space(t2->tid()) > ssize_t(MIN_STACK_FREE)  );
		EXPECT( isix_free_stack_space(t3->tid()) > ssize_t(MIN_STACK_FREE)  );
		EXPECT( isix_free_stack_space(t4->tid()) > ssize_t(MIN_STACK_FREE)  );
		EXPECT( isix_free_stack_space(nullptr) > ssize_t(MIN_STACK_FREE)  );
		//! Get task state should be ready or running
		auto state = isix::get_task_state( t1->tid() );
		EXPECT( (state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING)==true );
		state = isix::get_task_state( t2->tid() );
		EXPECT( (state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING)==true);
		state = isix::get_task_state( t3->tid() );
		EXPECT( (state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING)==true );
		state = isix::get_task_state( t4->tid() );
		EXPECT( (state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING)==true );
		//! Sleep the task and check it state
		isix::task_suspend( t4->tid() );
		//! Suspend special for delete
		isix::task_suspend( t1->tid() );
		auto old_count = t4->exec_count();
		state = isix::get_task_state( t4->tid() );
		EXPECT( state==OSTHR_STATE_SUSPEND );
		isix::wait_ms( 50 );
		//! Resume the task now
		EXPECT( isix::task_resume(t4->tid())==ISIX_EOK );
		isix::wait_ms( 50 );
		EXPECT( t4->exec_count() > old_count + 10 );
		state = isix::get_task_state( t1->tid() );
		EXPECT( state==OSTHR_STATE_SUSPEND );
		// Check T3 for self suspend
		t3->selfsuspend();
		isix::wait_ms(1);
		old_count = t3->exec_count();
		isix::wait_ms(2);
		EXPECT( old_count==t3->exec_count() );
		EXPECT( state==OSTHR_STATE_SUSPEND );
		EXPECT( isix::task_resume( t3->tid() )==ISIX_EOK );
		state = isix::get_task_state( t3->tid() );
		EXPECT( (state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING)==true );
		EXPECT( isix::free_stack_space(t1->tid()) >= c_stack_margin );
		EXPECT( isix::free_stack_space(t2->tid()) >= c_stack_margin );
		EXPECT( isix::free_stack_space(t3->tid()) >= c_stack_margin );
		EXPECT( isix::free_stack_space(t4->tid()) >= c_stack_margin );
		EXPECT( isix::free_stack_space(nullptr) >= c_stack_margin );
		//Now delete tasks
		delete t1;
		delete t2;
		delete t3;
		delete t4;
	},
	CASE( "06_tasks_02 Tasks suspended check" )
	{
		const auto oprio = isix::task_change_prio(nullptr, isix::get_min_priority() );
		EXPECT( oprio >= 0 );
		bool to_change = false;
		const auto test_task_suspended = [&]()
		{
			to_change = true;
			for(;;) {
				isix::wait_ms(10000);
			}
		};
		auto thr = isix::thread_create_and_run( c_stack_size,1,
				isix_task_flag_suspended,test_task_suspended );
		// Check for task create suspended
		EXPECT( thr==true );
		EXPECT( isix::get_task_state( thr.tid() )==OSTHR_STATE_SUSPEND );
		EXPECT( to_change==false );
		EXPECT( isix::task_resume(thr.tid() )==ISIX_EOK );
		auto state = isix::get_task_state( thr.tid() );
		EXPECT( (state==OSTHR_STATE_RUNNING || state==OSTHR_STATE_SLEEPING)==true);
		EXPECT( to_change==true );
		EXPECT( isix::task_change_prio(nullptr, oprio)>=0 );
		EXPECT( isix::free_stack_space(thr.tid()) >= c_stack_margin );
		EXPECT( isix::free_stack_space(nullptr) >= c_stack_margin );
	},
	CASE( "06_tasks_03 CPU load api check" )
	{
		int iload = 10;
		const auto cpuload_task = [](int load )
		{
			for( ;; ) {
				isix::wait_us( load * 1000  );
				isix::wait_ms( 100 - load );
			}
		};
		isix::wait_ms( 5000 );
		for( iload=10; iload<=99; iload+=10 ) {
			auto thr = isix::thread_create_and_run(c_stack_size,1,0,cpuload_task, iload );
			EXPECT( thr == true );
			isix::wait_ms( 4000 );
		 	const auto cpul = isix::cpuload();
			EXPECT( cpul >= iload*10-25 );
			EXPECT( cpul <= iload*10+25 );
			EXPECT( isix::free_stack_space(thr.tid()) >= c_stack_margin );
		}
		EXPECT( isix::free_stack_space(nullptr) >= c_stack_margin );
	},
	CASE( "06_tasks_04 C++11 thread api creation" )
	{
		{
			bool finished = false;
			auto thr1 = isix::thread_create_and_run( 2048, c_task_prio, 0,
					[&]( volatile bool &a, int b )
					{
					EXPECT( b==15 );
					isix::wait_ms( 100 );
					a = true;
					},
					std::ref(finished),
					15
					);
			isix::wait_ms( 200 );
			EXPECT( isix::free_stack_space(nullptr) >= c_stack_margin );
			EXPECT( finished );
		}
		{
			isix::memory_stat ms;
			isix::heap_stats( ms );
			const auto ram_beg = ms.free;
			{
				auto thr1 = isix::thread_create_and_run( c_stack_size, c_task_prio,
						0, thr11::thread2_func );
				isix_wait_ms( 900 );
				EXPECT( thr11::fin2==true );
			}
			// Not referenced task must free whole memory in idle task
			isix_wait_ms( 100 );
			isix::heap_stats( ms );
			const auto ram_end = ms.free;
			EXPECT( ram_beg>=ram_end );
			EXPECT( isix::free_stack_space(nullptr) >= c_stack_margin );
		}
	},
	CASE("06_tasks_05 Task wait and referenced api" )
	{
		//! Create referenced
		isix::memory_stat ms;
		isix::heap_stats( ms );
		auto ram_beg = ms.free;
		auto th1 = isix::task_create( waitref::task_ref, nullptr, c_stack_size, c_task_prio,
				isix_task_flag_ref|isix_task_flag_newlib );
		EXPECT( th1 );
		auto t1 = isix::get_jiffies();
		auto ret = isix::task_wait_for( th1 );
		auto t2 = isix::get_jiffies() - t1;
		//! Should return 0
		EXPECT( ret==ISIX_EOK );
		// Should match in range
		EXPECT( t2 >= 500U );
		EXPECT( t2 < 510U );
		// Task wait list should be empty
		EXPECT( thack_task_wait_list_is_empty(th1) );
		/** Check memory usage before and after because task is referenced
		 * difference between memory areas should be equal task stack size */
		isix::wait_ms(300);
		isix::heap_stats( ms );
		auto ram_end = ms.free;
		EXPECT( ram_end+thack_struct_size() >= ram_beg );
		EXPECT( (ram_beg-ram_end)==thack_struct_size() );
		EXPECT(  thack_getref_cnt( th1 ) == 1 );
		// Check the task state
		EXPECT( isix::get_task_state(th1)==OSTHR_STATE_EXITED );
		//Increment reference
		EXPECT( isix_task_ref( th1) == ISIX_EOK );
		EXPECT(  thack_getref_cnt( th1 ) == 2 );
		EXPECT( isix_task_unref( th1 ) ==ISIX_EOK );
		EXPECT(  thack_getref_cnt( th1 ) == 1 );
		EXPECT( isix_task_unref( th1 ) == ISIX_EOK );
		//! NOTE: Should be 0 but pointer to th is invalid now
		///EXPECT( thack_getref_cnt(th1) == 0 );
		isix::heap_stats( ms );
		ram_end = ms.free;
		EXPECT( ram_end>= ram_beg );
		isix::heap_stats(ms);
		EXPECT( isix::free_stack_space(nullptr) >= c_stack_margin );
	},
	CASE("06_tasks_06 Task wait reference notice")
	{
		isix::memory_stat ms;
		isix::heap_stats( ms );
		auto ram_beg = ms.free;
		// Create first for normal task_ref task. Five task_ref2 tasks wait when task 1 fin
		// and notice task 1 when ends
		auto th1 = isix::task_create( waitref::task_ref, nullptr, c_stack_size, c_task_prio,
				isix_task_flag_ref|isix_task_flag_newlib );
		auto tn1 = isix::task_create( waitref::task_ref2, th1, c_stack_size, c_task_prio,
				isix_task_flag_ref|isix_task_flag_newlib );
		auto tn2 = isix::task_create( waitref::task_ref2, th1, c_stack_size, c_task_prio,
				isix_task_flag_ref|isix_task_flag_newlib );
		auto tn3 = isix::task_create( waitref::task_ref2, th1, c_stack_size, c_task_prio,
				isix_task_flag_ref|isix_task_flag_newlib );
		auto tn4 = isix::task_create( waitref::task_ref2, th1, c_stack_size, c_task_prio,
				isix_task_flag_ref|isix_task_flag_newlib );
		auto t1 = isix::get_jiffies();
		EXPECT( isix::task_wait_for(tn1)==ISIX_EOK );
		EXPECT( isix::task_wait_for(tn2)==ISIX_EOK );
		EXPECT( isix::task_wait_for(tn3)==ISIX_EOK );
		EXPECT( isix::task_wait_for(tn4)==ISIX_EOK );
		auto t2 = isix::get_jiffies() - t1;
		EXPECT( t2 >= 550U );
		EXPECT( t2 < 600U );
		//Task th1 also should be in exited state
		isix_wait_ms(25);
		EXPECT( isix::get_task_state(th1) == OSTHR_STATE_EXITED );
		EXPECT( isix::get_task_state(tn1) == OSTHR_STATE_EXITED );
		EXPECT( isix::get_task_state(tn2) == OSTHR_STATE_EXITED );
		EXPECT( isix::get_task_state(tn3) == OSTHR_STATE_EXITED );
		EXPECT( isix::get_task_state(tn4) == OSTHR_STATE_EXITED );
		EXPECT(  thack_getref_cnt( th1 )==1 );
		EXPECT(  thack_getref_cnt( tn1 )==1 );
		EXPECT(  thack_getref_cnt( tn2 )==1 );
		EXPECT(  thack_getref_cnt( tn3 )==1 );
		EXPECT(  thack_getref_cnt( tn4 )==1 );
		EXPECT( thack_task_wait_list_is_empty(th1) );
		EXPECT( thack_task_wait_list_is_empty(tn1) );
		EXPECT( thack_task_wait_list_is_empty(tn2) );
		EXPECT( thack_task_wait_list_is_empty(tn3) );
		EXPECT( thack_task_wait_list_is_empty(tn4) );
		EXPECT( isix_task_unref( th1 )==ISIX_EOK );
		EXPECT( isix_task_unref( tn1 )==ISIX_EOK );
		EXPECT( isix_task_unref( tn2 )==ISIX_EOK );
		EXPECT( isix_task_unref( tn3 )==ISIX_EOK );
		EXPECT( isix_task_unref( tn4 )==ISIX_EOK );
		isix::heap_stats(ms);
		auto ram_end = ms.free;
		//Check if size match
		EXPECT( ram_beg==ram_end );
		EXPECT( isix::free_stack_space(nullptr) >= c_stack_margin );
	},
	CASE("06_tasks_07 Errno threadsafe")
	{
		constexpr auto errno_thread = [](int& err)
		{
			for( int i=0;i<16;++i )
			{
				errno = err;
			}
			isix_wait_ms(1);
			err = errno;
		};

		int err[4] { 5, 10, 15, 20 };
		static constexpr int except[4] { 5, 10, 15, 20 };
		errno = 50;

		auto th1 = isix::thread_create_and_run( c_stack_size, c_task_prio,
			        isix_task_flag_newlib, errno_thread, std::ref(err[0]) );
		auto th2 = isix::thread_create_and_run( c_stack_size, c_task_prio,
			        isix_task_flag_newlib, errno_thread, std::ref(err[1]) );
		auto th3 = isix::thread_create_and_run( c_stack_size, c_task_prio,
			        isix_task_flag_newlib, errno_thread, std::ref(err[2]) );
		auto th4 = isix::thread_create_and_run( c_stack_size, c_task_prio,
			        isix_task_flag_newlib, errno_thread, std::ref(err[3]) );
		EXPECT( th1 == true );
		EXPECT( th2 == true );
		EXPECT( th3 == true );
		EXPECT( th4 == true );
		isix_wait_ms(10);
		for( int i=0; i<4; ++i ) {
			EXPECT( err[i] == except[i] );
		}
		EXPECT( errno == 50 );
		EXPECT( isix::free_stack_space(nullptr) >= c_stack_margin );
	},
	CASE("06_task_08 Simple FPU single precision test without interrupts")
	{
		volatile float val = 1.0;
		const auto thr = [&]()
		{
			for( int i=0;i<100000; ++i ) {
				val += 0.5;
			}
		};
		auto th1 = isix::thread_create_and_run( c_stack_size, c_task_prio,
					isix_task_flag_newlib, thr
		);
		EXPECT( th1 == true );
		EXPECT( th1.wait_for() == ISIX_EOK );
		EXPECT( val == 50001 );
	}
#if ( __ARM_FP > 0 )
	,
	CASE("06_task_09 Simple FPU double precision test without interrupts")
	{
		volatile double val = 1.0;
		static constexpr auto n_loops = 100000U;
		const auto thr = [&]()
		{
			for( unsigned i=0;i<n_loops; ++i ) {
				val += 0.5;
			}
		};
		auto th1 = isix::thread_create_and_run( c_stack_size, c_task_prio,
					isix_task_flag_newlib, thr
		);
		EXPECT( th1 == true );
		EXPECT( th1.wait_for() == ISIX_EOK );
		EXPECT( val == 50001 );
	},
	CASE("06_task_10 FPU single precision two tasks and interrupt")
	{
		static constexpr auto n_loops = 10000000U;
		using namespace tests::fpu_sp;
		constexpr auto thr = []( int begin_val, bool& ok ) -> void
		{
			fill_and_add( begin_val );
			for( unsigned i=0; i<n_loops;++i ) {
				if( fill_and_add_check(begin_val) ) {
					ok = false;
					break;
				}
			}
			ok = true;
		};
		constexpr auto thr_c = []( float& val ) -> void
		{
			for( unsigned i=0;i<n_loops; ++i ) {
				val += 0.5;
			}
		};
		volatile bool irq_failed {};
		bool res1 {}; bool res2 {};
		float val = 1.0;
		int irq_nums {};
		const auto irq_fun = [&]()
		{
			using namespace tests::fpu_sp;
			irq_nums++;
			if( irq_failed ) {
				return;
			}
			base_regs_fill( 0x44 );
			if( base_regs_check(0x44) ) {
				irq_failed = true;
			}
		};
		tests::detail::periodic_timer_setup( irq_fun, 200 );
		auto th1 = isix::thread_create_and_run( 2048, c_task_prio,
				isix_task_flag_newlib, thr, 4, std::ref(res1) );
		auto th2 = isix::thread_create_and_run( 2048, c_task_prio,
				isix_task_flag_newlib, thr, 2, std::ref(res2) );
		auto th3 = isix::thread_create_and_run( 2048, c_task_prio,
				isix_task_flag_newlib, thr_c, std::ref(val) );
		EXPECT( th1 == true );
		EXPECT( th2 == true );
		EXPECT( th3 == true );
		EXPECT( th1.wait_for() == ISIX_EOK );
		EXPECT( th2.wait_for() == ISIX_EOK );
		EXPECT( th3.wait_for() == ISIX_EOK );
		EXPECT( res1 == true );
		EXPECT( res2 == true );
		EXPECT( int(val) == int(n_loops/2+1) );
		tests::detail::periodic_timer_stop();
		EXPECT( irq_nums > 10 );
		EXPECT( irq_failed == false );
		isix::wait_ms(100);
	}
#endif /* __ARM_FP > 0 */
};


extern lest::tests & specification();
MODULE( specification(), module )
