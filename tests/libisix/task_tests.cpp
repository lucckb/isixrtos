/*
 * =====================================================================================
 *
 *       Filename:  task_tests.cpp
 *
 *    Description:  Basic task testing isix
 *
 *        Version:  1.0
 *        Created:  09.12.2013 23:58:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include "task_tests.hpp"
#include <qunit.hpp>
#include <isix.h>
#include <foundation/dbglog.h>


namespace tests {


namespace {

	//Test basic task functionality
	class base_task_tests : public isix::task_base {
		static constexpr auto STACK_SIZE = 1024;
		volatile unsigned m_exec_count {};
		volatile bool m_req_selfsusp {};
		//Main function
		virtual void main() {
			for(;;) {
				++m_exec_count;
				if( m_req_selfsusp ) {
					m_req_selfsusp = false;
					isix::task_suspend(nullptr);
				}
			}
		}
	public:
		base_task_tests() {}
		static constexpr auto TASK_PRIO = 1;
		void start() {
			start_thread(STACK_SIZE, TASK_PRIO);
		}
		void selfsuspend() {
			m_req_selfsusp = true;
		}
		virtual ~base_task_tests() {}
		unsigned exec_count() const {
			return m_exec_count;
		}
		void exec_count( unsigned v ) {
			m_exec_count = v;
		}
	};

	void test_task_suspended( void *data ) {
		auto& do_it = *reinterpret_cast<bool*>(data);
		do_it = true;
		for(;;) {
			isix::wait_ms(10000);
		}
	}

}	//Unnamed namespace end

// Basic functionality test without IRQS
void task_tests::basic_funcs()
{	
	//Check if scheduler is running
	QUNIT_IS_TRUE( isix_is_scheduler_active() );
	auto t1 = new base_task_tests;
	auto t2 = new base_task_tests;
	auto t3 = new base_task_tests;
	auto t4 = new base_task_tests; 
	t1->start(); t2->start(); t3->start(); t4->start();
	//Try set private data
	QUNIT_IS_EQUAL( isix_set_task_private_data(t1->get_taskid(), 
				reinterpret_cast<void*>(1)), ISIX_EOK );
	QUNIT_IS_EQUAL( isix_set_task_private_data(t2->get_taskid(), 
				reinterpret_cast<void*>(2)), ISIX_EOK );
	QUNIT_IS_EQUAL( isix_set_task_private_data(t3->get_taskid(), 
				reinterpret_cast<void*>(3)), ISIX_EOK );
	QUNIT_IS_EQUAL( isix_set_task_private_data(t4->get_taskid(), 
				reinterpret_cast<void*>(4)), ISIX_EOK );
	//Active wait tasks shouldnt run
	for(auto tc = isix_get_jiffies(); isix_get_jiffies()<tc+5000; ) {
		asm volatile("nop\n");
	}
	QUNIT_IS_FALSE( t1->exec_count() );
	QUNIT_IS_FALSE( t2->exec_count() );
	QUNIT_IS_FALSE( t3->exec_count() );
	QUNIT_IS_FALSE( t4->exec_count() );
	//Now goto sleep
	isix_wait_ms( 5000 );
	//TASK should run now
	QUNIT_IS_TRUE( t1->exec_count()>0 );
	QUNIT_IS_TRUE( t2->exec_count()>0 );
	QUNIT_IS_TRUE( t3->exec_count()>0 );
	QUNIT_IS_TRUE( t4->exec_count()>0 );
	//Zero task count.. change prio and go active wait
	t1->exec_count(0);
	t2->exec_count(0);
	t3->exec_count(0);
	t4->exec_count(0);
	QUNIT_IS_FALSE( t1->exec_count() );
	QUNIT_IS_FALSE( t2->exec_count() );
	QUNIT_IS_FALSE( t3->exec_count() );
	QUNIT_IS_FALSE( t4->exec_count() );

	QUNIT_IS_EQUAL( isix_task_change_prio(t1->get_taskid(),0), base_task_tests::TASK_PRIO );
	QUNIT_IS_EQUAL( isix_task_change_prio(t2->get_taskid(),0), base_task_tests::TASK_PRIO );
	QUNIT_IS_EQUAL( isix_task_change_prio(t3->get_taskid(),0), base_task_tests::TASK_PRIO );
	QUNIT_IS_EQUAL( isix_task_change_prio(t4->get_taskid(),0), base_task_tests::TASK_PRIO );

	//Active wait tasks shouldnt run
	for( auto tc = isix_get_jiffies(); isix_get_jiffies()<tc+5000; ) {
			asm volatile("nop\n");
		}
	//TASK should run now
	QUNIT_IS_TRUE( t1->exec_count()>0 );
	QUNIT_IS_TRUE( t4->exec_count()>0 );
	
	//After finish all tasks check private data
	QUNIT_IS_EQUAL( isix_get_task_private_data(t1->get_taskid()), reinterpret_cast<void*>(1) );
	QUNIT_IS_EQUAL( isix_get_task_private_data(t2->get_taskid()), reinterpret_cast<void*>(2) );
	QUNIT_IS_EQUAL( isix_get_task_private_data(t3->get_taskid()), reinterpret_cast<void*>(3) );
	QUNIT_IS_EQUAL( isix_get_task_private_data(t4->get_taskid()), reinterpret_cast<void*>(4) );
	
	//Validate stack space functionality
	QUNIT_IS_TRUE( isix_free_stack_space(t1->get_taskid()) > MIN_STACK_FREE  );
	QUNIT_IS_TRUE( isix_free_stack_space(t2->get_taskid()) > MIN_STACK_FREE  );
	QUNIT_IS_TRUE( isix_free_stack_space(t3->get_taskid()) > MIN_STACK_FREE  );
	QUNIT_IS_TRUE( isix_free_stack_space(t4->get_taskid()) > MIN_STACK_FREE  );
	QUNIT_IS_TRUE( isix_free_stack_space(nullptr) > MIN_STACK_FREE  );

	//! Get task state should be ready or running
	auto state = isix::get_task_state( t1->get_taskid() );
	QUNIT_IS_TRUE( state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING );
	state = isix::get_task_state( t2->get_taskid() );
	QUNIT_IS_TRUE( state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING );
	state = isix::get_task_state( t3->get_taskid() );
	QUNIT_IS_TRUE( state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING );
	state = isix::get_task_state( t4->get_taskid() );
	QUNIT_IS_TRUE( state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING );

	//! Sleep the task and check it state
	isix::task_suspend( t4->get_taskid() );
	//! Suspend special for delete
	isix::task_suspend( t1->get_taskid() );
	auto old_count = t4->exec_count();
	state = isix::get_task_state( t4->get_taskid() );
	QUNIT_IS_EQUAL( state, OSTHR_STATE_SUSPEND );
	isix::wait_ms( 50 );
	//! Resume the task now
	QUNIT_IS_EQUAL( isix::task_resume(t4->get_taskid()), ISIX_EOK );
	isix::wait_ms( 50 );
	QUNIT_IS_TRUE( t4->exec_count() > old_count + 10 );
	state = isix::get_task_state( t1->get_taskid() );
	QUNIT_IS_EQUAL( state, OSTHR_STATE_SUSPEND );
	// Check T3 for self suspend
	t3->selfsuspend();
	isix::wait_ms(1);
	old_count = t3->exec_count();
	isix::wait_ms(2);
	QUNIT_IS_EQUAL( old_count, t3->exec_count() );
	QUNIT_IS_EQUAL( state, OSTHR_STATE_SUSPEND );

	QUNIT_IS_EQUAL( isix::task_resume( t3->get_taskid() ), ISIX_EOK );
	state = isix::get_task_state( t3->get_taskid() );
	QUNIT_IS_TRUE( state==OSTHR_STATE_READY || state==OSTHR_STATE_RUNNING );

	//Now delete tasks
	delete t1;
	delete t2;
	delete t3;
	delete t4;
	// Final test with create suspended
	{

		const auto oprio = isix::task_change_prio(nullptr, isix::get_min_priority() );
		QUNIT_IS_TRUE( oprio >= 0 );
		bool to_change = false;
		auto tsk = isix::task_create(test_task_suspended,&to_change,
				512,1, isix_task_flag_suspended );
		// Check for task create suspended
		QUNIT_IS_NOT_EQUAL( tsk, nullptr );
		QUNIT_IS_EQUAL( isix::get_task_state( tsk ) , OSTHR_STATE_SUSPEND );
		QUNIT_IS_FALSE( to_change );
		QUNIT_IS_EQUAL( isix::task_resume(tsk), ISIX_EOK );
		auto state = isix::get_task_state( tsk );
		QUNIT_IS_TRUE( state==OSTHR_STATE_RUNNING || state==OSTHR_STATE_SLEEPING );
		QUNIT_IS_TRUE( to_change );
		QUNIT_IS_TRUE( isix::task_change_prio(nullptr, oprio)>=0 );
		isix_task_kill( tsk );
	}
}


}	// Namespace test end

