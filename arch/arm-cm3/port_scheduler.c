#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <prv/scheduler.h>


/*-----------------------------------------------------------------------*/

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
    ::"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY)                 \
	)
/*-----------------------------------------------------------------------*/

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
    "0: .word isix_current_task\t\n"									\
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
void irq_handler_pend_sv(void) __attribute__((__interrupt__,naked));

void irq_handler_pend_sv(void)
{
    cpu_save_context();

    isixp_schedule();

    cpu_restore_context();
}

/*-----------------------------------------------------------------------*/
void irq_handler_svc(void) __attribute__((__interrupt__,naked));
void irq_handler_svc(void)
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
      "0: .word isix_current_task\t\n"
      );
}

/*-----------------------------------------------------------------------*/
//Create of stack context 
unsigned long* isixp_task_init_stack(unsigned long *sp, task_func_ptr_t pfun, void *param)
{
    *sp-- = INITIAL_XPSR;
    *sp-- = (unsigned long)pfun;    //PC
    *sp-- = 0x14;           //LR
    *sp-- = 0x12;           //R12
    *sp-- = 0x3;            //R3
    *sp-- = 0x2;            //R2
    *sp-- = 0x1;            //R1
    *sp-- = (unsigned long)param;   //R0
    *sp-- = 0x11;	//R11
    *sp-- = 0x10;	//R10
    *sp-- = 0x9;	//R9
    *sp-- = 0x8;	//R8
    *sp-- = 0x7;	//R7
    *sp-- = 0x6;	//R6
    *sp-- = 0x5;	//R5
    *sp = 0x4;	//R4
    return sp;
}

/*-----------------------------------------------------------------------*/

void irq_handler_timer_isr(void) __attribute__((__interrupt__));

void irq_handler_timer_isr(void)
{
    //Increment system ticks
    port_set_interrupt_mask();
	isixp_schedule_time();
	port_clear_interrupt_mask();

#ifdef CONFIG_USE_PREEMPTION
    /* Set a PendSV to request a context switch. */
    *(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
#endif
}

/*-----------------------------------------------------------------------*/
void port_set_interrupt_mask(void)
{
 asm volatile(  "msr BASEPRI,%0\t\n"
                ::"r"(configMAX_SYSCALL_INTERRUPT_PRIORITY)
             );
}

/*-----------------------------------------------------------------------*/
void port_clear_interrupt_mask(void)
{
    asm volatile("msr BASEPRI,%0\t\n"::"r"(0));
}

/*-----------------------------------------------------------------------*/
void port_yield(void )
{
  /* Set a PendSV to request a context switch. */
  *(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
}

/*-----------------------------------------------------------------------*/
void port_start_first_task(void) __attribute__((naked));
void port_start_first_task( void )
{
  __asm volatile(
      " ldr r0, =0xE000ED08 \t\n" /* Use the NVIC offset register to locate the stack. */	
      "ldr r0, [r0]\t\n"
      "ldr r0, [r0]\t\n"
      "msr msp, r0\r\n" 	/* Set the msp back to the start of the stack. */
      "svc 0\r\n"
      );
}
/*-----------------------------------------------------------------------*/

