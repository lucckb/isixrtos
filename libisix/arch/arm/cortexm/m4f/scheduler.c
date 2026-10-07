#include <isix/config.h>
#include <isix/types.h>
#include <isix/task.h>
#include <isix/prv/scheduler.h>
#include <isix/arch/isr_vectors.h>


//System Mode enable IRQ and FIQ
#define INITIAL_XPSR 0x01000000
#define INITIAL_EXEC_RETURN    0xfffffffd


/* PendSV context switch. The naked handler is a single asm statement:
 * compiler generated code could clobber r4-r11 before they are saved. */
ISIX_ISR_NACKED_VECTOR(pend_svc_isr_vector)
{
	asm volatile(
#if CONFIG_ISIX_SHUTDOWN_API
	"ldr r3, 2f\n"
	"ldrb r3, [r3]\n"
	"cmp r3, #0\n"
	"beq 1f\n"
#endif
	"clrex\n"
	"mrs r0, psp\n"
	"tst lr, #0x10\n"
	"it eq\n"
	"vstmdbeq r0!, {s16-s31}\n"
	"stmdb r0!, {r4-r11, lr}\n"
	"ldr r3, 0f\n"
	"ldr r2, [r3]\n"
	"str r0, [r2]\n"
	"push {r3, lr}\n"
	"mov r0, %[basepri]\n"
	"msr basepri, r0\n"
	"bl %c[sched]\n"
	"mov r0, #0\n"
	"msr basepri, r0\n"
	"pop {r3, lr}\n"
	"ldr r1, [r3]\n"
	"ldr r0, [r1]\n"
	"ldmia r0!, {r4-r11, lr}\n"
	"tst lr, #0x10\n"
	"it eq\n"
	"vldmiaeq r0!, {s16-s31}\n"
	"msr psp, r0\n"
	"bx lr\n"
#if CONFIG_ISIX_SHUTDOWN_API
	/* Return to the main context saved by the scheduler start */
	"1:\n"
	"bic lr, lr, #0x04\n"
	"orr lr, lr, #0x10\n"
	"bx lr\n"
#endif
	".align 2\n"
	"0: .word %c[curr]\n"
#if CONFIG_ISIX_SHUTDOWN_API
	"2: .word %c[run]\n"
#endif
	:: [basepri] "i"(ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY),
	   [sched] "i"(_isixp_schedule),
	   [curr] "i"(&_isix_current_task),
	   [run] "i"(&_isix_scheduler_running)
	);
}


//SVC handler call for start the first task
ISIX_ISR_NACKED_VECTOR(svc_isr_vector)
{
     asm volatile(
     "ldr r3, 2f\t\n"				/* Mark the scheduler as running. */
     "movs r2, #1\t\n"
     "strb r2, [r3]\t\n"
     "ldr r3, 0f\t\n"				/* Restore the context. */
     "ldr r1, [r3]\t\n"				/* Use _isix_current_task */
     "ldr r0, [r1]\t\n"			    /* The first item in the _isix_current_task
									   is the task top of stack. */
     "ldmia r0!, {r4-r11, r14}\t\n"	 /* Pop the registers that are not automatically
										saved on exception entry and the critical
										nesting count. */
     "msr psp, r0\t\n"				 /* Restore the task stack pointer. */
     "mov r0, #0\t\n"
     "msr basepri, r0\t\n"
     "bx r14\t\n"
     ".align 2 \t\n"
     "0: .word _isix_current_task\t\n"
     "2: .word _isix_scheduler_running\t\n"
      );
}

//Create of stack context
unsigned long* _isixp_task_init_stack(unsigned long *sp, task_func_ptr_t pfun, void *param)
{
	/* Simulate the stack frame as it would be created by a context switch
	interrupt. */

	/* Offset added to account for the way the MCU uses the stack on entry/exit
	of interrupts, and to ensure alignment. */
	sp--;

	*sp = INITIAL_XPSR;	/* xPSR */
	sp--;
	*sp = ( unsigned long ) pfun;	/* PC */
	sp--;
	*sp = ( unsigned long ) _isixp_task_terminator;	/* LR */

	/* Save code space by skipping register initialisation. */
	sp -= 5;	/* R12, R3, R2 and R1. */
	*sp = ( unsigned long ) param;	/* R0 */

	/* A save method is being used that requires each task to maintain its
	own exec return value. */
	sp--;
	*sp = INITIAL_EXEC_RETURN;

	sp -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */

	return sp;

}

