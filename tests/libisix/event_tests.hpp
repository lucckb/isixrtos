/*
 * =====================================================================================
 *
 *       Filename:  event_tests.hpp
 *
 *    Description:  EVents test API
 *
 *        Version:  1.0
 *        Created:  08.04.2015 18:29:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

namespace QUnit {
	class UnitTest;
}

namespace tests {

	//! Tasks events test
	class event_test {
	public:
		//! Constructor
		event_test( QUnit::UnitTest& unit_test )
			: qunit( unit_test )
		{}
		//! Run the tests
		void run() {
			//base_test();
			sync_test();
		}
	private:
		void base_test();
		void sync_test();
		QUnit::UnitTest &qunit;
	};

}
