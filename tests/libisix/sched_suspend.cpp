/*
 * =====================================================================================
 *
 *       Filename:  sched_suspend.cpp
 *
 *    Description:  SPINLOCK schedule base tests
 *
 *        Version:  1.0
 *        Created:  21.12.2013 20:37:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include "sched_suspend.hpp"
#include "qunit.hpp"
#include "timer_interrupt.hpp"
#include <isix.h>
#include <stm32system.h>
#include <foundation/dbglog.h>
#include <string>

//Internal temporary definition

extern "C" {
	
	void _isixp_lock_scheduler();
	void _isixp_unlock_scheduler();
}



namespace {
	//Task for testing lock scheduler
	class task : public isix::task_base {
		static constexpr auto STACK_SIZE = 1024;
		static constexpr auto THREAD_PRIO = 0;
		volatile char m_act_id { ' ' };
		const char  m_act_pattern;
		//Main test task
		virtual void main() noexcept {
			for(;;) {
				m_act_id = m_act_pattern;
				stm32::nop();
				//isix::isix_wait_ms( 1 );
				stm32::nop();
				stm32::nop();
				stm32::nop();
				stm32::nop();
			}
		}
	public:
		//Constructor
		task( char pattern )
			: m_act_pattern( pattern )
		{}
		//Start task
		void start() {
			start_thread( STACK_SIZE, THREAD_PRIO );
		}
		char get_id() const {
			return m_act_id;
		}
		void reset_id() {
			m_act_id = ' ';	
		}
	};
}

namespace tests {

//Scheduler API test
void sched_suspend::basic_lock()
{
	_isixp_lock_scheduler();
	for(int i=0;i<100;++i) {
		isix_yield();
	}
	_isixp_unlock_scheduler();
	_isixp_lock_scheduler();
	for(int i=0;i<10000000;++i )
		stm32::nop();
	_isixp_unlock_scheduler();
	for( int i=0; i<100; ++i ) {
		isix_yield();
	}
}
/*
 * Basic lock test check reschedule task 
 *  and also check the time jiffies time provider
 */
void sched_suspend::task_lock() 
{
	{
		task t1( 'A' );
		task t2( 'B' );
		task t3( 'C' );
		task t4( 'D' );
		t1.start();
		t2.start();
		t3.start();
		t4.start();
		isix_wait_ms( 500 );
		QUNIT_IS_EQUAL( t1.get_id(), 'A' );
		QUNIT_IS_EQUAL( t2.get_id(), 'B' );
		QUNIT_IS_EQUAL( t3.get_id(), 'C' );
		QUNIT_IS_EQUAL( t4.get_id(), 'D' );
	}
	{

		task *t1 = new task( 'A' );
		task *t2 = new task( 'B' );
		task *t3 = new task( 'C' );
		task *t4 = new task( 'D' );
		QUNIT_IS_EQUAL( reinterpret_cast<unsigned>(t1) % 4, 0U );
		QUNIT_IS_EQUAL( reinterpret_cast<unsigned>(t2) % 4, 0U );
		QUNIT_IS_EQUAL( reinterpret_cast<unsigned>(t3) % 4, 0U );
		QUNIT_IS_EQUAL( reinterpret_cast<unsigned>(t4) % 4, 0U );
		_isixp_lock_scheduler();
		t1->start();
		t2->start();
		t3->start();
		t4->start();
		for(int i=0;i<1000000;++i) stm32::nop();
		QUNIT_IS_EQUAL( t1->get_id(), ' ' );
		QUNIT_IS_EQUAL( t2->get_id(), ' ' );
		QUNIT_IS_EQUAL( t3->get_id(), ' ' );
		QUNIT_IS_EQUAL( t4->get_id(), ' ' );
		_isixp_unlock_scheduler();
		delete t1;
		delete t2;
		delete t3;
		delete t4;
	}
}

/** Wait for ujiffies testing PI */
void sched_suspend::non_block_wait()
{
	//Test 1
	auto t1 = isix_get_jiffies();
	isix_wait_us( 5000 );
	auto t2 = isix_get_jiffies();
	QUNIT_IS_TRUE( t2-t1==5 || t2-t1==6 );
	//Test2 long
	t1 = isix_get_jiffies();
	isix_wait_us( 500000 );
	t2 = isix_get_jiffies();
	QUNIT_IS_TRUE( t2-t1 >=499 && t2-t1<=501 );
	//Finall give a chance to cleanup resources
	isix::wait_ms(10);
}


} // Namespace test end 

