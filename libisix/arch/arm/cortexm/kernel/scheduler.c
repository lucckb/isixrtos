/*
 * =====================================================================================
 *
 *       Filename:  scheduler.c
 *
 *    Description:  Global scheduler porting layer
 *
 *        Version:  1.0
 *        Created:  18.04.2017 16:47:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <isix/config.h>
#include <isix/arch/cpu.h>
#include <isix/cortexm/scb_regs.h>
#include <isix/prv/scheduler.h>


#define CPUID_CORTEX_M7_r0p1  0x410FC271UL
#define CPUID_CORTEX_M7_r0p0  0x410FC270UL

//Cyclic schedule time interrupt
void __attribute__((__interrupt__)) systick_isr_vector(void)
{
	_isixp_schedule_time();

    /* Set a PendSV to request a context switch. */
    if(schrun) {
		SCB_ICSR = SCB_ICSR_PENDSVSET;
	}

}


//Set interrupt mask
void _isix_port_set_interrupt_mask(void)
{
	asm volatile(  "msr BASEPRI,%0\t\n"
		::"r"(ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY)
	);
}

//Clear interrupt mask
void _isix_port_clear_interrupt_mask(void)
{
    asm volatile("msr BASEPRI,%0\t\n"::"r"(0));
}


//Yield to another task
void _isix_port_yield(void)
{
	/* Set a PendSV to request a context switch. */
	SCB_ICSR = SCB_ICSR_PENDSVSET;
	_isix_port_flush_memory();
}


/* Start first task by svc call
 * If the scheduler is able to stop
 * (when CONFIG_ISIX_SHUTDOWN_API is defined)
 *  the return main context is preserved
 *  if not the MSP pointer is setup to statup value
 */

void  __attribute__((naked)) _isix_port_start_first_task( void )
{
	if( SCB_CPUID == CPUID_CORTEX_M7_r0p1 ) {
		isix_bug( "Buggy CPU core M7 rev. r0p1 unsupported." );
	}
	if( SCB_CPUID == CPUID_CORTEX_M7_r0p0 ) {
		isix_bug( "Buggy CPU core M7 rev. r0p0 unsupported." );
	}
#if CONFIG_ISIX_SHUTDOWN_API
	__asm volatile(
		"push {r4-r11}\t\n"
		"svc 0\t\n"
		"pop {r4-r11}\t\n"
		"bx lr\t\n"
	 );
#else
  __asm volatile(
      "ldr r0, =0xE000ED08 \t\n" /* Use the NVIC offset register to locate the stack. */
      "ldr r0, [r0]\t\n"
      "ldr r0, [r0]\t\n"
      "msr msp, r0\t\n"		/* Set the msp back to the start of the stack. */
      "svc 0\t\n"
	  "nop\r\n"
      );
#endif
}

