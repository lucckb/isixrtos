/* ------------------------------------------------------------------ */
#include <config.h>
#include <isix.h>
#include <usart_simple.h>
#include <foundation/dbglog.h>
#include <foundation/noncopyable.hpp>
#include <qunit.hpp>
#include <string>
#include <stm32crashinfo.h>
#include "semaphore_test.hpp"
#include "task_tests.hpp"
#include "fifo_test.hpp"
#include "atomic_tests.hpp"
/* ------------------------------------------------------------------ */
namespace {
namespace detail {

   isix::semaphore usem(1, 1);
   void usart_lock() {
       //usem.wait(isix::ISIX_TIME_INFINITE);
   }
   void usart_unlock() {
       //usem.signal();
   }
}}
/* ------------------------------------------------------------------ */
class unit_tests : public isix::task_base
{
	static constexpr auto STACK_SIZE = 4096;
    static constexpr auto TASKDEF_PRIORITY = 0;
	QUnit::UnitTest qunit {QUnit::verbose };
	tests::semaphores sem_test { qunit };
	tests::task_tests task_test { qunit };
	tests::fifo_test fifo_test { qunit };
	tests::atomic_tests atomic_test { qunit };
	//Test heap
	void heap_test() {
		auto ptr1 = isix::isix_alloc( 1 );
		QUNIT_IS_NOT_EQUAL( ptr1, nullptr );
		isix::isix_free( ptr1 );
		int fragments {};
		const int freem =  isix::isix_heap_free( &fragments );
		dbprintf("Free %i frags %i", freem, fragments );
		QUNIT_IS_TRUE( freem > 0 );
	}
	//Test basic tasks
    virtual void main() 
	{
		heap_test();
		atomic_test.run();
//		return ; //FIXME: Disable temporary nexts
		sem_test.run();
		task_test.run();	
		fifo_test.run();
		isix::isix_wait_ms(10);
		isix::isix_shutdown_scheduler();
	}
};

/* ------------------------------------------------------------------ */
int main()
{
#ifdef PDEBUG
    stm32::usartsimple_init( USART2,115200,true, CONFIG_PCLK1_HZ, CONFIG_PCLK2_HZ );
#endif	
    dblog_init_putc_locked( stm32::usartsimple_putc, NULL, detail::usart_lock, detail::usart_unlock );
	dbprintf("-------- BEGIN_TESTS ---------");
	static unit_tests test;
	test.start_thread(4096, 0);
	isix::isix_start_scheduler();
	return 0;
}

/* ------------------------------------------------------------------ */
extern "C" {
//Crash info interrupt handler
	void __attribute__((__interrupt__,naked)) hard_fault_exception_vector(void)
	{
		cm3_hard_hault_regs_dump();
	}
}
/* ------------------------------------------------------------------ */

