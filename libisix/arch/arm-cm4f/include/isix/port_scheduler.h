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
#define SYST_CVR (*((volatile unsigned long*)0xE000E018))
#define SYST_RVR (*((volatile unsigned long*)0xE000E014))

//Get HI resolution timer (must be inline)
static inline unsigned long port_get_hres_jiffies_timer_value(void)
{
	return SYST_RVR - SYST_CVR;
}
/*-----------------------------------------------------------------------*/
//Get hres timer max value
static inline unsigned long port_get_hres_jiffies_timer_max_value(void)
{
	return SYST_RVR;
}

#undef SYST_CVR
#undef SYST_RVR
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

