/*
 * =====================================================================================
 *
 *       Filename:  hrtimer.h
 *
 *    Description:  Hi resolution timer API
 *
 *        Version:  1.0
 *        Created:  10.04.2017 19:44:48
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#define SYST_CVR (*((volatile unsigned long*)0xE000E018))
#define SYST_RVR (*((volatile unsigned long*)0xE000E014))

//Get HI resolution timer (must be inline)
static inline __attribute__((always_inline))
	unsigned long port_get_hres_jiffies_timer_value(void)
{
	return SYST_RVR - SYST_CVR;
}

//Get hres timer max value
static inline __attribute__((always_inline))
	unsigned long port_get_hres_jiffies_timer_max_value(void)
{
	return SYST_RVR;
}

#undef SYST_CVR
#undef SYST_RVR


#ifdef __cplusplus
}
#endif

