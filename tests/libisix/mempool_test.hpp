/*
 * =====================================================================================
 *
 *       Filename:  mempool_test.hpp
 *
 *    Description:  Mempool test
 *
 *        Version:  1.0
 *        Created:  28.12.2013 17:26:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#ifndef  mempool_test_INC
#define  mempool_test_INC

/* ------------------------------------------------------------------ */
namespace QUnit {
	class UnitTest;
}
/* ------------------------------------------------------------------ */
namespace tests {

//Mempool class tester
class mempool {
public:
	//Constructor
	mempool( QUnit::UnitTest &unit_test ) 
	: qunit( unit_test )
	{}
	//Run method
	void run() {
		mempool_tests();
	}
private:
	void mempool_tests();
private:
	QUnit::UnitTest& qunit;
};

}	//Namespace end test
/* ------------------------------------------------------------------ */
#endif   /* ----- #ifndef mempool_test_INC  ----- */
