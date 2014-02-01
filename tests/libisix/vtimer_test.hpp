/*
 * =====================================================================================
 *
 *       Filename:  vtimer_test.hpp
 *
 *    Description:  VTIMER test
 *
 *        Version:  1.0
 *        Created:  02.01.2014 23:02:58
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#ifndef  vtimer_test_INC
#define  vtimer_test_INC

#include <isix.h>
#include <foundation/noncopyable.hpp>

namespace QUnit {
	class UnitTest;
}

namespace tests {

class vtimer : public fnd::noncopyable
{
public:
	vtimer( QUnit::UnitTest& unit_test )
	: qunit( unit_test )
	{
	}
	//Run all test
	void run() {
		basic();
		one_shoot();
	}
protected:
private:
	//Basic test
	void basic();
	//One shoot test
	void one_shoot();
private:
	class timer : public isix::virtual_timer {
	public:
		//Counter get
		unsigned counter() const {
			return m_counter;
		}
	protected:
		virtual void handle_timer() 
		{
			++m_counter;
		}
	private:
		unsigned m_counter {};
	};
private:
	QUnit::UnitTest& qunit;
	timer m_t1;
	timer m_t2;
	timer m_t3;
};


} //Vtimer end
#endif   /* ----- #ifndef vtimer_test_INC  ----- */
