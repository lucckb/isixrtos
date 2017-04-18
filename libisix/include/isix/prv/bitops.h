#pragma once

#include <limits.h>

#define __BITS_PER_LONG (sizeof(unsigned long)*CHAR_BIT)

static inline void set_bit(int nr, unsigned long *addr)
{
	addr[nr / __BITS_PER_LONG] |= 1UL << (nr % __BITS_PER_LONG);
}

static inline void clear_bit(int nr, unsigned long *addr)
{
	addr[nr / __BITS_PER_LONG] &= ~(1UL << (nr % __BITS_PER_LONG));
}

static __attribute__((always_inline)) inline
int test_bit(unsigned int nr, const unsigned long *addr)
{
	return ((1UL << (nr % __BITS_PER_LONG)) &
		(((unsigned long *)addr)[nr / __BITS_PER_LONG])) != 0;
}



