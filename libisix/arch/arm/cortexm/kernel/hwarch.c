/*
 * =====================================================================================
 *
 *       Filename:  ostimer.c
 *
 *    Description:  ISIX OS timer functions
 *
 *        Version:  1.0
 *        Created:  18.04.2017 13:23:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <isix/cortexm/systick_regs.h>
#include <isix/cortexm/systick.h>
#include <isix/arch/irq.h>
#include <isix/assert.h>
#include <isix/arch/cache.h>
#include <isix/arch/ostimer.h>
#include <stdint.h>

static unsigned long g_isix_core_freq;

unsigned long _isix_port_get_core_freq(void)
{
	return g_isix_core_freq;
}



/** Configure OS system timer according to the core_frequency
 * @param[in] Input core freqncy
 */
void _isix_port_conf_hardware( unsigned long core_freq )
{
	g_isix_core_freq = core_freq;
	//Enable dcache and icache
	isix_icache_enable( true );
	isix_dcache_enable( true );
	if( core_freq < 1000000UL ) {
		isix_bug(" Invalid core frequency. Should be  > 1M" );
	}
	if( !systick_set_frequency(CONFIG_ISIX_HZ, core_freq) ) {
		isix_bug( "Unable to configure frequency for systick");
	}
	isix_set_raw_irq_priority( isix_cortexm_irq_systick, 0xff );
	isix_set_raw_irq_priority( isix_cortexm_irq_svc_call, 0xff );
	isix_set_raw_irq_priority( isix_cortexm_irq_pend_svc, 0xff );
	systick_interrupt_enable();
	systick_counter_enable();
}


//Get HI resolution timer
unsigned long _isix_port_get_hres_jiffies_timer_value(void)
{
	return STK_RVR - STK_CVR;
}

//Get hres timer max value
unsigned long _isix_port_get_hres_jiffies_timer_max_value(void)
{
	return STK_RVR;
}

#ifdef CONFIG_ISIX_TICKLESS
void _isix_port_configure_tickless_oneshot_timer(ostick_t ticks_until_timeout, unsigned long ahb_freq)
{
	if( ticks_until_timeout == 0U ) {
		ticks_until_timeout = 1U;
	}
	systick_interrupt_disable();
	systick_counter_disable();
	uint64_t ticks_per_jiffy = (uint64_t)ahb_freq / (uint64_t)CONFIG_ISIX_HZ;
	if( ticks_per_jiffy == 0ULL ) {
		ticks_per_jiffy = 1ULL;
	}
	uint64_t cycles = (uint64_t)ticks_until_timeout * ticks_per_jiffy;
	uint64_t comp = (uint64_t)CONFIG_ISIX_TICKLESS_TIMER_COMPENSATION_TICKS * ticks_per_jiffy;
	if( cycles > comp ) {
		cycles -= comp;
	}
	if( cycles > 0xFFFFFFULL ) {
		cycles = 0xFFFFFFULL;
	}
	uint32_t reload = (uint32_t)cycles;
	if( reload > 0U ) {
		reload -= 1U;
	}
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_set_reload(reload);
	systick_clear();
	systick_interrupt_enable();
	systick_counter_enable();
}

void _isix_port_restore_periodic_tick_timer(unsigned long ahb_freq)
{
	systick_interrupt_disable();
	systick_counter_disable();
	if( !systick_set_frequency(CONFIG_ISIX_HZ, ahb_freq) ) {
		isix_bug("Unable to restore periodic SysTick");
	}
	systick_clear();
	systick_interrupt_enable();
	systick_counter_enable();
}

uint32_t _isix_port_tickless_get_oneshot_timer_current_value(void)
{
	return systick_get_value();
}

uint32_t _isix_port_tickless_get_oneshot_timer_reload_value(void)
{
	return systick_get_reload();
}
#endif /* CONFIG_ISIX_TICKLESS */



