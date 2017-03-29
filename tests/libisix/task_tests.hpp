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

#pragma once
#include <config/conf.h>

namespace QUnit {
	class UnitTest;
}

namespace tests {

	class task_tests {
		static constexpr auto MIN_STACK_FREE = 64;
	public:
		//Constructor
		task_tests( QUnit::UnitTest &unit_test )
		: qunit( unit_test)
		{}
		//Run all tests
		void run() {
#if CONFIG_ISIX_CPU_USAGE_API
			cpuload_test();
#endif
			basic_funcs();
			thread_test();
			thread_test2();
			wait_and_reference_api();
		}
		//Basic functionality testing
		void basic_funcs();
		// CPU load test
		void cpuload_test();
		// CPP11 thread API
		void thread_test();
		// Thread test2
		void thread_test2();
		// Wait and reference API test
		void wait_and_reference_api();
	private:
		QUnit::UnitTest& qunit;
	};

}

