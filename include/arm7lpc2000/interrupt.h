#ifndef __ASM_INTERRUPT_H
#define __ASM_INTERRUPT_H

#include <isix/types.h>

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

#endif

