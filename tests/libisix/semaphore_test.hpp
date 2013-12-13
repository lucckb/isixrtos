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

#include <isix.h>

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
		from_interrupt();
	}
private:
	void semaphore_prio_tests();
	void semaphore_time_test();
	void isr_test_handler();
	//Semaphore from interrupts
	void from_interrupt();
private:
	//Isix interrupt semaphore
	isix::semaphore m_sem_irq { 0, 0 };
	isix::semaphore m_sem_irq_get { 0, 0 };
	volatile int irq_get_isr_nposts = 0;
	volatile int test_count { }; //Post IRQ sem five times
	QUnit::UnitTest& qunit;
};

/* ------------------------------------------------------------------ */
}

/* ------------------------------------------------------------------ */



#endif   /* ----- #ifndef semaphore_test_INC  ----- */
