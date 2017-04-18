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



//Get HI resolution timer (must be inline)
unsigned long _isix_port_get_hres_jiffies_timer_value(void);

//Get hres timer max value
unsigned long _isix_port_get_hres_jiffies_timer_max_value(void);



#ifdef __cplusplus
}
#endif

