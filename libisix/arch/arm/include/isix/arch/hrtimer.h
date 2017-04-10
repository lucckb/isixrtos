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


#include <isix/arch/io.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#define _ISIX_PORT_SYST_CVR 0xE000E018
#define _ISIX_PORT_SYST_RVR 0xE000E014

//Get HI resolution timer (must be inline)
static inline __attribute__((always_inline))
	unsigned long _isix_port_get_hres_jiffies_timer_value(void)
{
	return ioread32(_ISIX_PORT_SYST_RVR) - ioread32(_ISIX_PORT_SYST_CVR);
}

//Get hres timer max value
static inline __attribute__((always_inline))
	unsigned long _isix_port_get_hres_jiffies_timer_max_value(void)
{
	return ioread32(_ISIX_PORT_SYST_RVR);
}

#undef _ISIX_PORT_SYST_CVR
#undef _ISIX_PORT_SYST_RVR


#ifdef __cplusplus
}
#endif

