/*
 * Board support shared by the coroutine examples: console and crash hooks.
 *
 * Author: Lucjan Bryndza
 */
#include "example_support.hpp"

#include <isix.h>
#include <boot/arch/arm/cortexm/crashinfo.h>
#include <foundation/sys/dbglog.h>
#include <isix/arch/irq_global.h>
#include <isix/arch/isr_vectors.h>
#include <periph/drivers/serial/uart_early.hpp>

namespace examples {

void console_init()
{
	static isix::semaphore lock { 1, 1 };
	dblog_init_locked(
		[](int ch, void*) { return periph::drivers::uart_early::putc(ch); },
		nullptr,
		[]() {
			if (!isix_irq_in_isr()) {
				lock.wait(ISIX_TIME_INFINITE);
			}
		},
		[]() {
			if (!isix_irq_in_isr()) {
				lock.signal();
			}
		},
		periph::drivers::uart_early::open, "serial0", 115200);
}

}

//! Application crash called from the hard fault vector
void application_crash(crash_mode type, unsigned long* sp)
{
#ifdef PDEBUG
	cortex_cm3_print_core_regs(type, sp);
#else
	static_cast<void>(type);
	static_cast<void>(sp);
#endif
	for (;;) {
		asm volatile("wfi\n");
	}
}

ISIX_ISR_NACKED_VECTOR(hard_fault_exception_vector)
{
	_cm3_hard_hault_entry_fn(application_crash);
}

extern "C" void isix_kernel_panic_callback(const char* file, int line, const char* msg)
{
	tiny_printf("ISIX_PANIC %s:%i %s\r\n", file, line, msg);
}
