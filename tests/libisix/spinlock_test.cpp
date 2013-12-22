/*
 * =====================================================================================
 *
 *       Filename:  spinlock_test.cpp
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
#include "spinlock_test.hpp"

#include "qunit.hpp"
#include "timer_interrupt.hpp"
#include <isix.h>
#include <stm32system.h>
#include <foundation/dbglog.h>
#include <string>

namespace tests {

namespace {
	class task : public isix::task_base
	{
	static constexpr auto STACK_SIZE = 1024;
	public:
		task( char ch_id, std::string& bs, isix::spinlock& lock )
			: m_id( ch_id), m_ret( bs ), m_lock( lock )
		{
		}
		void start( isix::prio_t prio ) {
			start_thread( STACK_SIZE, prio );
		}
	private:
		virtual void main() 
		{
			isix::scoped_lock<isix::spinlock> lock(m_lock);
			for(int i=0;i<1000000;++i)
				stm32::nop();
			m_ret.push_back( m_id );
		}
	private:
		const char m_id; 
		std::string& m_ret;
		isix::spinlock& m_lock;
	};
}

//Scheduler API test
void spinlock_tests::scheduler_api()
{ 
	isix::_isixp_lock_scheduler();
	dbprintf("Scheduler locked yield test");
	for(int i=0;i<100;++i) {
		isix::isix_yield();
	}
	isix::_isixp_unlock_scheduler();
	dbprintf("Short time scheduler lock");
	isix::_isixp_lock_scheduler();
	for(int i=0;i<10000000;++i )
		stm32::nop();
	isix::_isixp_unlock_scheduler();
	dbprintf("Long time scheduler lock");
#if 0
	for( int i=0; i<100; ++i ) {
		isix::isix_yield();
	}
#endif
	//dbprintf("Scheduler yield test");
}

//Basic spin test
void spinlock_tests::basic_test() 
{
	std::string tstr;
	isix::spinlock tstr_lock;
	task t1( 'A', tstr, tstr_lock ); 
	task t2( 'B', tstr, tstr_lock ); 
	task t3( 'C', tstr, tstr_lock ); 
	task t4( 'D', tstr, tstr_lock ); 
	t1.start(0);
	t2.start(1);
	t3.start(2);
	t4.start(3);
	const auto tbeg = isix::isix_get_jiffies();
	constexpr auto tout = 18000;
	bool ok {};
	while(true) {
		tstr_lock.lock();
		const auto r = tstr.size();
		tstr_lock.unlock();
		if( r == 4 ) {
			ok = true;
			break;
		}
		if( isix::isix_get_jiffies() - tbeg > tout ) {
			ok = false;
			break;
		}
		isix::isix_wait_ms(2);
	}
	QUNIT_IS_TRUE( ok );
	QUNIT_IS_EQUAL( tstr, "ABCD" );
}

}
