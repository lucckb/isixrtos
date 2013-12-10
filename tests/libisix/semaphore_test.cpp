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
#include <stm32system.h>
#include <stm32tim.h>
#include <stm32rcc.h>
namespace tests {

/* ------------------------------------------------------------------ */
namespace {
	//Isix interrupt semaphore
	isix::semaphore sem_irq( 0, 0 );
	isix::semaphore sem_irq_get( 0, 0 );
	int irq_get_isr_nposts = 0;
	constexpr auto N_TEST_POSTS = 25;
}

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

namespace {

/* ------------------------------------------------------------------ */
//! Interrupt FPU context on the lower possible level
void enable_irq_timers() 
{
    using namespace stm32;
    rcc_apb1_periph_clock_cmd( RCC_APB1Periph_TIM3, true );
    nvic_set_priority( TIM3_IRQn , 1, 1 );
    tim_timebase_init( TIM3, 0, TIM_CounterMode_Up, 65535, 0, 0 );
    tim_it_config( TIM3, TIM_IT_Update, true );
    tim_cmd( TIM3, true );
    nvic_irq_enable( TIM3_IRQn, true );
}

/* ------------------------------------------------------------------ */
//! Disable IRQ timers
void disable_irq_timers() {
	using namespace stm32;
	//Disable IRQ timers
    tim_it_config( TIM3, TIM_IT_Update, false);
    nvic_irq_enable( TIM3_IRQn, false);
}
/* ------------------------------------------------------------------ */
 //Interrrupt handlers
extern "C" {
    //TIM3 base initial IRQ
	void __attribute__((interrupt)) tim3_isr_vector() 
	{
		//Post IRQ sem five times
		static int test_count = 0;
		stm32::tim_clear_it_pending_bit( TIM3, TIM_IT_Update );
		if( test_count++ < N_TEST_POSTS ) {
        	sem_irq.signal_isr();
		} else {
			disable_irq_timers();
			while( sem_irq_get.get_isr() == isix::ISIX_EOK ) {
				++irq_get_isr_nposts;
			}
		}
    }
}

/* ------------------------------------------------------------------ */
} 	//
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
//Semaphore from interrupts
void semaphores::from_interrupt() {
	//First push 5 items 
	for( int i = 0; i < N_TEST_POSTS; ++i ) { 
		sem_irq_get.signal();
	}
	enable_irq_timers();
	int ret;
	//Do loop waits for irq
	int n_signals;
	for(n_signals=0; (ret=sem_irq.wait(1000)) == isix::ISIX_EOK; ++n_signals );
	//Check the result
	QUNIT_IS_EQUAL( ret, isix::ISIX_ETIMEOUT );
	QUNIT_IS_EQUAL( n_signals, N_TEST_POSTS );
	//Check get isr result
	QUNIT_IS_EQUAL( irq_get_isr_nposts, N_TEST_POSTS );
}
/* ------------------------------------------------------------------ */
}	// Namespace tests end
/* ------------------------------------------------------------------ */

