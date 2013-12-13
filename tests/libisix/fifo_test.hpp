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


namespace QUnit {
	class UnitTest;
}

namespace tests {

// Basic class functionality test
class fifo_test {
	static constexpr auto TASKDEF_PRIORITY=0;
public:
	fifo_test( QUnit::UnitTest& unit_test )
	: qunit( unit_test )
	{}
	
	//Run all tests
	void run() {
		base_tests();
		insert_overflow();
		interrupt_test();
			//Namespace for timer ;
	}
	//Base tests from external task 
	void base_tests();
	//Insert overflow test
	void insert_overflow();
	//Added operation for testing sem from interrupts
	void interrupt_test();
	//Interrupt handler
	void interrupt_handler() noexcept;
private:
	QUnit::UnitTest &qunit;
};


} //test namespace end


#endif   /* ----- #ifndef fifo_test_INC  ----- */
