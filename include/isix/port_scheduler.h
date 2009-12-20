#ifndef __PORT_SCHEDULER_H
#define __PORT_SCHEDULER_H
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
static const unsigned ISIX_PORT_SCHED_MIN_STACK_DEPTH = 96;
#else
//Minimum stack depth
#define ISIX_PORT_SCHED_MIN_STACK_DEPTH 96
#endif /*__cplusplus*/
/*-----------------------------------------------------------------------*/
void port_yield( void );

/*-----------------------------------------------------------------------*/

void port_start_first_task( void );

/*-----------------------------------------------------------------------*/
//TODO: IRQ lock and IRQ unlock
void port_set_interrupt_mask( void );

void port_clear_interrupt_mask( void );

/*-----------------------------------------------------------------------*/
//Idle task additional
void port_idle_task( void );

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */

/*-----------------------------------------------------------------------*/
#endif

