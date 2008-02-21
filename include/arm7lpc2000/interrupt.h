#ifndef __ASM_INTERRUPT_H
#define __ASM_INTERRUPT_H

#include <isix/types.h>
#include <asm/lpc214x_vic.h>
#include <asm/lpc214x.h>

/*-----------------------------------------------------------------------*/
//Pointer to task function
typedef void (*interrupt_proc_ptr_t)(void);

/*-----------------------------------------------------------------------*/

//Disable interrupt return IRQ state
reg_t irq_disable(void);

/*-----------------------------------------------------------------------*/

//Enable IRQ return previous IRQ state
reg_t irq_enable(void);

/*-----------------------------------------------------------------------*/

//Restore previous IRQ state
reg_t irq_restore(reg_t old_cpsr);

/*-----------------------------------------------------------------------*/

//Disable FIQ interrupt
reg_t fiq_disable(void);

/*-----------------------------------------------------------------------*/

//Enable FIQ interrupt
reg_t fiq_enable(void);

/*-----------------------------------------------------------------------*/
//Restore previous FIQ status
reg_t fiq_restore(reg_t old_cpsr);

/*-----------------------------------------------------------------------*/
/* Register irq interrupt
 * int_num - Interrupt number
 * prio - Interrupt priority 
 * irq_proc - Interrupt priority proc
 * return 0 if success otherwise error code */
int interrupt_register(u8 int_num,s16 prio,interrupt_proc_ptr_t interrupt_proc );

/*-----------------------------------------------------------------------*/
//Register fiq interrupt (arm specific issue)
int interrupt_register_fiq(u8 int_num);


/*-----------------------------------------------------------------------*/
//Mask selected interrupt
static inline void interrupt_mask(unsigned long mask)
{
    VICIntEnable = mask;
}

/*-----------------------------------------------------------------------*/
// Unmask selected interrupt
static inline void interrupt_umask(unsigned long mask)
{
   VICIntEnClr = mask;
}

/*-----------------------------------------------------------------------*/

/* Unregister specified interrupt */
int interrupt_unregister(u8 int_num);

/*-----------------------------------------------------------------------*/
//Definition of task function IRQ
#define INTERRUPT_PROC(FUNC)						\
	void FUNC(void) __attribute__ ((interrupt("IRQ")));	\
	void FUNC(void)

/*-----------------------------------------------------------------------*/
//Definition of task function FIQ
#define INTERRUPT_FIQ_PROC(FUNC)						\
	void FUNC(void) __attribute__ ((interrupt("FIQ")));	\
	void FUNC(void)

/*-----------------------------------------------------------------------*/
//Interrupt controler info end of isr
static inline void interrupt_isr_exit(void) { VICVectAddr = 0; }

/*-----------------------------------------------------------------------*/
#endif


