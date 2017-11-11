/*
 * =====================================================================================
 *
 *       Filename:  fifo_test.cpp
 *
 *    Description:  Base fifo testing
 *
 *        Version:  1.0
 *        Created:  23.06.2017 20:46:27
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
#include <memory>


namespace {

	constexpr auto c_stack_margin = 100;
	class task_test
	{
	static constexpr auto STACK_SIZE = 1024;
	public:
		//Number of items per task
		static constexpr auto N_ITEMS = 4;
		//Constructor
		task_test( char ch_id, osprio_t prio, isix::fifo<char> &fifo )
			: m_fifo( fifo ), m_id( ch_id ), m_prio( prio )
			, m_thr( isix::thread_create( std::bind(&task_test::thread,std::ref(*this))))
		{
		}
		task_test( task_test& ) = delete;
		task_test& operator=( task_test& ) = delete;
		void start() {
			m_thr.start_thread(STACK_SIZE, m_prio);
		}
	private:
		//Main function from another task
		void thread( ) noexcept
		{
			//isix_wait_ms(100);
			for( int i = 0; i< N_ITEMS; ++i ) {
				m_error = m_fifo.push( m_id );
				if( m_error ) break;
			}
		}
	private:
		isix::fifo<char>& m_fifo;
		const char m_id;
		int m_error { -32768 };
		osprio_t m_prio;
		isix::thread m_thr;
	};

}	//Unnamed namespace end


//Delivery class task
namespace {
	
	static constexpr auto IRQ_QTEST_SIZE = 64U;
	//Overflow task for testing the push
	class overflow_task
	{
	static constexpr auto STACK_SIZE = 2048;
	public:
		//Number of items per task
		static constexpr auto N_ITEMS = 4;
		//Constructor
		overflow_task( osprio_t prio, isix::fifo<int> &fifo )
			:  m_fifo( fifo ), m_prio( prio )
			, m_thr( isix::thread_create( std::bind(&overflow_task::thread,std::ref(*this))))
		{
		}
		//Error code
		int error() const {
			return m_error;
		}
		//Start function
		void start() {
			m_thr.start_thread( STACK_SIZE, m_prio );
		}
		const auto tid() const noexcept {
			return m_thr.tid();
		}
	private:
		//Main function from another task
		void thread() noexcept
		{
			for( auto i=0U; i < IRQ_QTEST_SIZE; ++i ) {
				m_error = m_fifo.push(i+1);
				if( m_error != ISIX_EOK ) {
					break;
				}
			}
		}
	private:
		isix::fifo<int> &m_fifo;
		int m_error { -32768 };
		osprio_t m_prio;
		isix::thread m_thr;
	};
}


namespace {
	constexpr auto NOT_FROM_IRQ=0;
	constexpr auto TASKDEF_PRIORITY=0;
	isix::fifo<int> m_fifo_n { IRQ_QTEST_SIZE/2 };
	isix::fifo<int> m_fifo_noirq { IRQ_QTEST_SIZE/2, isix_fifo_f_noirq };
	volatile size_t m_irq_cnt {};
	volatile int m_last_irq_err {};

	//Verify incremental context
	template <typename T>
		bool verify_values( const std::vector<T>& vec, int scnt )
		{
			for( auto v : vec ) {
				if( v != scnt ) {
					return false;
				}
				++scnt;
			}
			return true;
		}
	void interrupt_handler( isix::fifo<int>& fifo ) noexcept
	{
		if( m_irq_cnt++ < IRQ_QTEST_SIZE ) {
			m_last_irq_err  = fifo.push_isr( m_irq_cnt );
			if (m_last_irq_err != ISIX_EOK ) {
				tests::detail::periodic_timer_stop();
			}
		} else {
			tests::detail::periodic_timer_stop();
		}
	}


	void delivery_test( lest::env& lest_env, uint16_t time_irq, isix::fifo<int>& fifoin )
	{
		std::unique_ptr<overflow_task> task;
		if( time_irq != NOT_FROM_IRQ ) {
			m_irq_cnt = 0;
			tests::detail::periodic_timer_setup(
				std::bind(interrupt_handler,std::ref(m_fifo_n)), time_irq
			);
		} else {
			task.reset( new overflow_task( isix_get_task_priority(nullptr), fifoin) );
			task->start();
		}
		int val; int ret;
		std::vector<int> test_vec;
		for( int n=0; (ret=fifoin.pop(val,1000))==ISIX_EOK; ++n ) {
			test_vec.push_back( val );
		}
		EXPECT( ret==ISIX_ETIMEOUT );
		EXPECT( fifoin.size() == 0 );

		//Final parameter check
		EXPECT( test_vec.size() == IRQ_QTEST_SIZE );
		EXPECT( verify_values( test_vec, 1 ) );
		EXPECT( m_last_irq_err == int(ISIX_EOK) );
		tests::detail::periodic_timer_stop();
		if( time_irq != NOT_FROM_IRQ ) {
			tests::detail::periodic_timer_stop();
		}
	}
}

static const lest::test module[] =
{
	CASE("08_fifo_01 Fifo basic delivery")
	{
		int err;
		char ch;
		isix::fifo<char> fifo_tst( 32 );
		EXPECT( fifo_tst.is_valid() );
		task_test t1( 'A', 3, fifo_tst );
		task_test t2( 'B', 2, fifo_tst );
		task_test t3( 'C', 1, fifo_tst );
		task_test t4( 'D', 0, fifo_tst );
		t1.start(); t2.start(); t3.start(); t4.start();
		std::string tbuf;
		for( ; (err=fifo_tst.pop( ch, 1000 ))==ISIX_EOK; tbuf+=ch );
		EXPECT( err==ISIX_ETIMEOUT );
		EXPECT( tbuf.size() == task_test::N_ITEMS * 4U );
		EXPECT( tbuf=="DDDDCCCCBBBBAAAA" );
		EXPECT( isix::free_stack_space( nullptr ) > c_stack_margin );
		isix::wait_ms(20);
	},
	CASE("08_fifo_02 Fifo insert overflow check")
	{
		static constexpr auto FIFO_SIZE = 64;
		isix::fifo<char> ovfifo( FIFO_SIZE );
		EXPECT( ovfifo.is_valid() );
		int err {};
		for( size_t i = 0; i<FIFO_SIZE; ++i ) {
			err = ovfifo.push('A');
			if( err != ISIX_EOK )
				break;
		}
		char ch;
		EXPECT( err == ISIX_EOK );
		EXPECT( ovfifo.size() == FIFO_SIZE );
		EXPECT( ovfifo.push('X',1000) == ISIX_ETIMEOUT );
		EXPECT( ovfifo.push('X',1000) == ISIX_ETIMEOUT );
		EXPECT( ovfifo.pop(ch) == ISIX_EOK );
		EXPECT( ch ==  'A' );
		EXPECT( ovfifo.size() == FIFO_SIZE -1 );
		EXPECT( isix::free_stack_space( nullptr ) > c_stack_margin );
	},
	CASE( "08_fifo_03 Fifo irq slow delivery check" )
	{
		delivery_test(lest_env, 65535 , m_fifo_n );
		EXPECT( isix::free_stack_space( nullptr ) > c_stack_margin );
	},
	CASE( "08_fifo_04 Fifo irq fast delivery check" )
	{
		delivery_test( lest_env, 4000, m_fifo_n );
		EXPECT( isix::free_stack_space( nullptr ) > c_stack_margin );
	},
	CASE( "08_fifo_05 Fifo noirq delivery check" )
	{
		delivery_test( lest_env, NOT_FROM_IRQ, m_fifo_n );
		EXPECT( isix::free_stack_space( nullptr ) > c_stack_margin );
	},
	CASE( "08_fifo_06 Slow fifo after noirq" )
	{
		delivery_test(lest_env, 65535, m_fifo_n );
		EXPECT( isix::free_stack_space( nullptr ) > c_stack_margin );
	},
	CASE( "08_fifo_07 Not from irq exchanged" )
	{
		delivery_test(lest_env, NOT_FROM_IRQ, m_fifo_noirq );
		EXPECT( isix::free_stack_space( nullptr ) > c_stack_margin );
	},
};


extern lest::tests & specification();
MODULE( specification(), module )
