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
/*
task_scheduler
{
    ...
if power up sequence true
{
clear bins
reset timer counter
}
    Determine if scheduled task is idle or non-idle
    Case task idle:
    {
        stop timer counter
        calculate delta from timer counter
        sum counter delta with contents of non-idle task bin
        store results in non-idle task bin
        reset timer counter
        start timer counter
    }
    Case task non-idle:
    {
        stop timer counter
        calculate delta from timer counter
        sum counter delta with contents of idle task bin
        store results in idle task bin
        reset timer counter
        start timer counter
    ...
}

 U(n) = S ai / S (ai + bi)

 where U(n) is the utilization for n-tasks, ai the deltas for the idle task, S
 ai the summation of the deltas for the idle task, bi the deltas for all the
 nonidle tasks, and S (ai + bi) the summation of all the deltas for both the
 nonidle and idle tasks. This ratio gives the aggregate processor utilization
 for a single task technique. 
*/

#include <isix/osstats.h>
#include <isix/ostime.h>
#include <isix/prv/osstats.h>

#ifdef ISIX_LOGLEVEL_OSSTATS
#undef ISIX_CONFIG_OSSTATS 
#define ISIX_CONFIG_LOGLEVEL ISIX_LOGLEVEL_OSSTATS
#endif
#include <isix/prv/printk.h>

#ifdef ISIX_CONFIG_CPU_USAGE_API
struct cpu_stats {
	ostick_t ilast;
	ostick_t isum;
	ostick_t nlast;
	ostick_t nsum;
};

//! Cpu global statistics data
struct cpu_stats cstats;

#define CPULOAD_MAX 1000UL

/** Return the current CPU load of the system 
 * @return CPUload in profiles for ex 1000
 */
int isix_cpuload( void )
{
	ostick_t norm_c = cstats.nsum;
	ostick_t idle_c = cstats.isum;
	if( cstats.nsum || cstats.isum ) {
		return (CPULOAD_MAX*idle_c)/(idle_c+norm_c);
	}
	else {
		return 0;
	}
}

//! Calculate nearst power of two 

#define __LOG2A(s) (((s) &0xffffffff00000000) ? (32 +_LOG2B((s) >>32)): (_LOG2B(s)))
#define _LOG2B(s) (((s) &0xffff0000)         ? (16 +_LOG2C((s) >>16)): (_LOG2C(s)))
#define _LOG2C(s) (((s) &0xff00)             ? (8  +_LOG2D((s) >>8)) : (_LOG2D(s)))
#define _LOG2D(s) (((s) &0xf0)               ? (4  +_LOG2E((s) >>4)) : (_LOG2E(s)))
#define _LOG2E(s) (((s) &0xc)                ? (2  +_LOG2F((s) >>2)) : (_LOG2F(s)))
#define _LOG2F(s) (((s) &0x2)                ? (1)                  : (0))

#define LOG2_UINT64 __LOG2A
#define LOG2_UINT32 _LOG2B
#define LOG2_UINT16 _LOG2C
#define LOG2_UINT8  _LOG2D
#define CYCLES_RST_COUNT  (1UL<<((LOG2_UINT64(ISIX_CONFIG_HZ-1)+1)))

/** Reschedule API information for task 
 * @param[in] idle_scheduled idle task is scheduled
 */
void _isixp_schedule_update_statistics( bool idle_scheduled )
{
	ostick_t t = isix_get_jiffies();
	if( idle_scheduled ) 
	{
		cstats.nsum += t>cstats.nlast?t-cstats.nlast:cstats.nlast-t;
		cstats.nlast = t;
	}
	else 
	{
		cstats.isum += t>cstats.ilast?t-cstats.ilast:cstats.ilast-t;
		cstats.ilast = t;
	}
	if( t % CYCLES_RST_COUNT == 0 ) 
	{
		cstats.isum = 0;
		cstats.nsum = 0;
	}
}

#endif /* ISIX_CONFIG_CPU_USAGE_API */

