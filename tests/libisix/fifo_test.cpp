/*
 * =====================================================================================
 *
 *       Filename:  fifo_test.cpp
 *
 *    Description:  Fifo basic testing
 *
 *        Version:  1.0
 *        Created:  11.12.2013 18:53:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#include "fifo_test.hpp"
#include <isix.h>
#include <qunit.hpp>
#include <vector>
#include <foundation/dbglog.h>
#include "timer_interrupt.hpp" 

namespace tests {

namespace {
	class task_test : public isix::task_base 
	{
	static constexpr auto STACK_SIZE = 1024;
	public:
		//Number of items per task
		static constexpr auto N_ITEMS = 4;
		//Constructor
		task_test( char ch_id, isix::prio_t prio, isix::fifo<char> &fifo ) 
			: task_base( STACK_SIZE, prio ), 
            m_fifo( fifo ), m_id( ch_id )
		{
		}
		virtual ~task_test() {}
	protected:
		//Main function from another task
		virtual void main( void ) 
		{
			for( int i = 0; i< N_ITEMS; ++i ) {
				m_error = m_fifo.push( m_id );
				if( m_error ) break;
			}
		}
	private:
		isix::fifo<char>& m_fifo; 
		const char m_id;
		int m_error { -32768 };
	};

}	//Unnamed namespace end


//Delivery class task
namespace {
	
	//Overflow task for testing the push
	class overflow_task : public isix::task_base 
	{
	static constexpr auto STACK_SIZE = 1024;
	public:
		//Number of items per task
		static constexpr auto N_ITEMS = 4;
		//Constructor
		overflow_task( isix::prio_t prio, isix::fifo<int> &fifo ) 
			: task_base( STACK_SIZE, prio ), m_fifo( fifo )
		{
		}
		//Destructor
		virtual ~overflow_task() {}
		//Error code
		int error() const {
			return m_error;
		}
	protected:
		//Main function from another task
		virtual void main() 
		{
			for( int i=0; i < fifo_test::IRQ_QTEST_SIZE; ++i ) {
				m_error = m_fifo.push(i+1);
				if( m_error != isix::ISIX_EOK ) {
					break;
				}
			}
		}
	private:
		isix::fifo<int> &m_fifo;
		int m_error { -32768 };
	};
}

//Unnamend namespace
namespace {

	//Verify incremental context
	template <typename T> 
		bool verify_values( const std::vector<T>& vec, int scnt ) 
		{
			for( auto v : vec ) {
				if( v != scnt ) {
					dbprintf("Failed at  %i!=%i", v, scnt );
					return false;
				}
				++scnt;
			}
			return true;
		}
}
//Base tests from external task 
void fifo_test::base_tests() 
{
	static constexpr auto test_prio = 3;
	QUNIT_IS_EQUAL( isix::isix_task_change_prio( nullptr, test_prio ), TASKDEF_PRIORITY );	        
	isix::fifo<char> fifo_tst( 32 );
	QUNIT_IS_TRUE( fifo_tst.is_valid() );  
	  
	task_test t1( 'A',  3, fifo_tst );
	task_test t2( 'B',  2, fifo_tst );
	task_test t3( 'C',  1, fifo_tst );
	task_test t4( 'D',  0, fifo_tst );

	char tbuf[17] {};
	size_t tbcnt {};
	int err;
	char ch;
	for( tbcnt=0; (err=fifo_tst.pop( ch, 1000 ))==isix::ISIX_EOK; tbuf[tbcnt++]=ch );
	QUNIT_IS_EQUAL( err, isix::ISIX_ETIMEOUT );
	QUNIT_IS_EQUAL( tbcnt, task_test::N_ITEMS * 4 );
	QUNIT_IS_FALSE( std::strcmp(tbuf, "DDDDCCCCBBBBAAAA") );
	QUNIT_IS_EQUAL( isix::isix_task_change_prio(nullptr,TASKDEF_PRIORITY ), test_prio );
}

//Overflow semaphore test
void fifo_test::insert_overflow() 
{	
	/* 
	 * FIXME: It must be fixed when tasks are deleted and the next the taks are created
	 * it seems like bug in the OS but not critical
	 * Probably the reason is that the task have not been deleted yet but
	 * sempaphore was deleted so race condition occurs
	 */
	isix::isix_wait_ms(10);
	static constexpr auto FIFO_SIZE = 64;
	isix::fifo<char> ovfifo( FIFO_SIZE );
	QUNIT_IS_TRUE( ovfifo.is_valid() );
	int err {};
	for( size_t i = 0; i<FIFO_SIZE; ++i ) {
		err = ovfifo.push('A');
		if( err != isix::ISIX_EOK )
			break;
	}
	char ch;
	QUNIT_IS_EQUAL( err, isix::ISIX_EOK );
	QUNIT_IS_EQUAL( ovfifo.size(), FIFO_SIZE );
	QUNIT_IS_EQUAL( ovfifo.push('X',1000), isix::ISIX_ETIMEOUT );
	QUNIT_IS_EQUAL( ovfifo.push('X',1000), isix::ISIX_ETIMEOUT );
	QUNIT_IS_EQUAL( ovfifo.pop(ch), isix::ISIX_EOK );
	QUNIT_IS_EQUAL( ch, 'A' );
	QUNIT_IS_EQUAL( ovfifo.size(), FIFO_SIZE -1 );
}  

//Interrupt handler
void fifo_test::interrupt_handler() noexcept
{
	if( m_irq_cnt++ < IRQ_QTEST_SIZE ) {
		m_last_irq_err  = m_irqf.push_isr( m_irq_cnt );
		if (m_last_irq_err != isix::ISIX_EOK ) {
			detail::periodic_timer_stop();
		}
	} else {
		detail::periodic_timer_stop();
	}
}

//Interrupt pop_isr test
void fifo_test::interrupt_pop_isr( std::vector<int>& back ) noexcept 
{
	int elem { -1 };
	if( m_irqf.pop_isr( elem ) == isix::ISIX_EOK ) {
		back.push_back( elem );
	}
}

//Pop isr test
void fifo_test::pop_isr_test( uint16_t timer_val ) 
{
	std::vector<int> back_vect;
	detail::periodic_timer_setup( 
		std::bind( &fifo_test::interrupt_pop_isr, std::ref(*this),
			std::ref(back_vect) ), timer_val
	);
	static constexpr int test_val = 1280;
	int error;
	for(int p = 0; p < IRQ_QTEST_SIZE; ++p ) {
		error = m_irqf.push( p + test_val );	
		if( error ) break;
	}
	QUNIT_IS_EQUAL( error, isix::ISIX_EOK );
	isix::isix_wait_ms( 100 );
	QUNIT_IS_EQUAL( back_vect.size(), IRQ_QTEST_SIZE );
	QUNIT_IS_TRUE( verify_values( back_vect, test_val ) );
}

	
//Added operation for testing sem from interrupts
void fifo_test::delivery_test( uint16_t time_irq ) 
{
	dbprintf("Begin delivery test value [%i]", time_irq );
	overflow_task* task {};
	if( time_irq != NOT_FROM_IRQ ) {
		m_irq_cnt = 0;
		detail::periodic_timer_setup(
			std::bind( &fifo_test::interrupt_handler, std::ref(*this) ), 65535
		);
	} else {
		task = new overflow_task( isix::isix_get_task_priority(nullptr), m_irqf );
	}
	int val; int ret;
	std::vector<int> test_vec;
	for( int n=0; (ret=m_irqf.pop(val, 1000)) == isix::ISIX_EOK; ++n ) {
		test_vec.push_back( val );
	}
	QUNIT_IS_EQUAL( ret, isix::ISIX_ETIMEOUT );
	QUNIT_IS_EQUAL( m_irqf.size(), 0 );
 
	//Final parameter check
	QUNIT_IS_EQUAL( test_vec.size(), IRQ_QTEST_SIZE );
	QUNIT_IS_TRUE( verify_values( test_vec, 1 ) );
	QUNIT_IS_EQUAL( m_last_irq_err, isix::ISIX_EOK );
		detail::periodic_timer_stop();
	if( time_irq != NOT_FROM_IRQ ) {
		detail::periodic_timer_stop();
	} else {
		//TODO: Fixme delete crap related to free resources
	//	delete task;
	}
}

}//Test Ns end
