/*
 * =====================================================================================
 *
 *       Filename:  semaphore_tests.cpp
 *
 *    Description:  Semaphore unit testing for isix rtos
 *
 *        Version:  1.0
 *        Created:  09.12.2013 22:37:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza <lucck(at)boff(dot)pl>
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

/* ------------------------------------------------------------------ */
#include "semaphore_test.hpp"
#include "qunit.hpp"
#include <isix.h>

namespace tests {

/* ------------------------------------------------------------------ */
namespace {
/* ------------------------------------------------------------------ */
	//Basic semaphore test
	class semaphore_task_test : public isix::task_base {
		
		static constexpr auto STACK_SIZE = 1024;
		//Main funcs
		virtual void main() {
            m_error = m_sem.wait( isix::ISIX_TIME_INFINITE );
            m_items.push_back( m_id );  
			for(;;) isix::isix_wait_ms(1000);
		}
	public:
		semaphore_task_test( char ch_id, isix::prio_t prio, isix::semaphore &sem, std::string &items ) 
			: task_base( STACK_SIZE, prio ), 
            m_sem( sem ), m_id( ch_id ), m_items( items )
		{
		}
		virtual ~semaphore_task_test() {}
        int error() const {
			return m_error;
        }
    private:
        isix::semaphore& m_sem;
        const char  m_id;
        std::string& m_items;
        int m_error { -32768 };
	};

/* ------------------------------------------------------------------ */
	//Semaphore time test
	class semaphore_time_task : public isix::task_base {
		static constexpr auto TASK_PRIO = 3;
		static constexpr auto STACK_SIZE = 1024;
		static constexpr auto sem_tout = 500;
	public:
		semaphore_time_task( isix::semaphore& sem  ) 
			: task_base( STACK_SIZE, TASK_PRIO ),
			m_sem(sem)
		{	
		}
		virtual ~semaphore_time_task(){}
		virtual void main() {
			for( ;; ) { 
				m_error = m_sem.wait ( sem_tout );
				m_notify_sem.signal();
			}
		}
        int error() const {
          
            m_notify_sem.wait( isix::ISIX_TIME_INFINITE );
			return m_error;
        }
	private:
		isix::semaphore &m_sem;
        int m_error { -32768 };
		mutable isix::semaphore m_notify_sem { 0, 1 };
	};
/* ------------------------------------------------------------------ */
}	// Unnamed namespace end


/* ------------------------------------------------------------------ */
//Create timing semaphore test
void semaphores::semaphore_time_test() 
{
		isix::semaphore sigs(0);
		semaphore_time_task t1( sigs );
		QUNIT_IS_TRUE( t1.is_valid() );
		QUNIT_IS_EQUAL( t1.error() , isix::ISIX_ETIMEOUT );
		sigs.signal();
		QUNIT_IS_EQUAL( t1.error() , isix::ISIX_EOK );
}

/* ------------------------------------------------------------------ */
//Semaphore priority testing
void semaphores::semaphore_prio_tests() 
{
	//TODO: Must be fixed the task creation like tomek said 
	//ThreadRunner<MyThreadClass>
	static constexpr auto test_prio = 3;
	QUNIT_IS_EQUAL( isix::isix_task_change_prio( nullptr, test_prio ), TASKDEF_PRIORITY );	        
	std::string tstr;		
	isix::semaphore sigs(0);
	QUNIT_IS_TRUE( sigs.is_valid() );  
	semaphore_task_test t1('A', 3, sigs, tstr );
	semaphore_task_test t2('B', 2, sigs, tstr );
	semaphore_task_test t3('C', 1, sigs, tstr );
	semaphore_task_test t4('D', 0, sigs, tstr );
	QUNIT_IS_TRUE( t1.is_valid() );
	QUNIT_IS_TRUE( t2.is_valid() );
	QUNIT_IS_TRUE( t3.is_valid() );
	QUNIT_IS_TRUE( t4.is_valid() );
	QUNIT_IS_EQUAL( sigs.signal(), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( sigs.signal(), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( sigs.signal(), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( sigs.signal(), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( tstr, "DCBA" );
	//Check semaphore status
	QUNIT_IS_EQUAL( t1.error(), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( t2.error(), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( t3.error(), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( t4.error(), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( isix::isix_task_change_prio(nullptr,TASKDEF_PRIORITY ), test_prio );	        
}


/* ------------------------------------------------------------------ */
}	// Namespace tests end
/* ------------------------------------------------------------------ */

