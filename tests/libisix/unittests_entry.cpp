#include <config/conf.h>
#include <isix.h>
#include <foundation/sys/dbglog.h>
#include <foundation/algo/noncopyable.hpp>
#include <periph/drivers/serial/uart_early.hpp>
#include <boot/arch/arm/cortexm/crashinfo.h>
#include <periph/drivers/serial/uart_early.hpp>
#include <isix/arch/irq_global.h>
#include <string>
#include <sys/stat.h>
#define UNITY_FIXTURE_NO_EXTRAS
#include <unity.h>
#include <unity_fixture.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <sys/lock.h>

//! Global symbol for enable printf floating point support
//asm (".global _printf_float");

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#define ENTRY_EXCEPTIONS 1
#else
#define ENTRY_EXCEPTIONS 0
#endif

//! Unit tests main thread
static void unittests_thread(void*)
{
#if ENTRY_EXCEPTIONS
	try {
#endif
		UNITY_BEGIN();
		RUN_TEST_GROUP(basic_primitives);
		RUN_TEST_GROUP(mempool);
		RUN_TEST_GROUP(mutex);
		RUN_TEST_GROUP(sched_suspend);
		RUN_TEST_GROUP(semaphores);
		RUN_TEST_GROUP(tasks);
		RUN_TEST_GROUP(vtimer);
		RUN_TEST_GROUP(fifo);
		int code = UNITY_END();

		isix::memory_stat mstat;
		isix::heap_stats( mstat );
		dbprintf("Free stack space %u\n", isix::free_stack_space() );
		dbprintf("Heap free %u used %u\n", mstat.free, mstat.used );
		dbprintf("Unit test finished with code %i\n", code );
		isix::wait_ms( 100 );
		isix::shutdown_scheduler();
#if ENTRY_EXCEPTIONS
	} catch( const std::exception& e ) {
		dbprintf("Unhandled std::exception %s", e.what() );
		return;
	} catch( ... ) {
		dbprintf("Unhandled unknown exception");
		return;
	}
#endif
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
            if (!isix_irq_in_isr()) {
                m_ulock_sem.wait(ISIX_TIME_INFINITE);
            }
		},
		[]() {
            if (!isix_irq_in_isr()) {
                m_ulock_sem.signal();
            }
		},
		periph::drivers::uart_early::open,
		"serial0", 115200
	);
	dbprintf("ISIX VERSION %s", isix::get_version() );
	const auto hwnd =
		isix::task_create( unittests_thread, nullptr, 16U*1024U, 0, isix_task_flag_newlib);
	if( !hwnd ) {
		dbprintf("Unable to create task");
		return -1;
	}
	isix_start_scheduler();
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
	//Crash info interrupt handler
ISIX_ISR_NACKED_VECTOR(hard_fault_exception_vector)
{
    _cm3_hard_hault_entry_fn( application_crash );
}

//Isix panic callback
extern "C" void isix_kernel_panic_callback( const char* file, int line, const char *msg )
{
    fnd::tiny_printf("ISIX_PANIC %s:%i %s\r\n", file, line, msg );
}

//! Extra function for stdlib support
extern "C" int _gettimeofday(struct timeval* tp, void*)
{
    const auto j = isix::get_ujiffies();
    tp->tv_sec = j / 1000'000;
    tp->tv_usec = j % 1000'000;
    return 0;
}
