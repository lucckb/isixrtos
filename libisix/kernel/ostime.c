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
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
/*--------------------------------------------------------------*/
/** Busy waiting for selecred amount of time
 * @param[in] timeout Number of microseconds for busy wait
 * @return None
 */
void isix_wait_us( unsigned timeout )
{
	osutick_t t1 = isix_get_ujiffies();
	for(;;) 
	{
		osutick_t t2 =  isix_get_ujiffies();
		if( t2>=t1 ) { if( t2-t1>timeout ) break; }
		else { if( t1-t2>timeout) break; }
	}
}
/*--------------------------------------------------------------*/
//! Convert ms to ticks
ostick_t isix_ms2tick(unsigned long ms)
{
	ostick_t ticks = (ISIX_CONFIG_HZ * ms)/1000UL;
	if(ticks==0) ticks++;
	return ticks;
}
/*--------------------------------------------------------------*/
//! Isix wait selected amount of time
int isix_wait(ostick_t timeout)
{
	if(schrun)
	{
		//If scheduler is running delay on semaphore
		_isixp_enter_critical();
		_isixp_set_sleep_timeout( THR_STATE_SLEEPING, timeout );
		_isixp_exit_critical();
		isix_yield();
		return ISIX_EOK;
	}
	else
	{
		//If scheduler is not running delay on busy wait
		ostick_t t1 = isix_get_jiffies();
		if(t1+timeout>t1)
		{
			t1+= timeout;
			while(t1>isix_get_jiffies()) port_idle_cpu();
		}
		else
		{
			t1+= timeout;
			while(t1<isix_get_jiffies()) port_idle_cpu();
		}
		return ISIX_EOK;
	}
}

/*--------------------------------------------------------------*/
