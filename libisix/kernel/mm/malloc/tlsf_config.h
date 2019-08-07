/*
 * =====================================================================================
 *
 *       Filename:  tlsf_config.h
 *
 *    Description:  TLSF configration file
 *
 *        Version:  1.0
 *        Created:  18.06.2017 11:18:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#define USE_PRINTF (0)

//! Enable TLSF statistics
#define TLSF_STATISTIC (1)

//! Log2 maximum allowed memory block (512KB for embedded fill be sufficient)
#define MAX_FLI (19)


//! Enable locking mechanism
#define	TLSF_USE_LOCKS	(1)
