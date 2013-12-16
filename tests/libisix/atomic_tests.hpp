/*
 * =====================================================================================
 *
 *       Filename:  atomic_tests.hpp
 *
 *    Description:  Atomics operation tests
 *
 *        Version:  1.0
 *        Created:  16.12.2013 18:17:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#ifndef  atomic_tests_INC
#define  atomic_tests_INC

namespace QUnit {
	class UnitTest;
}

namespace tests {


class atomic_tests {
	static constexpr auto TASKDEF_PRIORITY = 0;
public:
	//Default constructor
	atomic_tests( QUnit::UnitTest& unit_test )
	: qunit( unit_test )
	{}

	//Run all tests
	void run() {
	atomic_sem_test( );	
	}
private:
	//Atomic semaphore primitives test
	void atomic_sem_test( );
private:
	QUnit::UnitTest &qunit;
};

}
#endif   /* ----- #ifndef atomic_tests_INC  ----- */
