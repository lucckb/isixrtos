/*
 * =====================================================================================
 *
 *       Filename:  semaphore_test.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09.12.2013 22:56:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef  semaphore_test_INC
#define  semaphore_test_INC


/* ------------------------------------------------------------------ */
namespace QUnit {
	class UnitTest;
}
/* ------------------------------------------------------------------ */
namespace tests {

/* ------------------------------------------------------------------ */
/* Basic semaphore test class */
class semaphores {
    static constexpr auto TASKDEF_PRIORITY = 0;
public:
	//Constructor 
	semaphores( QUnit::UnitTest &unit_test )
	: qunit( unit_test)
	{}
	void run() {
		semaphore_prio_tests();
		semaphore_time_test();
	}
private:
	void semaphore_prio_tests();
	void semaphore_time_test();
private:
	QUnit::UnitTest& qunit;
};

/* ------------------------------------------------------------------ */
}

/* ------------------------------------------------------------------ */



#endif   /* ----- #ifndef semaphore_test_INC  ----- */
