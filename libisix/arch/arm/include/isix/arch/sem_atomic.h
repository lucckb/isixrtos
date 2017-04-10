/*
 * =====================================================================================
 *
 *       Filename:  _isix_port_atomic.h
 *
 *    Description:  Port atomic semaphore implementation
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

#include <stdint.h>

//! Atomic type definition
typedef struct
{
	int32_t value;
	int32_t limit;
}	_isix_port_atomic_sem_t;

//! Unlimited value for semaphoe implementation
enum __isix_port_atomic_const_e {
	sys_atomic_unlimited_value = 0
};


/** Initialize sem locking primitive
 * @param [out] lock Lock object
 * @param [in] value Initial semaphore value
 */
static inline __attribute__((always_inline))
	void _isix_port_atomic_sem_init( _isix_port_atomic_sem_t* lock, int value , int limit ) {
		lock->value = value;
		lock->limit = limit;
	}


/** Function try wait on the spinlock semaphore
 * @param sem[out] Semaphore primitive object
 * @return false if lock failed or positive sem value
 * if semafore is successfuly obtained
 */
static inline __attribute__((always_inline))
	int _isix_port_atomic_sem_dec( _isix_port_atomic_sem_t* lock ) {
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
	int _isix_port_atomic_sem_trydec( _isix_port_atomic_sem_t* lock ) {
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
	int _isix_port_atomic_sem_inc( _isix_port_atomic_sem_t* lock ) {
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
void _isix_port_atomic_clear_locks(void)
{
	asm volatile("clrex\n");
}

/** Sys atomic read value
 * @param[in] lock Semaphore lock object
 * @return readed value*/
static inline __attribute__((always_inline))
long _isix_port_atomic_sem_read_val( _isix_port_atomic_sem_t* semcnt )
{
	long ret;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrex %[ret],[%[addr]]\n"
	   "strex %[lock],%[ret],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
	   "dmb\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(&semcnt->value)
		: "cc", "memory"
	);
	return ret;
}

/**
 * Sys atomic write value
 * @param[in] lock Semaphore object input
 * @param[in] val Input value
 * @return Previous value
 */
static inline __attribute__((always_inline))
int _isix_port_atomic_sem_write_val( _isix_port_atomic_sem_t* semcnt, int val )
{
	long ret;
	unsigned long lock;
	if( semcnt->limit > 0 ) {
		if( val > semcnt->limit ) {
			val = semcnt->limit;
		}
	}
	if( val < 0 ) {
		val = 0;
	}
	asm volatile
	(
	"1:	ldrex %[ret],[%[addr]]\n"
	   "strex %[lock],%[val],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
	   "dmb\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(&semcnt->value), [val]"r"(val)
		: "cc", "memory"
	);
	return ret;
}

