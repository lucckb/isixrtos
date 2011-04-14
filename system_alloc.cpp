/* ------------------------------------------------------------ */
/*
 *    tiny_alloc.cpp
 *
 *     Created on: 2009-08-08
 *     Author: lucck
 *
 * ------------------------------------------------------------ */
#ifndef COMPILED_UNDER_ISIX
#include "tiny_alloc.h"
#endif

#include "system_alloc.h"
#include <cstring>
/* -------------------------------------------------------------- */
#ifndef COMPILED_UNDER_ISIX
//It can be redefined
#define foundation_alloc fnd::tiny_alloc
#define foundation_free fnd::tiny_free

#else /*COMPILED_UNDER_ISIX*/

namespace isix {
extern "C" {
void *isix_alloc(size_t size);
void isix_free(void *p);
}
}

#define foundation_alloc isix::isix_alloc
#define foundation_free isix::isix_free

#endif /*COMPILED_UNDER_ISIX*/

/* -------------------------------------------------------------- */
void* operator new( size_t n )
{
    if(n==0) n++;
    return foundation_alloc(n);
}

void operator delete( void* p)
{
    if(p)
    	foundation_free(p);
}

void* operator new[]( size_t n)
{
    if(n==0) n++;
    return foundation_alloc(n);
}

void operator delete[]( void* p)
{
    if(p)
    	foundation_free(p);
}

void* malloc(size_t size)
{
	return foundation_alloc(size);
}

void free(void *ptr)
{
	foundation_free(ptr);
}

void *calloc(size_t nmemb, size_t size)
{
	size_t ns = nmemb * size;
	void *ptr = malloc(ns);
	if(ptr)
	{
		std::memset(ptr,0,ns);
	}
	return ptr;
}

void *realloc(void *ptr, size_t size)
{
	//TODO: Not implemented
	return NULL;
}

/* -------------------------------------------------------------- */

