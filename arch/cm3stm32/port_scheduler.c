#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <asm/context.h>
#include <asm/interrupt.h>
#include <prv/scheduler.h>
#include <isix/time.h>


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

    schedule();
    
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
      "0: .word current_task\t\n"
      );
}

/*-----------------------------------------------------------------------*/
//Create of stack context 
reg_t* task_init_stack(reg_t *sp,task_func_ptr_t pfun,void *param)
{
    *sp-- = INITIAL_XPSR;
    *sp-- = (reg_t)pfun;    //PC
    *sp-- = 0x14;           //LR
    *sp-- = 0x12;           //R12
    *sp-- = 0x3;            //R3
    *sp-- = 0x2;            //R2
    *sp-- = 0x1;            //R1
    *sp-- = (reg_t)param;   //R0
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
    schedule_time();
#ifdef CONFIG_USE_PREEMPTION
    /* Set a PendSV to request a context switch. */
    *(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
#endif
}

/*-----------------------------------------------------------------------*/
void port_yield(void )
{
  /* Set a PendSV to request a context switch. */
  *(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
}
/*-----------------------------------------------------------------------*/
void start_first_task(void) __attribute__((naked));
void start_first_task( void )
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

