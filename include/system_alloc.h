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
#include <stddef.h>
/*----------------------------------------------------------------*/
#if defined(__cplusplus) && !defined(CONFIG_ENABLE_EXCEPTIONS)
#include <new>

//New delete operators
void* operator new( size_t n )  throw();

void operator delete( void* p)  throw();

void* operator new[]( size_t n)  throw();

void operator delete[]( void* p)  throw();

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

