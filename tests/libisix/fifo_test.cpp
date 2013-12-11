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
	QUNIT_IS_EQUAL( isix::isix_task_change_prio(nullptr,TASKDEF_PRIORITY ), test_prio );
	for( tbcnt=0; (err=fifo_tst.pop( ch, 1000 ))==isix::ISIX_EOK; tbuf[tbcnt++]=ch );
	QUNIT_IS_EQUAL( err, isix::ISIX_ETIMEOUT );
	QUNIT_IS_EQUAL( tbcnt, task_test::N_ITEMS * 4 );
	QUNIT_IS_FALSE( std::strcmp(tbuf, "DDDDCCCCBBBBAAAA") );
}

//Overflow semaphore test
void fifo_test::insert_overflow() 
{	
	static constexpr auto FIFO_SIZE = 64;
	isix::fifo<char> ovfifo( FIFO_SIZE );
	QUNIT_IS_TRUE( ovfifo.is_valid() );
	int err {};
	for( size_t i = 0; i<FIFO_SIZE; ++i ) {
		err = ovfifo.push('A');
		if( err != isix::ISIX_EOK )
			break;
	}
	QUNIT_IS_EQUAL( err, isix::ISIX_EOK );
	QUNIT_IS_EQUAL( ovfifo.size(), FIFO_SIZE );
	QUNIT_IS_EQUAL( ovfifo.push('X'), isix::ISIX_EFIFOFULL );
}

} //Test Ns end
