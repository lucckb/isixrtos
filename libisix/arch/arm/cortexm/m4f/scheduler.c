#include <isix/config.h>
#include <isix/types.h>
#include <isix/task.h>
#include <isix/prv/scheduler.h>


/** Restore the context to the place when scheduler was started to run
 *  Use Main stack pointer clear bit 2 in the LR
 *  Drop flating point stack frame set bit 4 in the LR
 */
#define cpu_restore_main_context()								\
	asm volatile (												\
	"bic lr,lr,#0x04\t\n"										\
	"orr lr,lr,#0x10\t\n"										\
	"bx lr\t\n"													\
	)


//Save context
#define cpu_save_context()										\
    asm volatile (												\
	"clrex\t\n"													\
    "mrs r0, psp\t\n"											\
    "ldr r3,0f\t\n"												\
    "ldr r2,[r3]\t\n"                                           \
    "tst r14, #0x10\t\n"										\
    "it eq\t\n"													\
    "vstmdbeq r0!, {s16-s31}\t\n"								\
    "stmdb r0!, {r4-r11,r14}\t\n"                               \
    "str r0, [r2]\t\n"                                          \
    "stmdb sp!, {r3,r14}\t\n"                                   \
    "mov r0,%0\t\n"                                             \
    "msr basepri,r0\t\n"                                        \
    ::"i"(ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY)                  \
	)

//Restore context
#define cpu_restore_context()                                   \
    asm volatile  (                                             \
    "mov r0,#0\t\n"                                             \
    "msr basepri,r0\t\n"                                        \
	"ldmia sp!, {r3,r14}\t\n"                                   \
    "ldr r1,[r3]\t\n"                                           \
    "ldr r0, [r1]\t\n"                                          \
    "ldmia r0!, {r4-r11, r14}\t\n"                              \
    "tst r14, #0x10\r\n"										\
    "it eq\t\n"													\
    "vldmiaeq r0!, {s16-s31}\t\n"								\
	"msr psp, r0\t\n"                                           \
    "bx r14\r\n"                                                \
    ".align 2 \t\n"												\
    "0: .word _isix_current_task\t\n"							\
   )




//System Mode enable IRQ and FIQ
#define INITIAL_XPSR 0x01000000
#define INITIAL_EXEC_RETURN    0xfffffffd


//Pend SV interrupt (context switch)
void __attribute__((__interrupt__,naked)) pend_svc_isr_vector(void)
{
#if CONFIG_ISIX_SHUTDOWN_API
	if( schrun ) {
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


//SVC handler call for start the first task
void __attribute__((__interrupt__,naked)) svc_isr_vector(void)
{
     asm volatile(
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

