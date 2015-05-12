/*
 * =====================================================================================
 *
 *       Filename:  port_atomic.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/17/2013 10:00:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (lb)
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#pragma once


#include <asm/atomic.h>

//! Atomic type definition
typedef struct 
{
	int32_t value;
	int32_t limit;
}	_port_atomic_sem_t;

//! Unlimited value for semaphoe implementation
enum _port_atomic_const_e {
	sys_atomic_unlimited_value = 0
};


//! Atomic counter type
typedef sys_atomic_t _port_atomic_int_t;


/** Initialize sem locking primitive 
 * @param [out] lock Lock object
 * @param [in] value Initial semaphore value 
 */
static inline __attribute__((always_inline))
	void port_atomic_sem_init( _port_atomic_sem_t* lock, int value , int limit ) {
		lock->value = value;
		lock->limit = limit;
	}


/** Function try wait on the spinlock semaphore 
 * @param sem[out] Semaphore primitive object
 * @return false if lock failed or positive sem value
 * if semafore is successfuly obtained
 */
static inline __attribute__((always_inline))
	int port_atomic_sem_dec( _port_atomic_sem_t* lock ) {
		int32_t exlck, val;
		asm volatile
			(
			 "1: ldrex %[val], [%[lock_addr]]\n"
			 "sub %[val],#1\n"
			 "strex %[exlck], %[val], [%[lock_addr]]\n"
			 "cmp %[exlck],#0\n"
			 "bne 1b\n"
			 "dmb\n"
			 : [val]"=&r"(val), [exlck] "=&r"(exlck)
			 : [lock_addr] "r"(&lock->value)
			 : "cc", "memory"
			);
		return val;
	}

/** Function try wait on the spinlock semaphore 
 * @param sem[out] Semaphore primitive object
 * @return false if lock failed or positive sem value
 * if semafore is successfuly obtained
 */
static inline __attribute__((always_inline))
	int port_atomic_sem_trydec( _port_atomic_sem_t* lock ) {
		int32_t exlck, val, ret;
		asm volatile
			(
			 "1: ldrex %[val], [%[lock_addr]]\n"
			 "mov %[ret], %[val]\n"
			 "subs %[val],#1\n"
			 "it mi\n"
			 "addmi %[val],#1\n"
			 "strex %[exlck], %[val], [%[lock_addr]]\n"
			 "cmp %[exlck],#0\n"
			 "bne 1b\n"
			 "dmb\n"
			 : [val]"=&r"(val), [exlck] "=&r"(exlck), [ret]"=&r"(ret)
			 : [lock_addr] "r"(&lock->value)
			 : "cc", "memory"
			);
		return ret;
	}
/**
 * Function signal the semaphore
 * @param [out] sem Semaphore primitive object
 */
static inline __attribute__((always_inline))
	int port_atomic_sem_inc( _port_atomic_sem_t* lock ) {
		int32_t exlck, val;
		asm volatile
			(
			 "1: ldrex %[val], [%[lock_addr]]\n"
			 "add %[val], #1\n"
			 "cmp %[maxval], #0\n"
			 "itt gt\n"
			 "cmpgt %[val], %[maxval]\n"
			 "movgt %[val], %[maxval]\n"
			 "strex %[exlck], %[val], [%[lock_addr]]\n"
			 "cmp %[exlck],#0\n"
			 "bne 1b\n"
			 "dmb\n"
			 : [val]"=&r"(val), [exlck] "=&r"(exlck) 
			 : [lock_addr] "r"(&lock->value), [maxval]"r"(lock->limit)
			 : "cc", "memory"
			);
		return val;
	}


/**
 * Clear all exclusive locks
 * @note It should be called during context switching
 */
static inline __attribute__((always_inline))
void port_atomic_clear_locks(void) 
{
	asm volatile("clrex\n");
}

/** Sys atomic read value 
 * @param[in] lock Semaphore lock object
 * @return readed value*/
static inline __attribute__((always_inline))
long port_atomic_sem_read_val( _port_atomic_sem_t* lock ) 
{
	return sys_atomic_read_int32_t( &lock->value );
}

/**
 * Sys atomic write value
 * @param[in] lock Semaphore object input
 * @param[in] val Input value
 * @return Previous value
 */
static inline __attribute__((always_inline))
int port_atomic_sem_write_val( _port_atomic_sem_t* lock, int val )
{
	if( lock->limit > 0 ) {
		if( val > lock->limit ) {
			val = lock->limit;
		}
	}
	if( val < 0 ) {
		val = 0;
	}
	return sys_atomic_write_int32_t( &lock->value, val );
}

/** Sys atomic initialize 
 * @param[in] val Atomic value type
 * @param[in] value Initial value
 */
static inline __attribute__((always_inline))
void port_atomic_init( _port_atomic_int_t* val, int value )
{
	sys_atomic_init( val , value );
}

/** 
 *	Port atomic increment
 *	@param[in] val Atomic type
 *	@return returned value
 */
static inline __attribute__((always_inline))
int port_atomic_inc( _port_atomic_int_t* val ) 
{
	return sys_atomic_add_return( 1, val );
}

/** 
 *	Port atomic decrement
 *	@param[in] val Atomic type
 *	@return returned value
 */
static inline __attribute__((always_inline))
int port_atomic_dec( _port_atomic_int_t* val ) 
{
	return sys_atomic_sub_return( 1, val );
}

/**
 * Sys atomic read value
 * @param[in] val Atomic type
 * @return counter value
 */
static inline __attribute__((always_inline))
int port_atomic_read( _port_atomic_int_t* val )
{
	return sys_atomic_read( val );
}

/** 
 *	Port atomic increment
 *	@param[in] val Atomic type
 *	@return returned value
 */
static inline __attribute__((always_inline))
unsigned port_unsigned_atomic_inc( _port_atomic_int_t* val ) 
{
	return (unsigned)sys_atomic_add_return( 1, val );
}

/** 
 *	Port atomic decrement
 *	@param[in] val Atomic type
 *	@return returned value
 */
static inline __attribute__((always_inline))
unsigned port_unsigned_atomic_dec( _port_atomic_int_t* val ) 
{
	return (unsigned)sys_atomic_sub_return( 1, val );
}

/**
 * Sys atomic read value
 * @param[in] val Atomic type
 * @return counter value
 */
static inline __attribute__((always_inline))
unsigned port_unsigned_atomic_read( _port_atomic_int_t* val )
{
	return sys_atomic_read_unsigned( val );
}

/**
 * Sys atomic read value
 * @param[in] val Atomic type
 * @return counter value
 */
static inline __attribute__((always_inline))
void port_unsigned_atomic_write( _port_atomic_int_t* v,  unsigned value )
{
	sys_atomic_write_unsigned( v, value );
}

/** Atomic compare and exchange  */
static inline __attribute__((always_inline))
uintptr_t port_cmpxchg( uintptr_t *ptr, uintptr_t old, uintptr_t newv )
{
	return sys_cmpxchg( ptr, old, newv );
}

static inline __attribute__((always_inline))
uintptr_t port_atomic_read_uintptr_t( const uintptr_t* ptr )
{	
	return sys_atomic_read_uintptr_t( ptr );
}
