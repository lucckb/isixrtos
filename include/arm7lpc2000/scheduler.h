#ifndef __ASM_SCHEDULER_H
#define __ASM__SCHEDULER_H


/*-----------------------------------------------------------------------*/
//Minimum stack depth
#define SCHED_MIN_STACK_DEPTH 192

/*-----------------------------------------------------------------------*/
//Yield the cpu
#define cpu_yield() asm volatile("swi #1\t\n")

/*-----------------------------------------------------------------------*/

#endif
