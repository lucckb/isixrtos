/*
 * =====================================================================================
 *
 *       Filename:  atomic_locks.h
 *
 *    Description:	ARM spinlock semaphore implementation
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


#ifndef  _ASM_ATOMIC_LOCKS_H
#define  _ASM_ATOMIC_LOCKS_H
/*--------------------------------------------------------------*/
#include <stddef.h>

/*--------------------------------------------------------------*/
//! Temporary define address of
#define address_of(ptr, type, member) \
	(uintptr_t)(ptr) + offsetof(type,member)
/*--------------------------------------------------------------*/
#ifdef __cplusplus
namespace sys {
#endif
/*--------------------------------------------------------------*/
//! Atomic type definition
typedef struct 
{
	long value;
}	sys_atomic_sem_lock_t;

/*--------------------------------------------------------------*/
/** Initialize sem locking primitive 
 * @param [out] lock Lock object
 * @param [in] value Initial semaphore value 
 */
static inline void sys_atomic_sem_init( sys_atomic_sem_lock_t* lock, long value )
{
	lock->value = value;
}
/*--------------------------------------------------------------*/
/** Function try wait on the spinlock semaphore 
 * @param sem[out] Semaphore primitive object
 * @return false if lock failed or positive sem value
 * if semafory is successfuly obtained
 */
static inline int sys_atomic_try_sem_dec( sys_atomic_sem_lock_t* lock )
{
	long exlck, val;
	asm volatile
	(
		"1: ldrex %[val], [%[lock_addr]]\n"
		"cmp %[val], #0\n"
		"beq 1f\n"
		"sub %[val],#1\n"
	    "strex %[exlck], %[val], [%[lock_addr]]\n"
		"cmp %[exlck],#0\n"
		"bne 1b\n"
		"1: dmb\n"
		: [lock_addr] "+r"( address_of(lock,sys_atomic_sem_lock_t,value) ),
		  [exlck] "=&r"(exlck), [val]"+r"(val)
		:
		: "cc", "memory"
	);
	return val;
}
/*--------------------------------------------------------------*/
/**
 * Function signal the semaphore
 * @param [out] sem Semaphore primitive object
 */
static inline int sys_atomic_try_sem_inc( sys_atomic_sem_lock_t* lock )
{	
	long exlck, val;
	asm volatile
	(
		"1: ldrex %[val], [%[lock_addr]]\n"
		"add %[val], #1\n"
	    "strex %[exlck], %[val], [%[lock_addr]]\n"
		"cmp %[exlck],#0\n"
		"bne 1b\n"
		"dmb\n"
		: [lock_addr] "+r"( address_of(lock,sys_atomic_sem_lock_t,value) ),
		  [exlck] "=&r"(exlck), [val]"+r"(val)
		:
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
/*----------------------------------------------------------*/
#undef address_of
/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------------------*/

#endif   /* ----- Atomic locks  ----- */
