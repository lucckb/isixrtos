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
void port_set_interrupt_mask(void);

void port_clear_interrupt_mask(void);
/*-----------------------------------------------------------------------*/

#endif

