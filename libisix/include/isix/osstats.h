/*
 * =====================================================================================
 *
 *       Filename:  osstats.h
 *
 *    Description:  OS statistic API
 *
 *        Version:  1.0
 *        Created:  12.05.2015 19:26:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <isix/config.h>
#include <isix/types.h>

#ifdef ISIX_CONFIG_CPU_USAGE_API
/** Return the current CPU load of the system 
 * @return CPUload in profiles for ex 1000
 */
int isix_cpuload( void );

#endif

