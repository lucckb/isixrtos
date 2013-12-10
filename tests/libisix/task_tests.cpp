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

/* ------------------------------------------------------------------ */
namespace tests {

/* ------------------------------------------------------------------ */
namespace {
/* ------------------------------------------------------------------ */
	//Test basic task functionality
	class base_task_tests : public isix::task_base {
		static constexpr auto STACK_SIZE = 1024;
		volatile unsigned m_exec_count {};
		//Main function
		virtual void main() {
			for(;;) {
				++m_exec_count;
			}
		}
	public:
		static constexpr auto TASK_PRIO = 1;
		base_task_tests()
			: task_base(STACK_SIZE, TASK_PRIO)
		{}
		virtual ~base_task_tests() {}
		unsigned exec_count() const {
			return m_exec_count;
		}
		void exec_count( unsigned v ) {
			m_exec_count = v;
		}
	};

	
/* ------------------------------------------------------------------ */
}	//Unnamed namespace end
/* ------------------------------------------------------------------ */
// Basic functionality test without IRQS
void task_tests::basic_funcs()
{
	auto t1 = new base_task_tests;
	auto t2 = new base_task_tests;
	auto t3 = new base_task_tests;
	auto t4 = new base_task_tests; 
	//Try set private data
	QUNIT_IS_EQUAL( isix::isix_set_task_private_data(t1->get_taskid(), reinterpret_cast<void*>(1)), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( isix::isix_set_task_private_data(t2->get_taskid(), reinterpret_cast<void*>(2)), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( isix::isix_set_task_private_data(t3->get_taskid(), reinterpret_cast<void*>(3)), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( isix::isix_set_task_private_data(t4->get_taskid(), reinterpret_cast<void*>(4)), isix::ISIX_EOK );
	//Active wait tasks shouldnt run
	for(auto tc = isix::isix_get_jiffies(); isix::isix_get_jiffies()<tc+5000; ) {
		asm volatile("nop\n");
	}
	QUNIT_IS_FALSE( t1->exec_count() );
	QUNIT_IS_FALSE( t2->exec_count() );
	QUNIT_IS_FALSE( t3->exec_count() );
	QUNIT_IS_FALSE( t4->exec_count() );
	//Now goto sleep
	isix::isix_wait_ms( 5000 );
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

	QUNIT_IS_EQUAL( isix::isix_task_change_prio(t1->get_taskid(),0), base_task_tests::TASK_PRIO );
	QUNIT_IS_EQUAL( isix::isix_task_change_prio(t2->get_taskid(),0), base_task_tests::TASK_PRIO );
	QUNIT_IS_EQUAL( isix::isix_task_change_prio(t3->get_taskid(),0), base_task_tests::TASK_PRIO );
	QUNIT_IS_EQUAL( isix::isix_task_change_prio(t4->get_taskid(),0), base_task_tests::TASK_PRIO );

	//Active wait tasks shouldnt run
	for(auto tc = isix::isix_get_jiffies(); isix::isix_get_jiffies()<tc+5000; ) {
			asm volatile("nop\n");
		}
	//TASK should run now
	QUNIT_IS_TRUE( t1->exec_count()>0 );
	QUNIT_IS_TRUE( t4->exec_count()>0 );
	
	//After finish all tasks check private data
	QUNIT_IS_EQUAL( isix::isix_get_task_private_data(t1->get_taskid()), reinterpret_cast<void*>(1) );
	QUNIT_IS_EQUAL( isix::isix_get_task_private_data(t2->get_taskid()), reinterpret_cast<void*>(2) );
	QUNIT_IS_EQUAL( isix::isix_get_task_private_data(t3->get_taskid()), reinterpret_cast<void*>(3) );
	QUNIT_IS_EQUAL( isix::isix_get_task_private_data(t4->get_taskid()), reinterpret_cast<void*>(4) );
	
	//Validate stack space functionality
	QUNIT_IS_TRUE( isix::isix_free_stack_space(t1->get_taskid()) > MIN_STACK_FREE  );
	QUNIT_IS_TRUE( isix::isix_free_stack_space(t2->get_taskid()) > MIN_STACK_FREE  );
	QUNIT_IS_TRUE( isix::isix_free_stack_space(t3->get_taskid()) > MIN_STACK_FREE  );
	QUNIT_IS_TRUE( isix::isix_free_stack_space(t4->get_taskid()) > MIN_STACK_FREE  );
	QUNIT_IS_TRUE( isix::isix_free_stack_space(nullptr) > MIN_STACK_FREE  );	

	//Now delete tasks
	delete t1;
	delete t2;
	delete t3;
	delete t4;
}

/* ------------------------------------------------------------------ */
}

/* ------------------------------------------------------------------ */
