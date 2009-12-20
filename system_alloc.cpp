/* ------------------------------------------------------------ */
/*
 *    tiny_alloc.cpp
 *
 *     Created on: 2009-08-08
 *     Author: lucck
 *
 * ------------------------------------------------------------ */
#ifndef __UNDER_ISIX__
#include "tiny_alloc.h"
#endif

#include "system_alloc.h"
#include <cstring>
/* -------------------------------------------------------------- */
#ifndef __UNDER_ISIX__
//It can be redefined
#define foundation_alloc tiny_alloc
#define foundation_free tiny_free

#else /*__UNDER_ISIX__*/

void *isix_alloc(size_t size);
void isix_free(void *p);
#define foundation_alloc isix_alloc
#define foundation_free isix_free

#endif /*__UNDER_ISIX__*/

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

