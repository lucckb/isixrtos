/*
 * =====================================================================================
 *
 *       Filename:  errno_test.hpp
 *
 *    Description:  Errno tests running
 *
 *        Version:  1.0
 *        Created:  25.03.2015 17:13:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */

#pragma once

namespace QUnit {
	class UnitTest;
}

namespace tests {

	class errno_threadsafe {
	public:
		errno_threadsafe( QUnit::UnitTest& unit_test ) 
			: qunit( unit_test ) 
		{
		}
		void run();
	private:
		QUnit::UnitTest& qunit;
	};

}

