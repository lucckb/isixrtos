/*
 * =====================================================================================
 *
 *       Filename:  osstats.c
 *
 *    Description:  OS statistic API
 *
 *        Version:  1.0
 *        Created:  12.05.2015 19:29:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#include <isix/osstats.h>
#include <isix/prv/osstats.h>
#include <stdatomic.h>

#ifdef CONFIG_ISIX_LOGLEVEL_OSSTATS
#undef CONFIG_ISIX_OSSTATS 
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_OSSTATS
#endif
#include <isix/prv/printk.h>

#if CONFIG_ISIX_CPU_USAGE_API


//! Calculate nearst power of two 
#define _LOG2A(s) (((s) &0xffffffff00000000) ? (32 +_LOG2B((s) >>32)): (_LOG2B(s)))
#define _LOG2B(s) (((s) &0xffff0000)         ? (16 +_LOG2C((s) >>16)): (_LOG2C(s)))
#define _LOG2C(s) (((s) &0xff00)             ? (8  +_LOG2D((s) >>8)) : (_LOG2D(s)))
#define _LOG2D(s) (((s) &0xf0)               ? (4  +_LOG2E((s) >>4)) : (_LOG2E(s)))
#define _LOG2E(s) (((s) &0xc)                ? (2  +_LOG2F((s) >>2)) : (_LOG2F(s)))
#define _LOG2F(s) (((s) &0x2)                ? (1)                  : (0))

#define LOG2_UINT64 _LOG2A
#define LOG2_UINT32 _LOG2B
#define LOG2_UINT16 _LOG2C
#define LOG2_UINT8  _LOG2D
#define CYCLES_RST_COUNT  (1UL<<((LOG2_UINT64(CONFIG_ISIX_HZ-1)+1)))
#define CPULOAD_MAX 1000LU


struct cpu_stats {
	ostick_t idle_t;
	ostick_t idle_sum;
	ostick_t norm_t;
	ostick_t norm_sum;
	atomic_int rload;		// Final cpuload
	bool old_state;
};

//! Cpu global statistics data
struct cpu_stats cstats;


/** Return the current CPU load of the system
 * @return CPUload in profiles for ex 1000
 */
int isix_cpuload( void )
{
	return atomic_load( &cstats.rload );
}


/** Reschedule API information for task 
 * @param[in] t timestamp of event
 * @param[in] idle_scheduled idle task is scheduled
 */
void _isixp_schedule_update_statistics( ostick_t t, bool idle_scheduled )
{
	if( cstats.old_state != idle_scheduled )
	{
		if( idle_scheduled )
		{
			cstats.norm_t = t;
			cstats.idle_sum += t>cstats.idle_t?t-cstats.idle_t:cstats.idle_t-t;
		}
		else
		{
			cstats.idle_t = t;
			cstats.norm_sum += t>cstats.norm_t?t-cstats.norm_t:cstats.norm_t-t;
		}
	}
	if( (t%CYCLES_RST_COUNT) == 0 )
	{
		ostick_t den = cstats.idle_sum + cstats.norm_sum;
		if( den ) {
			atomic_store( &cstats.rload, (CPULOAD_MAX*cstats.idle_sum)/den );
		}
		cstats.idle_sum = cstats.norm_sum = 0;
	}
	cstats.old_state = idle_scheduled;
}

#endif /* CONFIG_ISIX_CPU_USAGE_API */

