/*
 * =====================================================================================
 *
 *       Filename:  semaphores_test.cpp
 *
 *    Description:  Semaphores test
 *
 *        Version:  1.0
 *        Created:  23.06.2017 19:45:21
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
#include "timer_interrupt.hpp"

namespace
{
	constexpr auto N_TEST_POSTS = 25;
	//Basic semaphore test
	class semaphore_task_test : public isix::task_base
	{
		static constexpr auto STACK_SIZE = 2048;
		//Main funcs
		virtual void main()  noexcept
		{
            m_error = m_sem.wait( ISIX_TIME_INFINITE );
            m_items.push_back( m_id );  
			m_join_sem.signal();
			//for(;;) isix::isix_wait_ms(1000);
		}
	public:
		semaphore_task_test( char ch_id, osprio_t prio, isix::semaphore &sem, std::string &items )
            : m_sem( sem ), m_id( ch_id ), m_items( items ), m_prio( prio )
		{
		}
		void start() {
			start_thread(STACK_SIZE, m_prio );
		}
		virtual ~semaphore_task_test() {}
        int error() const {
			return m_error;
        }
		void join() {
			m_join_sem.wait( ISIX_TIME_INFINITE );
		}
		int val() const {
			return m_sem.getval();
		}
    private:
        isix::semaphore& m_sem;
        const char  m_id;
        std::string& m_items;
        int m_error { -32768 };
		osprio_t m_prio;
		isix::semaphore m_join_sem { 0, 1 };
	};

	//Semaphore time test
	class semaphore_time_task : public isix::task_base {
		static constexpr auto TASK_PRIO = 3;
		static constexpr auto STACK_SIZE = 2048;
		static constexpr auto sem_tout = 500;
	public:
		semaphore_time_task( isix::semaphore& sem  )
			: m_sem(sem)
		{
		}
		virtual ~semaphore_time_task(){}
		virtual void main() noexcept {
			for( ;; ) { 
				m_error = m_sem.wait ( sem_tout );
				m_notify_sem.signal();
			}
		}
		void start() {
			start_thread(STACK_SIZE, TASK_PRIO);
		}
        int error() const {
            m_notify_sem.wait( ISIX_TIME_INFINITE );
			return m_error;
        }
		int val() const {
			return m_sem.getval();
		}
	private:
		isix::semaphore &m_sem;
        int m_error { -32768 };
		mutable isix::semaphore m_notify_sem { 0, 1 };
	};

}

const lest::test module[] =
{
	CASE( "05_sem_01 Semaphore timeout check" )
	{
		isix::semaphore sigs(0);
		semaphore_time_task t1( sigs ); t1.start();
		EXPECT( t1.is_valid()==true );
		EXPECT( t1.error()==ISIX_ETIMEOUT );
		sigs.signal();
		EXPECT( t1.error()==ISIX_EOK );
	},
	CASE( "05_sem_02 Semaphore priority check" )
	{
		static constexpr auto TASKDEF_PRIORITY = 0;
		//ThreadRunner<MyThreadClass>
		static constexpr auto test_prio = 3;
		EXPECT( isix_task_change_prio( nullptr, test_prio )==TASKDEF_PRIORITY );
		std::string tstr;
		isix::semaphore sigs(0);
		EXPECT( sigs.is_valid()==true);
		semaphore_task_test t1('A', 3, sigs, tstr );
		semaphore_task_test t2('B', 2, sigs, tstr );
		semaphore_task_test t3('C', 1, sigs, tstr );
		semaphore_task_test t4('D', 0, sigs, tstr );
		t1.start(); t2.start(); t3.start(); t4.start();
		EXPECT( t1.is_valid()==true);
		EXPECT( t2.is_valid()==true);
		EXPECT( t3.is_valid()==true);
		EXPECT(t4.is_valid()==true);
		EXPECT( sigs.signal()==ISIX_EOK );
		EXPECT( sigs.signal()==ISIX_EOK );
		EXPECT( sigs.signal()==ISIX_EOK );
		EXPECT( sigs.signal()==ISIX_EOK );
		t1.join(); t2.join(); t3.join(); t4.join();
		EXPECT( tstr=="DCBA" );
		//Check semaphore status
		EXPECT( t1.error()==ISIX_EOK );
		EXPECT( t2.error()==ISIX_EOK );
		EXPECT( t3.error()==ISIX_EOK );
		EXPECT( t4.error()==ISIX_EOK );
		EXPECT( isix_task_change_prio(nullptr,TASKDEF_PRIORITY )==test_prio );
	},
	CASE( "05_sem_03 Semaphore reset api" )
	{
		auto sigs = new isix::semaphore(0);
		std::string tstr;
		EXPECT( sigs->is_valid() == true );
		semaphore_task_test t1('A', 3, *sigs, tstr );
		semaphore_task_test t2('B', 2, *sigs, tstr );
		semaphore_task_test t3('C', 1, *sigs, tstr );
		semaphore_task_test t4('D', 0, *sigs, tstr );
		t1.start(); t2.start(); t3.start(); t4.start();
		EXPECT( t1.is_valid()==true);
		EXPECT( t2.is_valid()==true);
		EXPECT( t3.is_valid()==true);
		EXPECT( t4.is_valid()==true);
		//! Give some time for add to sem
		isix::wait_ms(25);
		EXPECT( sigs->reset(5)==ISIX_EOK );
		t1.join(); t2.join(); t3.join(); t4.join();
		EXPECT( tstr=="DCBA" );
		//Check semaphore status
		EXPECT( t1.error()==ISIX_ERESET );
		EXPECT( t2.error()==ISIX_ERESET );
		EXPECT( t3.error()==ISIX_ERESET );
		EXPECT( t4.error()==ISIX_ERESET );
		EXPECT( t1.val()==5 );
		EXPECT( t2.val()==5 );
		EXPECT( t3.val()==5 );
		EXPECT( t4.val()==5 );
		// Try to wait and next delete
		delete sigs;
		sigs = new isix::semaphore(0);
		semaphore_task_test t5('Z', 2, *sigs, tstr ); t5.start();
		semaphore_task_test t6('Y', 2, *sigs, tstr ); t6.start();
		isix::wait_ms(25);
		delete sigs;
		t5.join(); t6.join();
		EXPECT( t5.error()==ISIX_EDESTROY );
		EXPECT( t6.error()==ISIX_EDESTROY );
	},
	CASE( "05_sem_04 Semaphore interrupt api check" )
	{
		volatile int irq_get_isr_nposts {};
		volatile int test_count { }; //Post IRQ sem five times
		isix::semaphore m_sem_irq { 0, 0 };
		isix::semaphore m_sem_irq_get { 0, 0 };
		const auto isr_test_handler = [&]()
		{
			if( test_count++ < N_TEST_POSTS )
			{
				m_sem_irq.signal_isr();
			} else {
				while( m_sem_irq_get.trywait() == ISIX_EOK ) {
					if( ++irq_get_isr_nposts > N_TEST_POSTS*2 ) {
						break;
					}
				}
				tests::detail::periodic_timer_stop();
			}
		};
		int ret;
		//First push 5 items
		for( int i = 0; i < N_TEST_POSTS; ++i ) {
			m_sem_irq_get.signal();
		}
		tests::detail::periodic_timer_setup(isr_test_handler, 65535);
		//Do loop waits for irq
		int n_signals;
		for(n_signals=0; (ret=m_sem_irq.wait(1000))==ISIX_EOK; ++n_signals) {

		}
		//Check the result
		EXPECT( ret==ISIX_ETIMEOUT );
		EXPECT( n_signals==N_TEST_POSTS );
		//Check get isr result
		EXPECT( irq_get_isr_nposts==N_TEST_POSTS );
		isix::wait_ms(250);
	}
};


extern lest::tests & specification();
MODULE( specification(), module )
