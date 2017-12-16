/*
 * =====================================================================================
 *
 *       Filename:  unittests_entry.cpp
 *
 *    Description:  Unit test starting entry point
 *
 *        Version:  1.0
 *        Created:  22.06.2017 18:39:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <config/conf.h>
#include <isix.h>
#include <usart_simple.h>
#include <foundation/sys/dbglog.h>
#include <foundation/algo/noncopyable.hpp>
#include <stm32crashinfo.h>
#include <string>
#include <sys/stat.h>
#include <lest/lest.hpp>

//! Global symbol for enable printf floating point support
//asm (".global _printf_float");

// Global object for test specification register
lest::tests& specification()
{
	static lest::tests tests;
	return tests;
}

/*
	//vStartEventGroupTasks();
	//isix::wait_ms(500);
	//tests::detail::periodic_timer_setup( vPeriodicEventGroupsProcessing );
*/

//! Unit tests main thread
static void unittests_thread(void*)
{
	try {
		lest::run( specification(), { "-c" } );
		//int code = lest::run( specification(), {"--", "06_task_10" } );
		const int code = lest::run( specification(), {} );
		{
			isix::memory_stat mstat;
			isix::heap_stats( mstat );
			dbprintf("Free stack space %u\n", isix::free_stack_space() );
			dbprintf("Heap free %u used %u\n", mstat.free, mstat.used );
			dbprintf("Unit test finished with code %i\n", code );
			isix::wait_ms( 100 );
			isix::shutdown_scheduler();
		}
	} catch( const std::exception& e ) {
		dbprintf("Unhandled std::exception %s", e.what() );
		return;
	} catch( ... ) {
		dbprintf("Unhandled unknown exception");
		return;
	}
}

// Main core function
int main()
{
	//static constexpr auto baud_hi = 3000000;
	static constexpr auto baud_lo = 115200;
    stm32::usartsimple_init( USART1, baud_lo ,false, CONFIG_PCLK1_HZ, CONFIG_PCLK2_HZ );
	dblog_init_putc( stm32::usartsimple_putc, nullptr );
	dbprintf("ISIX VERSION %s", isix::get_version() );
	const auto hwnd =
		isix::task_create( unittests_thread, nullptr, 8192, 0, isix_task_flag_newlib);
	if( !hwnd ) {
		dbprintf("Unable to create task");
		return -1;
	}
	isix_start_scheduler();
	dbprintf("Main exit\n");
	return 0;
}


//! Handle crash handlers
extern "C" {
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
}

//! Extra functions for stdlib support
extern "C" {
	int _gettimeofday (struct timeval *tp, void *) {
		const auto j = isix::get_ujiffies();
		tp->tv_sec = j / 1000'000;
		tp->tv_usec = j % 1000'000;
		return 0;
	}
}

