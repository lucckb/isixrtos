/*
 * =====================================================================================
 *
 *       Filename:  task_tests.hpp
 *
 *    Description: Basic task testing 
 *
 *
 *        Version:  1.0
 *        Created:  10.12.2013 00:03:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *   Organization: Boff
 *
 * =====================================================================================
 */

/* ------------------------------------------------------------------ */
#ifndef  task_tests_INC
#define  task_tests_INC

/* ------------------------------------------------------------------ */
namespace QUnit {
	class UnitTest;
}
/* ------------------------------------------------------------------ */
namespace tests {
	
/* ------------------------------------------------------------------ */
class task_tests {
	static constexpr auto MIN_STACK_FREE = 64;
public:
	//Constructor
	task_tests( QUnit::UnitTest &unit_test )
	: qunit( unit_test)
	{}
	//Run all tests
	void run() {
		basic_funcs();
	}
	//Basic functionality testing 
	void basic_funcs();
private:
	QUnit::UnitTest& qunit;
};
/* ------------------------------------------------------------------ */
}

#endif   /* ----- #ifndef task_tests_INC  ----- */
