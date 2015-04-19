/*
 * =====================================================================================
 *
 *       Filename:  atomic_ops.h
 *
 *    Description:	Atomic specific operation helper semaphore
 *
 *        Version:  1.0
 *        Created:  15.12.2013 14:52:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include "atomic_int.h"
/*----------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*--------------------------------------------------------------*/
//! Atomic type definition
typedef struct 
{
	int32_t value;
	int32_t limit;
}	sys_atomic_sem_lock_t;

/*--------------------------------------------------------------*/
//! Unlimited value for semaphoe implementation
enum sys_atomic_const_e {
	sys_atomic_unlimited_value = 0
};
/*--------------------------------------------------------------*/
/** Initialize sem locking primitive 
 * @param [out] lock Lock object
* @param [in] value Initial semaphore value 
 */
static inline void sys_atomic_sem_init( sys_atomic_sem_lock_t* lock, int value, int limit )
{
	lock->value = value;
	lock->limit = limit;
}
/*--------------------------------------------------------------*/
/** Function try wait on the spinlock semaphore 
 * @param sem[out] Semaphore primitive object
 * @return non zero if application would block  
 * if semafore is successfuly obtained
 */
static inline int sys_atomic_sem_dec( sys_atomic_sem_lock_t* lock )
{
	int32_t exlck, val, ret;
	asm volatile
	(
		"1: ldrex %[val], [%[lock_addr]]\n"
		"mov %[ret], %[val]\n"
		"cmp %[val], #0\n"
		"it eq\n"
		"clrexeq\n"
		"beq 1f\n"
		"sub %[val],#1\n"
	    "strex %[exlck], %[val], [%[lock_addr]]\n"
		"cmp %[exlck],#0\n"
		"bne 1b\n"
		"1: dmb\n"
		: [val]"=&r"(val), [exlck] "=&r"(exlck), [ret]"=&r"(ret)
		: [lock_addr] "r"(&lock->value)
		: "cc", "memory"
	);
	return ret;
}

/*--------------------------------------------------------------*/
/**
 * Function signal the semaphore
 * @param [out] lock Semaphore primitive object
 * @param [in] Maximum atomic value. If 0 or negative inc is unlimited
 */
static inline int sys_atomic_sem_inc( sys_atomic_sem_lock_t* lock )
{	
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
/*--------------------------------------------------------------*/
/**
 * Clear all exclusive locks
 * @note It should be called during context switching
 */
static inline void sys_atomic_clear_locks(void)
{
	asm volatile("clrex\n");
}
/*--------------------------------------------------------------*/
/**
 * Sys atomic write value
 * @param[in] lock Semaphore object input
 * @param[in] val Input value
 * @return Previous value
 */
static inline int sys_atomic_sem_write_val( sys_atomic_sem_lock_t* lock, int val )
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
/*----------------------------------------------------------*/
/** Sys atomic read value 
 * @param[in] lock Semaphore lock object
 * @return readed value*/
static inline int sys_atomic_sem_read_val( sys_atomic_sem_lock_t* lock ) 
{
	return sys_atomic_read_int32_t( &lock->value );
}

/*----------------------------------------------------------*/
/** Sys atomic wait for interrupt 
*/
static inline void sys_atomic_wait_for_interrupt()
{
	asm volatile("wfi\n");
}
/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------------------*/

