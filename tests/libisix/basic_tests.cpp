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
#include "sched_suspend.hpp"
#include "mempool_test.hpp"
#include "vtimer_test.hpp"
#include "errno_test.hpp"
#include "event_tests.hpp"
#include "event_groups_demo.h"
#include "timer_interrupt.hpp"
#include <stm32adc.h>
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
	tests::sched_suspend sched_test { qunit };
	tests::mempool mempool_test { qunit };
	tests::vtimer timer_test { qunit };
	tests::errno_threadsafe errno_test { qunit };
	tests::event_test event_test { qunit };
	//Test heap
	void heap_test() {
		QUNIT_IS_EQUAL( reinterpret_cast<long>(this)%4, 0 );
		auto ptr1 = isix_alloc( 1 );
		auto ptr2 = isix_alloc( 1 );
		QUNIT_IS_NOT_EQUAL( ptr1, nullptr );
		QUNIT_IS_EQUAL( reinterpret_cast<long>(ptr1)%ISIX_CONFIG_BYTE_ALIGNMENT_SIZE , 0 );
		QUNIT_IS_NOT_EQUAL( ptr2, nullptr );
		QUNIT_IS_EQUAL( reinterpret_cast<long>(ptr2)%ISIX_CONFIG_BYTE_ALIGNMENT_SIZE , 0 );
		isix_free( ptr1 );
		isix_free( ptr2 );
		int fragments {};
		const int freem =  isix_heap_free( &fragments );
		dbprintf("Free %i frags %i", freem, fragments );
		QUNIT_IS_TRUE( freem > 0 );
	}
	//Test basic tasks
    virtual void main() 
	{
#if 0
		timer_test.run();
		dbprintf("Timer tests end");
		isix::wait_ms(500);
		isix::shutdown_scheduler();
		return;
#endif
#if 1
		heap_test();
		atomic_test.run();
		sched_test.run();
		sem_test.run();
		task_test.run();	
		fifo_test.run();
		mempool_test.run();
		timer_test.run();
		errno_test.run();
		event_test.run();
		isix_wait_ms(100);
		dbprintf("Bye bye");
		isix_shutdown_scheduler();
		dbprintf("It never should show it");
#else
		vStartEventGroupTasks();
		isix::wait_ms(500);
		tests::detail::periodic_timer_setup( vPeriodicEventGroupsProcessing );
#endif
	}
};

/* ------------------------------------------------------------------ */
int main()
{
#ifdef PDEBUG
	//static constexpr auto baud_hi = 3000000;
	static constexpr auto baud_lo = 115200;
    stm32::usartsimple_init( USART2, baud_lo ,true, CONFIG_PCLK1_HZ, CONFIG_PCLK2_HZ );
#endif	
	dblog_init_putc( stm32::usartsimple_putc, nullptr );
	dbprintf("-------- BEGIN_TESTS ---------");
	static unit_tests test;
	test.start_thread(4096, 0);
	isix_start_scheduler();
	return 0;
}

/* ------------------------------------------------------------------ */
extern "C" {
#ifdef PDEBUG
	//Crash info interrupt handler
	void __attribute__((__interrupt__,naked)) hard_fault_exception_vector(void)
	{
		cm3_hard_hault_regs_dump();
	}
	//Isix panic callback
	void isix_kernel_panic_callback( const char* file, int line, const char *msg )
	{
		fnd::tiny_printf("ISIX_PANIC %s:%i %s\r\n", file, line, msg );
	}
#endif
}
/* ------------------------------------------------------------------ */

