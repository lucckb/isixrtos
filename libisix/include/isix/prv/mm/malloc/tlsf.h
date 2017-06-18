/*
 * Two Levels Segregate Fit memory allocator (TLSF)
 * Version 2.4.6
 *
 * Written by Miguel Masmano Tello <mimastel@doctor.upv.es>
 *
 * Thanks to Ismael Ripoll for his suggestions and reviews
 *
 * Copyright (C) 2008, 2007, 2006, 2005, 2004
 *
 * This code is released using a dual license strategy: GPL/LGPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of the GNU General Public License Version 2.0
 * Released under the terms of the GNU Lesser General Public License Version 2.1
 *
 */

#ifndef _TLSF_H_
#define _TLSF_H_

#include <sys/types.h>


#define init_memory_pool _isixp_mmtlsf_init_memory_pool
#define get_used_size _isixp_mmtlsf_get_used_size
#define get_max_size _isixp_mmtlsf_get_max_size
#define destroy_memory_pool _isixp_mmtlsf_destroy_memory_pool
#define add_new_area _isixp_mmtlsf_add_new_area
#define malloc_ex _isixp_mmtlsf_malloc_ex
#define free_ex _isixp_mmtlsf_free_ex
#define realloc_ex _isixp_mmtlsf_realloc_ex
#define calloc_ex _isixp_mmtlsf_calloc_ex

#define tlsf_malloc isix_alloc
#define tlsf_free isix_free
#define tlsf_realloc isix_realloc
#define tlsf_calloc _isixp_mmtlsf_tlsf_calloc

#define get_free_size  _isixp_mmtlsf_get_free_size
#define get_block_size _isixp_mmtlsf_get_block_size

extern size_t init_memory_pool(size_t, void *);
extern size_t get_used_size(void *);
extern size_t get_max_size(void *);
extern void destroy_memory_pool(void *);
extern size_t add_new_area(void *, size_t, void *);
extern void *malloc_ex(size_t, void *);
extern void free_ex(void *, void *);
extern void *realloc_ex(void *, size_t, void *);
extern void *calloc_ex(size_t, size_t, void *);

extern void *tlsf_malloc(size_t size);
extern void tlsf_free(void *ptr);
extern void *tlsf_realloc(void *ptr, size_t size);
extern void *tlsf_calloc(size_t nelem, size_t elem_size);

/** Stuff added by LB */
size_t get_free_size(void *mem_pool);
size_t get_block_size(void *ptr);

#endif
