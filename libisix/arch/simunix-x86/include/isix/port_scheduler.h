#ifndef __PORT_SCHEDULER_H
#define __PORT_SCHEDULER_H
#include <unistd.h>
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
static const unsigned ISIX_PORT_SCHED_MIN_STACK_DEPTH = 112;
#else
//Minimum stack depth
#define ISIX_PORT_SCHED_MIN_STACK_DEPTH 112
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
//Cleanup task for example for free additional memory
void port_cleanup_task(void *sp);

/*-----------------------------------------------------------------------*/
//Idle task additional
static inline void port_idle_cpu( void )
{
    usleep(50);
}

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */

/*-----------------------------------------------------------------------*/
#endif

