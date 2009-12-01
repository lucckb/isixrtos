/*----------------------------------------------------------------*/
/*
 *
 *  New heap allocator for the ISIX
 *  Created on: 2009-11-11
 *      Author: lucck
 */
/*----------------------------------------------------------------*/
#ifndef __CPP_ALLOC_HPP__
#define __CPP_ALLOC_HPP__
/*----------------------------------------------------------------*/
#include "tiny_alloc.h"
#include <new>
/*----------------------------------------------------------------*/
#ifdef __cplusplus
//New delete operators
void* operator new( size_t n );

void operator delete( void* p);

void* operator new[]( size_t n);

void operator delete[]( void* p);

#endif

/* -------------------------------------------------------------- */
//Standard system alloc redefs
#ifdef __cplusplus
 extern "C" {
#endif

void* malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);

#ifdef __cplusplus
 }
#endif

/* -------------------------------------------------------------- */
#endif

