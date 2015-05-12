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
 * @param[in] idle_scheduled idle task is scheduled
 */
void _isixp_schedule_update_statistics( bool idle_scheduled );

#else
#define _isixp_schedule_update_statistics(i) \
	do { (void)i; } while(0)
#endif
