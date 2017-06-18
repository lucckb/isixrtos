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

#include <isix/config.h>
#include <stddef.h>

//! Get aligned memory req
static inline __attribute__((always_inline))
size_t _isixp_align_size( size_t len )
{
	if( len & (ISIX_BYTE_ALIGNMENT_SIZE-1) )
		return len + ISIX_BYTE_ALIGNMENT_SIZE - (len&(ISIX_BYTE_ALIGNMENT_SIZE-1) );
	else
		return len;
}



