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

#include <isix/arch/irq.h>
#include <isix/arch/scheduler.h>
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
	isix_set_raw_irq_priority( isix_cortexm_irq_systick, 0xff );
	isix_set_raw_irq_priority( isix_cortexm_irq_svc_call, 0xff );
	isix_set_raw_irq_priority( isix_cortexm_irq_pend_svc, 0xff );
	_isix_port_systimer_init( core_freq );
}
