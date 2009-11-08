#ifndef __PORT_SCHEDULER_H
#define __PORT_SCHEDULER_H


/*-----------------------------------------------------------------------*/
//Minimum stack depth
#define PORT_SCHED_MIN_STACK_DEPTH 192

/*-----------------------------------------------------------------------*/
void port_yield(void );

/*-----------------------------------------------------------------------*/

void port_start_first_task(void);

/*-----------------------------------------------------------------------*/
//TODO: IRQ lock and IRQ unlock
void port_isr_lock(void);

void port_isr_unlock(void);
/*-----------------------------------------------------------------------*/

#endif

