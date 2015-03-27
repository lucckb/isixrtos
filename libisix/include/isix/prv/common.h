/*
 * =====================================================================================
 *
 *       Filename:  common.h
 *
 *    Description:  Common API for isix RTOS
 *
 *        Version:  1.0
 *        Created:  25.03.2015 23:05:10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include "isix_config.h"
#include <stddef.h>

#ifdef __cplusplus 
namespace isix {
#endif

static inline __attribute__((always_inline)) 
size_t _isixp_align_size( size_t len )
{
	if( len & (ISIX_CONFIG_BYTE_ALIGNMENT_SIZE-1) )
		return len + ISIX_CONFIG_BYTE_ALIGNMENT_SIZE - (len&(ISIX_CONFIG_BYTE_ALIGNMENT_SIZE-1) );
	else
		return len;
}

#ifdef __cplusplus
}
#endif

