#ifndef __PORT_SCHEDULER_H
#define __PORT_SCHEDULER_H
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
static const unsigned ISIX_PORT_SCHED_MIN_STACK_DEPTH = 112 + 16*sizeof(long);
#else
//Minimum stack depth
#define ISIX_PORT_SCHED_MIN_STACK_DEPTH (112+16*sizeof(long))
#endif /*__cplusplus*/
/*-----------------------------------------------------------------------*/
/* Port yield function for the RTOS */
void port_yield( void );

/*-----------------------------------------------------------------------*/
/* Port start first task */
void port_start_first_task( void );

/*-----------------------------------------------------------------------*/
/* Port set interrupt mask */
void port_set_interrupt_mask( void );

/*-----------------------------------------------------------------------*/
/* Port clear interrupt mask */
void port_clear_interrupt_mask( void );
/*-----------------------------------------------------------------------*/
//Cleanup task for example dealocate memory
#ifdef __cplusplus
static inline void port_cleanup_task(void */*sp*/) {}
#else
#define port_cleanup_task(p) do {} while(0)
#endif
/*-----------------------------------------------------------------------*/
//Idle task additional
static inline void port_idle_cpu( void )
{
#ifndef ISIX_DEBUG
	asm volatile("wfi\t\n");
#endif
}

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */

/*-----------------------------------------------------------------------*/
#endif

