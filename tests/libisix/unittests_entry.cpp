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
#include <foundation/sys/dbglog.h>
#include <foundation/algo/noncopyable.hpp>
#include <periph/drivers/serial/uart_early.hpp>
#include <boot/arch/arm/cortexm/crashinfo.h>
#include <periph/drivers/serial/uart_early.hpp>
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
	static isix::semaphore m_ulock_sem { 1, 1 };
    isix::wait_ms( 500 );
	dblog_init_locked(
		[](int ch, void*) {
			return periph::drivers::uart_early::putc(ch);
		},
		nullptr,
		[]() {
			m_ulock_sem.wait(ISIX_TIME_INFINITE);
		},
		[]() {
			m_ulock_sem.signal();
		},
		periph::drivers::uart_early::open,
		"serial0", 115200
	);
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

//! Application crash called from hard fault
void application_crash( crash_mode type, unsigned long* sp )
{
#ifdef PDEBUG
	cortex_cm3_print_core_regs( type, sp );
#else
	static_cast<void>(type);
	static_cast<void>(sp);
#endif
	for(;;) asm volatile("wfi\n");
}


//! Handle crash handlers
extern "C" {
	//Crash info interrupt handler
	void __attribute__((__interrupt__,naked)) hard_fault_exception_vector(void)
	{
		_cm3_hard_hault_entry_fn( application_crash );
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

