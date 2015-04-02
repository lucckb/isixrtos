#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <isix/task.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
/*-----------------------------------------------------------------------*/
//Save context
#define cpu_save_context()										\
    asm volatile (												\
    "mrs r0, psp\t\n"					        				\
    "stmdb r0!, {r4-r11}\t\n"                                   \
    "ldr r3,0f\t\n"                                   			\
    "ldr r2,[r3]\t\n"                                           \
    "str r0, [r2]\t\n"                                          \
    "stmdb sp!, {r3,r14}\t\n"                                   \
    "mov r0,%0\t\n"                                             \
    "msr basepri,r0\t\n"                                        \
    ::"i"(ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY)                 \
	)
/*-----------------------------------------------------------------------*/
//Restore context
#define cpu_restore_context()                                   \
    asm volatile  (                                             \
    "mov r0,#0\t\n"                                             \
    "msr basepri,r0\t\n"                                        \
	"ldmia sp!, {r3,r14}\t\n"                                   \
    "ldr r1,[r3]\t\n"                                           \
    "ldr r0, [r1]\t\n"                                          \
    "ldmia r0!, {r4-r11}\t\n"                                   \
    "msr psp, r0\t\n"                                           \
    "bx r14\r\n"                                                \
    ".align 2 \t\n"												\
    "0: .word _isix_current_task\t\n"							\
   )
/*-----------------------------------------------------------------------*/
/** Restore the context to the place when scheduler was started to run
 *  Use Main stack pointer negate bit 2 of LR
 */
#define cpu_restore_main_context()								\
	asm volatile (												\
	"bic lr,lr,#0x04\t\n"											\
	"bx lr\t\n"													\
	)

/*-----------------------------------------------------------------------*/
#define portNVIC_INT_CTRL           ( ( volatile unsigned long *) 0xe000ed04 )
#define portNVIC_PENDSVSET          0x10000000
/*-----------------------------------------------------------------------*/

//System Mode enable IRQ and FIQ
#define INITIAL_XPSR 0x01000000

#ifndef DEBUG_SCHEDULER
#define DEBUG_SCHEDULER DBG_OFF
#endif

/*-----------------------------------------------------------------------*/
//Pend SV interrupt (context switch)
void __attribute__((__interrupt__,naked)) pend_svc_isr_vector(void)
{
#ifdef ISIX_CONFIG_SHUTDOWN_API
	if( _isix_scheduler_running ) {
		  cpu_save_context();
		  _isixp_schedule();
		  cpu_restore_context();
	} else {
		cpu_restore_main_context();
	}
#else
    cpu_save_context();

    _isixp_schedule();

    cpu_restore_context();
#endif
}

/*-----------------------------------------------------------------------*/
//SVC handler call for start the first task
void __attribute__((__interrupt__,naked)) svc_isr_vector(void)
{
     asm volatile(
     "ldr r3, 0f\t\n" /* Restore the context. */
     "ldr r1, [r3]\t\n"			 /* Use pxCurrentTCBConst to get the pxCurrentTCB address. */
     "ldr r0, [r1]\t\n" 			/* The first item in pxCurrentTCB is the task top of stack. */
     "ldmia r0!, {r4-r11}\t\n"	 /* Pop the registers that are not automatically saved on exception entry and the critical nesting count. */
     "msr psp, r0\t\n" /* Restore the task stack pointer. */
     "mov r0, #0\t\n"
     "msr basepri, r0\t\n"
     "orr r14, #0xd\t\n"
     "bx r14\n"
     ".align 2 \t\n"
     "0: .word _isix_current_task\t\n"
      );
}
/*-----------------------------------------------------------------------*/
//! Terminate the process when task exits
static void __attribute__((noreturn)) _isixp_process_terminator(void)
{
	isix_task_kill(NULL);
	for(;;);
}
/*-----------------------------------------------------------------------*/
//Create of stack context 
unsigned long* _isixp_task_init_stack(unsigned long *sp, task_func_ptr_t pfun, void *param)
{
	/* Simulate the stack frame as it would be created by a context switch
	interrupt. */
	sp--; /* Offset added to account for the way the MCU uses the stack on entry/exit of interrupts. */
	*sp = INITIAL_XPSR;	/* xPSR */
	sp--;
	*sp = ( unsigned long ) pfun;	/* PC */
	sp--;
	*sp = ( unsigned long ) _isixp_process_terminator;	/* LR */
	sp -= 5;	/* R12, R3, R2 and R1. */
	*sp = ( unsigned long ) param;	/* R0 */
	sp -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */
	return sp;
}
/*-----------------------------------------------------------------------*/
//Cyclic schedule time interrupt
void  __attribute__((__interrupt__)) systick_isr_vector(void)
{
	_isixp_schedule_time();

#ifdef ISIX_CONFIG_USE_PREEMPTION
    /* Set a PendSV to request a context switch. */
    if(_isix_scheduler_running) {
    	*(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
    }
#endif
}

/*-----------------------------------------------------------------------*/
//Set interrupt mask
void port_set_interrupt_mask(void)
{
 asm volatile(  "msr BASEPRI,%0\t\n"
                ::"r"(ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY)
             );
}

/*-----------------------------------------------------------------------*/
//Clear interrupt mask
void port_clear_interrupt_mask(void)
{
    asm volatile("msr BASEPRI,%0\t\n"::"r"(0));
}

/*-----------------------------------------------------------------------*/
//Yield to another task
void port_yield(void )
{
	/* Set a PendSV to request a context switch. */
	*(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
	port_flush_memory();
}

/*-----------------------------------------------------------------------*/
/* Start first task by svc call
 * If the scheduler is able to stop
 * (when ISIX_CONFIG_SHUTDOWN_API is defined)
 *  the return main context is preserved
 *  if not the MSP pointer is setup to statup value
 */
void  __attribute__((naked)) port_start_first_task( void )
{
#ifdef ISIX_CONFIG_SHUTDOWN_API
	__asm volatile(
		"push {r4-r11}\t\n"
		"svc 0\t\n"
		"pop {r4-r11}\t\n"
		"bx lr\t\n"
	);
#else
	__asm volatile(
      " ldr r0, =0xE000ED08 \t\n" /* Use the NVIC offset register to locate the stack. */	
      "ldr r0, [r0]\t\n"
      "ldr r0, [r0]\t\n"
      "msr msp, r0\t\n" 	/* Set the msp back to the start of the stack. */
      "svc 0\t\n"
	  "nop\t\n"
      );
#endif
}
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
