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


#ifdef ISIX_CONFIG_CPU_USAGE_API
/** Return the current CPU load of the system 
 * @return CPUload in profiles for ex 1000
 */
int isix_cpuload( void )
{

}

/** Reschedule API information for task 
 * @param[in] new_task New rescheduled task
 * @param[in] old_task Previously scheduled task
 */
void _isixp_schedule_update_statistics( 
	const struct isix_task* new_task, const struct isix_task* old_task )
{

}

#endif

