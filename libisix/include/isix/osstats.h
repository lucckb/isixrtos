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


#if CONFIG_ISIX_CPU_USAGE_API


#ifdef __cplusplus
extern "C" {
#endif

/** Return the current CPU load of the system
 * The value is in per mille and is refreshed at the first context switch
 * after each window of about 1024 ticks, so it is up to date also in tickless mode.
 * @return CPUload in profiles for ex 1000
 */
int isix_cpuload( void );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace isix {
namespace {
	inline int cpuload() {
		return ::isix_cpuload();
	}
}}
#endif /* __cplusplus */

#endif /* CONFIG_ISIX_CPU_USAGE_API  */

