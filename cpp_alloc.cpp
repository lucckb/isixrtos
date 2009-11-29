/* ------------------------------------------------------------ */
/*
 *    tiny_alloc.cpp
 *
 *     Created on: 2009-08-08
 *     Author: lucck
 *
 * ------------------------------------------------------------ */

#include "tiny_alloc.h"

/* -------------------------------------------------------------- */
void* operator new( size_t n ) { return tiny_alloc(n); }

void operator delete( void* p) { return tiny_free(p); }

void* operator new[]( size_t n) { return tiny_alloc(n); }

void operator delete[]( void* p) { return tiny_free(p); }

/* -------------------------------------------------------------- */

