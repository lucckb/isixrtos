/*
 * =====================================================================================
 *
 *       Filename:  fifo_test.h
 *
 *    Description:  Basic fifo testing
 *
 *        Version:  1.0
 *        Created:  11.12.2013 18:55:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#ifndef  fifo_test_INC
#define  fifo_test_INC

#include <isix.h>
#include <vector>

namespace QUnit {
	class UnitTest;
}

namespace tests {

// Basic class functionality test
class fifo_test {
	static constexpr auto TASKDEF_PRIORITY=0;
	static constexpr auto NOT_FROM_IRQ=0;
public:
	//Default qtest size
	static constexpr auto IRQ_QTEST_SIZE = 64;
	//Constructor
	fifo_test( QUnit::UnitTest& unit_test )
	: qunit( unit_test )
	{}
	
	//Run all tests
	void run() {
		base_tests();
		insert_overflow();
		delivery_test( 65535 );
		delivery_test( 1 );
		delivery_test( NOT_FROM_IRQ );
		pop_isr_test( 65535 );
	}
	//Base tests from external task 
	void base_tests();
	//Insert overflow test
	void insert_overflow();
	//Added operation for testing sem from interrupts
	void delivery_test( uint16_t time_irq ); 
	//Pop isr test
	void pop_isr_test( uint16_t timer_val );
	//Interrupt handler
	void interrupt_handler() noexcept;
	//Interrupt pop_isr test
	void interrupt_pop_isr( std::vector<int>& back ) noexcept;
private:
	QUnit::UnitTest &qunit;
	isix::fifo<int> m_irqf { IRQ_QTEST_SIZE/2 };
	volatile size_t m_irq_cnt {};
	volatile int m_last_irq_err {};
};


} //test namespace end


#endif   /* ----- #ifndef fifo_test_INC  ----- */
