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
#include <config.h>

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
#ifdef ISIX_CONFIG_CPU_USAGE_API
			cpuload_test();
#endif
			basic_funcs();
			thread_test();
		}
		//Basic functionality testing 
		void basic_funcs();
		// CPU load test
		void cpuload_test();
		// CPP11 thread API 
		void thread_test();
	private:
		QUnit::UnitTest& qunit;
	};

}

