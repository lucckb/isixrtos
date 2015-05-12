/*
 * =====================================================================================
 *
 *       Filename:  osstats.h
 *
 *    Description:  OS statistics private ISIX API
 *
 *        Version:  1.0
 *        Created:  12.05.2015 19:34:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <isix/types.h>

#ifdef ISIX_CONFIG_CPU_USAGE_API
struct isix_task;

/** Reschedule API information for task 
 * @param[in] new_task New rescheduled task
 * @param[in] old_task Previously scheduled task
 */
void _isixp_schedule_update_statistics( 
	const struct isix_task* new_task, 
	const struct isix_task* old_task );
#else
#define _isixp_schedule_update_statistics(o,n) do {} while(0)
#endif
