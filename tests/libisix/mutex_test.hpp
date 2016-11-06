/*
 * =====================================================================================
 *
 *       Filename:  mutex_test.hpp
 *
 *    Description:  Mutex test group
 *
 *        Version:  1.0
 *        Created:  04.11.2016 21:33:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <isix.h>
#include <vector>

namespace QUnit {
	class UnitTest;
}
namespace tests {
	class mutexes {
	public:
		mutexes( QUnit::UnitTest& ut )
			: qunit( ut )
		{
		}
		void run() {
			test01();
			test02();
			test03();
			test04();
			test05();
		}
	private:
		void test01();
		void test02();
		void test03();
		void test04();
		void test05();
	private:
		QUnit::UnitTest& qunit;
	};
}

