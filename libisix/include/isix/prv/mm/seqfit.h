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

//! Initialize global heap
void _isixp_seqfit_alloc_init(void);

//! Allocate memory
void* _isixp_seqfit_alloc(size_t size);

//! Free memory
void _isixp_seqfit_free(void *p);

//! Return heap info
size_t _isixp_seqfit_heap_free(int *fragments);

//! Get real region size
size_t _isixp_seqfit_heap_getsize( void* ptr );

//Simple realloc implementation
void *_isixp_seqfit_realloc(void *ptr, size_t size );


