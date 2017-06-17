/*
 * =====================================================================================
 *
 *       Filename:  tlsf_bits.h
 *
 *    Description:  TLSF bits helper
 *
 *        Version:  1.0
 *        Created:  18.06.2017 10:55:10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

static inline int clz(unsigned int x)
{
    return x ? __builtin_clz(x) : sizeof(x) * 8;
}

/* integer binary logarithm (rounding down):
 *   log2(0) == -1, log2(5) == 2
 */
static inline int __log2(unsigned int x)
{
    return sizeof(x) * 8 - clz(x) - 1;
}

/* find first set:
 *   __ffs(1) == 0, __ffs(0) == -1, __ffs(1<<31) == 31
 */
static inline int ls_bit(unsigned int word)
{
    return __log2(word & (unsigned int)(-(unsigned int) word));
}

static inline int ms_bit(unsigned int word)
{
    const int bit = word ? 32 - __builtin_clz(word) : 0;
    return bit - 1;
}


