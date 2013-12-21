/*
 * =====================================================================================
 *
 *       Filename:  spinlock_test.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  21.12.2013 20:40:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#ifndef  spinlock_test_INC
#define  spinlock_test_INC

namespace QUnit {
	class UnitTest;
}

namespace tests {

class spinlock_tests {
public:
	//Default constructor
	spinlock_tests( QUnit::UnitTest& unit_test )
	: qunit( unit_test )
	{
	}
	//Run all tests
	void run() {
		scheduler_api();
		basic_test();
	}
private:
	void scheduler_api();
	void basic_test();
private:
	QUnit::UnitTest &qunit;
};

}
#endif   /* ----- #ifndef spinlock_test_INC  ----- */
