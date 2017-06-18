/*
 * =====================================================================================
 *
 *       Filename:  seqfit.h
 *
 *    Description:  seqfit memory allocator
 *
 *        Version:  1.0
 *        Created:  16.06.2017 19:13:09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <stddef.h>



#define seqfit_alloc_init _isixp_alloc_init
#define seqfit_alloc isix_alloc
#define seqfit_free isix_free
#define seqfit_heap_stats isix_heap_stats
#define seqfit_heap_getsize isix_heap_getsize
#define seqfit_realloc isix_realloc


struct isix_memory_stat;

//! Initialize global heap
void seqfit_alloc_init(void);

//! Allocate memory
void* seqfit_alloc(size_t size);

//! Free memory
void seqfit_free(void *p);

//! Return heap info
void seqfit_heap_stats( struct isix_memory_stat* meminfo );

//! Get real region size
size_t seqfit_heap_getsize( void* ptr );

//Simple realloc implementation
void* seqfit_realloc(void *ptr, size_t size );


