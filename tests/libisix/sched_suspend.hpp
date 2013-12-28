/*
 * =====================================================================================
 *
 *       Filename:  sched_suspend.h
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


#ifndef  sched_suspend_INC
#define  sched_suspend_INC

namespace QUnit {
	class UnitTest;
}

namespace tests {

class sched_suspend {
public:
	//Default constructor
	sched_suspend( QUnit::UnitTest& unit_test )
	: qunit( unit_test )
	{
	}
	//Run all tests
	void run() {
		basic_lock();
		task_lock();
	}
private:
	//Basic lock test
	void basic_lock();
	//Advanced lock tests
	void task_lock();
private:
	QUnit::UnitTest &qunit;
};

}
#endif   /* ----- #ifndef sched_suspend_INC  ----- */
