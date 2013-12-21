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
			//isix::scoped_lock<isix::spinlock> lock(m_lock);
			m_lock.lock();
			for(int i=0;i<10000;++i)
				stm32::nop();
			m_ret.push_back( m_id );
			m_lock.unlock();
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
	for(int i=0;i<100;++i)
		stm32::nop();
	isix::_isixp_unlock_scheduler();
	dbprintf("Short time scheduler lock");
	isix::_isixp_lock_scheduler();
	for(int i=0;i<10000000;++i )
		stm32::nop();
	isix::_isixp_unlock_scheduler();
	dbprintf("Long time scheduler lock");
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
	isix::isix_wait_ms( 3000 );
	dbprintf("Exited return val %s\n", tstr.c_str() );
}

}
