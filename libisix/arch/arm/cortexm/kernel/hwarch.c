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
#include <isix/scheduler.h>
#include <isix/arch/scheduler.h>
#include <isix/arch/cache.h>



/** Configure OS system timer according to the core_frequency
 * @param[in] Input core freqncy
 */
void _isix_port_conf_hardware( unsigned long core_freq )
{
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



