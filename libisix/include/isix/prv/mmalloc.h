/*
 * =====================================================================================
 *
 *       Filename:  mmalloc.h
 *
 *    Description:  Private memory allocation info
 *
 *        Version:  1.0
 *        Created:  17.06.2017 18:24:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#ifndef _ISIX_KERNEL_CORE_
#	error This is private header isix kernel headers cannot be used by app
#endif

//! Initialize global heap
void _isixp_alloc_init(void);


