/*
 * =====================================================================================
 *
 *       Filename:  atomic.h
 *
 *    Description:	Linux compatibile atomic OPS  
 *
 *        Version:  1.0
 *        Created:  12/19/2013 03:09:04 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (lb) 
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#pragma once

#include <stdint.h>
#include <asm/atomic_int.h>

#ifdef __cplusplus
extern "C" {
#endif

//! Atomic basic type
typedef union 
{
	int32_t counter;
	uint32_t ucounter;
} sys_atomic_t;

/** Initialize atomic value  */
static inline void sys_atomic_init( sys_atomic_t* v, int value )
{
	v->counter = value;
}

/** Sys atomic set */
static inline void sys_atomic_write( sys_atomic_t* v, int value ) 
{
	sys_atomic_write_int32_t( &v->counter, value );
}

/** Sys atomic read */
static inline int sys_atomic_read( sys_atomic_t* v )
{
	return sys_atomic_read_int32_t( &v->counter );
}
/** Sys atomic set */
static inline void sys_atomic_write_unsigned( sys_atomic_t* v, unsigned value ) 
{
	sys_atomic_write_uint32_t( &v->ucounter, value );
}

/** Sys atomic read */
static inline unsigned sys_atomic_read_unsigned( sys_atomic_t* v )
{
	return sys_atomic_read_uint32_t( &v->ucounter );
}

/** Atomic add integer value */
static inline void sys_atomic_add( int i, sys_atomic_t* v )
{
	unsigned long tmp;
	int result;

	asm volatile (
	"1:	ldrex	%0, [%3]\n"
	"	add	%0, %0, %4\n"
	"	strex	%1, %0, [%3]\n"
	"	teq	%1, #0\n"
	"	bne	1b\n"
	: "=&r" (result), "=&r" (tmp), "+Qo" (v->counter)
	: "r" (&v->counter), "Ir" (i)
	: "cc");
}

/**  Atomic add and return value  */
static inline int sys_atomic_add_return( int i, sys_atomic_t *v)
{
	unsigned long tmp;
	int result;

	asm volatile (
	"	dmb\n"
	"1:	ldrex	%0, [%3]\n"
	"	add	%0, %0, %4\n"
	"	strex	%1, %0, [%3]\n"
	"	teq	%1, #0\n"
	"	bne	1b\n"
	"	dmb\n"
	: "=&r" (result), "=&r" (tmp), "+Qo" (v->counter)
	: "r" (&v->counter), "Ir" (i)
	: "cc");

	return result;
}

/** Atomic sub */
static inline void sys_atomic_sub( int i, sys_atomic_t *v )
{
	unsigned long tmp;
	int result;

	asm volatile (
	"1:	ldrex	%0, [%3]\n"
	"	sub	%0, %0, %4\n"
	"	strex	%1, %0, [%3]\n"
	"	teq	%1, #0\n"
	"	bne	1b\n"
	: "=&r" (result), "=&r" (tmp), "+Qo" (v->counter)
	: "r" (&v->counter), "Ir" (i)
	: "cc");
}

/** Atomic sub and return */
static inline int sys_atomic_sub_return( int i, sys_atomic_t *v )
{
	unsigned long tmp;
	int result;

	asm volatile (
	" 	dmb\n"
	"1:	ldrex	%0, [%3]\n"
	"	sub	%0, %0, %4\n"
	"	strex	%1, %0, [%3]\n"
	"	teq	%1, #0\n"
	"	bne	1b\n"
	"	dmb\n"
	: "=&r" (result), "=&r" (tmp), "+Qo" (v->counter)
	: "r" (&v->counter), "Ir" (i)
	: "cc");

	return result;
}



#ifdef __cplusplus
}
#endif
