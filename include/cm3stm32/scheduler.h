#ifndef __ASM_SCHEDULER_H
#define __ASM__SCHEDULER_H


/*-----------------------------------------------------------------------*/
//Minimum stack depth
#define SCHED_MIN_STACK_DEPTH 192

/*-----------------------------------------------------------------------*/
void port_yield(void );

//Yield the cpu
#define cpu_yield() yield();

/*-----------------------------------------------------------------------*/
void start_first_task(void);

#endif
