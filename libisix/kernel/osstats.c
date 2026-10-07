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
	ostick_t state_t;		// Timestamp of the last update
	ostick_t win_t;			// Timestamp of the last published window
	ostick_t busy_sum;		// Ticks spent outside of idle in the window
	ostick_t idle_sum;		// Ticks spent in idle in the window
	atomic_int rload;		// Final cpuload
	bool old_idle;			// Idle was scheduled at the last update
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
	// Window is closed on the first switch after the boundary, tickless has no switch every tick
	const ostick_t delta = t - cstats.state_t;
	if( cstats.old_idle ) {
		cstats.idle_sum += delta;
	} else {
		cstats.busy_sum += delta;
	}
	cstats.state_t = t;
	if( (ostick_t)(t - cstats.win_t) >= CYCLES_RST_COUNT )
	{
		const ostick_t den = cstats.busy_sum + cstats.idle_sum;
		if( den ) {
			atomic_store( &cstats.rload,
				(int)(((uint64_t)CPULOAD_MAX * cstats.busy_sum) / den) );
		}
		cstats.busy_sum = cstats.idle_sum = 0;
		cstats.win_t = t;
	}
	cstats.old_idle = idle_scheduled;
}

#endif /* CONFIG_ISIX_CPU_USAGE_API */

