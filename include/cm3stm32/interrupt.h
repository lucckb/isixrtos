#ifndef __ASM_INTERRUPT_H
#define __ASM_INTERRUPT_H

#include <isix/types.h>
#include <isix/config.h>


/*-----------------------------------------------------------------------*/
//TODO: IRQ lock and IRQ unlock
#define sched_isr_lock() asm volatile("msr BASEPRI,%0\t\n"::"r"(configMAX_SYSCALL_INTERRUPT_PRIORITY))
#define sched_isr_unlock() asm volatile("msr BASEPRI,%0\t\n"::"r"(0))

/*-----------------------------------------------------------------------*/
#endif


