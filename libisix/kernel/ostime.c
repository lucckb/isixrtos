/*
 * =====================================================================================
 *
 *       Filename:  ostime.c
 *
 *    Description:  OS time support
 *
 *        Version:  1.0
 *        Created:  04.04.2015 23:45:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <isix/ostime.h>
#include <isix/scheduler.h>
#include <isix/prv/scheduler.h>
#include <isix/arch/cpu.h>
#include <isix/assert.h>

//Get the system time in microseconds
osutick_t isix_get_ujiffies(void)
{
	uint32_t cycles_per_tick;
	isix_enter_critical();
	const ostick_t jiffies = isix_get_jiffies();
	const uint32_t cycles = _isix_port_systimer_subtick( &cycles_per_tick );
	isix_exit_critical();
	const osutick_t us_per_tick = (osutick_t)1000000 / (osutick_t)ISIX_HZ;
	return (osutick_t)jiffies * us_per_tick + (osutick_t)cycles * us_per_tick / cycles_per_tick;
}

/** Busy waiting for selecred amount of time
 * @param[in] timeout Number of microseconds for busy wait
 * @return None
 */
void isix_wait_us( unsigned timeout )
{
    isix_assert_isr();
	osutick_t t1 = isix_get_ujiffies();
	for(;;) 
	{
		osutick_t t2 =  isix_get_ujiffies();
		if( t2>=t1 ) { if( t2-t1>timeout ) break; }
		else { if( t1-t2>timeout) break; }
	}
}

//! Convert ms to ticks
ostick_t isix_ms2tick(unsigned long ms)
{
	ostick_t ticks = (CONFIG_ISIX_HZ * ms)/1000UL;
	if(ticks==0) ticks++;
	return ticks;
}

//! Isix wait selected amount of time
int isix_wait(ostick_t timeout)
{
    isix_assert_isr();
	if(schrun)
	{
		//If scheduler is running delay on semaphore
		isix_enter_critical();
		_isixp_set_sleep_timeout( OSTHR_STATE_SLEEPING, timeout );
		isix_exit_critical();
		_isix_port_yield();
		return ISIX_EOK;
	}
	else
	{
		//If scheduler is not running delay on busy wait
		ostick_t t1 = isix_get_jiffies();
		if(t1+timeout>t1)
		{
			t1+= timeout;
			while(t1>isix_get_jiffies()) _isix_port_idle_cpu();
		}
		else
		{
			t1+= timeout;
			while(t1<isix_get_jiffies()) _isix_port_idle_cpu();
		}
		return ISIX_EOK;
	}
}


