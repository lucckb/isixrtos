#pragma once

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
static const unsigned ISIX_PORT_SCHED_MIN_STACK_DEPTH = 112;
#else
//Minimum stack depth
#define ISIX_PORT_SCHED_MIN_STACK_DEPTH 112
#endif /*__cplusplus*/
/*-----------------------------------------------------------------------*/
/** Port yield CPU */
void port_yield( void );

/*-----------------------------------------------------------------------*/
/** Start first task */
void port_start_first_task( void );

/*-----------------------------------------------------------------------*/
//TODO: IRQ lock and IRQ unlock
void port_set_interrupt_mask( void );

void port_clear_interrupt_mask( void );
/*-----------------------------------------------------------------------*/
//Cleanup task for example dealocate memory
#ifdef __cplusplus
static inline __attribute__((always_inline))
	void port_cleanup_task(void */*sp*/) {}
#else
#define port_cleanup_task(p) do {} while(0)
#endif
/*-----------------------------------------------------------------------*/
//Idle task additional
static inline  __attribute__((always_inline)) 
	void port_idle_cpu( void )
{
#ifndef ISIX_DEBUG
	asm volatile("wfi\t\n");
#endif
}
/*-----------------------------------------------------------------------*/
//!Flush the memory
static inline  __attribute__((always_inline)) 
	void port_flush_memory( void )
{
	asm volatile("dsb\t\n");
	asm volatile("isb\t\n");
}
/*-----------------------------------------------------------------------*/
#define SYST_CVR (*((volatile unsigned long*)0xE000E018))
#define SYST_RVR (*((volatile unsigned long*)0xE000E014))

//Get HI resolution timer (must be inline)
static inline  __attribute__((always_inline))
	unsigned long port_get_hres_jiffies_timer_value(void)
{
	return SYST_RVR - SYST_CVR;
}
/*-----------------------------------------------------------------------*/
//Get hres timer max value
static inline __attribute__((always_inline)) 
	unsigned long port_get_hres_jiffies_timer_max_value(void)
{
	return SYST_RVR;
}

#undef SYST_CVR
#undef SYST_RVR
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */

/*-----------------------------------------------------------------------*/


