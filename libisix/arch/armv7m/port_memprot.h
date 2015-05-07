/*
 * =====================================================================================
 *
 *       Filename:  port_memprot.h
 *
 *    Description:  Port mem protection header
 *
 *        Version:  1.0
 *        Created:  06.05.2015 19:46:36
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
#include <stdbool.h>
#include <stddef.h>


/** Function initialize default memory protection layout just before run
 */
#if ISIX_CONFIG_MEMORY_PROTECTION_MODEL > 0 
void port_memory_protection_set_default_map(void);
#else
#define port_memory_protection_set_default_map() do {} while(0)
#endif


