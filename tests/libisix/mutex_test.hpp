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
#if 0
		void run() {
			test01();
			test02();
			test03();
			test04();
			test05();
			test06();
			test07();
			test08();
		}
#else
		void run() {
			//test09();
			test10();
		}
#endif
	private:
		void test01();
		void test02();
		void test03();
		void test04();
		void test05();
		void test06();
		void test07();
		void test08();
		void test09();
		void test10();
	private:
		QUnit::UnitTest& qunit;
	};
}

