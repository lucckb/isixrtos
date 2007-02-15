#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <asm/context.h>
#include <asm/interrupt.h>
#include <isix/scheduler.h>
#include <asm/lpc214x.h>

//System Mode enable IRQ and FIQ
#define INITIAL_SPSR 0x0000001f

//Timer 0 issue
#define T0MCR_INTERRUPT_ON_MR0 0x1U
#define T0MCR_RESET_ON_MR0 0x2U
#define T0TCR_COUNTER_RESET 0x2U
#define T0TCR_COUNTER_ENABLE 0x1U
#define T0IR_MR0 0x1U
//Timer ticks rate
#define T0_TICKS 1000000
//Reload value
#define MR0_ADDVAL (T0_TICKS/HZ)

/*-----------------------------------------------------------------------*/
//Yield processor
void cpu_swi_yield(void) __attribute__((interrupt("SWI"),naked));

void cpu_swi_yield(void)
{
    //Add offset to LR according to SWI
    asm volatile("ADD LR,LR,#4 \t\n");
    cpu_save_context();
    scheduler();
    cpu_restore_context();
}

/*-----------------------------------------------------------------------*/
//Create of stack context 
reg_t* task_init_stack(reg_t *sp,task_func_ptr_t pfun,void *param)
{
     reg_t *orig_sp = sp;
    *sp-- = (reg_t)pfun + 4;
    *sp-- = 14; //R14
    *sp-- = (reg_t)orig_sp; //R13
    *sp-- = 12;  //R12
    *sp-- = 11; //R11
    *sp-- = 10; //R10
    *sp-- = 9; //R9
    *sp-- = 8; //R8
    *sp-- = 7 ; //R7
    *sp-- = 6;   //R6
    *sp-- = 5;  //R5
    *sp-- = 4; //R4
    *sp-- = 3;  //R3
    *sp-- = 2;  //R2
    *sp-- = 1;  //R1
    *sp-- = (reg_t)param;  //R0
    *sp = INITIAL_SPSR;   //SPSR
    return sp;
}

/*-----------------------------------------------------------------------*/
//Sys timer interrupt using timer 0 to provide timer tick
#ifndef CONFIG_USE_PREEMPTION

INTERRUPT_PROC(sys_timer_isr)

#else

void sys_timer_isr(void) __attribute__((interrupt("IRQ"),naked));
void sys_timer_isr(void)

#endif
{
#ifdef  CONFIG_USE_PREEMPTION
     cpu_save_context();
#endif
    //Add const var to match register
    T0MR0 = T0TC + MR0_ADDVAL;
    //Increment system ticks
    sched_time++;
    //End of interrupt
#ifdef  CONFIG_USE_PREEMPTION
    scheduler();
    T0IR = T0IR_MR0;
    interrupt_isr_exit();
    cpu_restore_context();
#else
    //Controller end of intterrupt
    T0IR = T0IR_MR0;
    interrupt_isr_exit();
#endif
}

/*-----------------------------------------------------------------------*/
/* Initialize system timer */
void sys_time_init(void)
{
    //Timer increment ticks after 1us
    T0PR = CONFIG_PCLK/T0_TICKS -1;
    //If compare match then interrupt
    T0MCR |= T0MCR_INTERRUPT_ON_MR0;// |T0MCR_RESET_ON_MR0;
    //Set MR to req ticks
    T0MR0 = MR0_ADDVAL;
    //Reset Counter and prescaler
    T0TCR = T0TCR_COUNTER_RESET;
    //Kasuj flage przerwania
    T0IR = T0IR_MR0;
    //Register interrupt with lower priority
    interrupt_register(INTERRUPT_NUM_TIMER0,INTERRUPT_PRIO(15),sys_timer_isr);
    //Enable timer
    T0TCR = T0TCR_COUNTER_ENABLE;
}

