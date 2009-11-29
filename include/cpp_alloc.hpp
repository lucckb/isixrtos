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

/*----------------------------------------------------------------*/

void* operator new( size_t n );

void operator delete( void* p);

void* operator new[]( size_t n);

void operator delete[]( void* p);

/* -------------------------------------------------------------- */
#endif

