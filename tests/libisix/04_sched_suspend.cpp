/*
 * =====================================================================================
 *
 *       Filename:  sched_suspend.cpp
 *
 *    Description:  Scheduler suspend tests
 *
 *        Version:  1.0
 *        Created:  22.06.2017 20:48:35
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

//Internal API MOCK
extern "C" {
	
	void _isixp_lock_scheduler();
	void _isixp_unlock_scheduler();
}

namespace
{
	//Task for testing lock scheduler
	class task
	{
	public:
		task& operator=( task& ) = delete;
		task( task& ) = delete;

		//Constructor
		explicit task( char pattern )
			: m_act_pattern( pattern )
			, m_thr( isix::thread_create( std::bind(&task::thread,std::ref(*this))))
		{
		}
		//Start task
		void start() {
			m_thr.start_thread( STACK_SIZE, THREAD_PRIO );
		}
		char get_id() const {
			return m_act_id;
		}
		void reset_id() {
			m_act_id = ' ';
		}
	private:
		//Main test task
		void thread() noexcept
		{
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
	private:
		static constexpr auto STACK_SIZE = 2048;
		static constexpr auto THREAD_PRIO = 0;
		volatile char m_act_id { ' ' };
		const char  m_act_pattern;
		isix::thread m_thr;
	};
}

const lest::test module[] =
{
	CASE( "04_sched_01 Basic lock test" )
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
		EXPECT( isix::is_scheduler_active() == true );
	},
	CASE( "04_sched_02 Basic resched test" )
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
		EXPECT( t1.get_id()== 'A' );
		EXPECT( t2.get_id()== 'B' );
		EXPECT( t3.get_id()== 'C' );
		EXPECT( t4.get_id()== 'D' );
	},
	CASE( "04_sched_03 Tasks reordering test" )
	{
		task *t1 = new task( 'A' );
		task *t2 = new task( 'B' );
		task *t3 = new task( 'C' );
		task *t4 = new task( 'D' );
		EXPECT( reinterpret_cast<unsigned>(t1) % 4==0U );
		EXPECT( reinterpret_cast<unsigned>(t2) % 4==0U );
		EXPECT( reinterpret_cast<unsigned>(t3) % 4==0U );
		EXPECT( reinterpret_cast<unsigned>(t4) % 4==0U );
		_isixp_lock_scheduler();
		t1->start();
		t2->start();
		t3->start();
		t4->start();
		for(int i=0;i<1000000;++i) stm32::nop();
		EXPECT( t1->get_id()== ' ' );
		EXPECT( t2->get_id()== ' ' );
		EXPECT( t3->get_id()== ' ' );
		EXPECT( t4->get_id()== ' ' );
		_isixp_unlock_scheduler();
		delete t1;
		delete t2;
		delete t3;
		delete t4;
	},
	CASE( "04_sched_04 Ujiffies check" )
	{
		//Test 1
		auto t1 = isix_get_jiffies();
		isix_wait_us( 5000 );
		auto t2 = isix_get_jiffies();
		EXPECT( t2-t1 >= 5U );
		EXPECT( t2-t1 <= 6U );
		//Test2 long
		t1 = isix_get_jiffies();
		isix_wait_us( 500000 );
		t2 = isix_get_jiffies();
		EXPECT(t2-t1 >= 499U );
		EXPECT(t2-t1 <= 501U );
		//Final give a chance to cleanup resources
		isix::wait_ms(10);
	}
};


extern lest::tests & specification();
MODULE( specification(), module )
