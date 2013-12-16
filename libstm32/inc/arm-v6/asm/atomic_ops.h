/*
 * =====================================================================================
 *
 *       Filename:  atomic_ops.h
 *
 *    Description:	Atomic specific operation header
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


#ifndef  _ASM_ATOMIC_OPS_H
#define  _ASM_ATOMIC_OPS_H
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
/** Atomic write uint8_t
 * Write and read to selected address. Function
 * should be called from main context
 * @param[out] addr Address with location to write
 * @param[in] val Value to write
 * @return Value read from specified location
 */
static inline uint8_t sys_atomic_write_uint8_t( volatile uint8_t *addr, uint8_t val )
{
	uint8_t ret;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrexb %[ret],[%[addr]]\n"
	   "strexb %[lock],%[val],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr), [val]"r"(val)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** Try write atomic into specified location
 * This function should be called from interrupt context
 * @param[out] addr Atomic location to write
 * @param[in] val Value to write into selected addr
 * @return 1 if unable to access memory
 */
static inline long sys_atomic_try_write_uint8_t( volatile uint8_t *addr, uint8_t val )
{
	long lock;
	asm volatile
	(
		"ldrexb %[lock],[%[addr]]\n"
		"strexb %[lock],%[val],[%[addr]]\n"
		: [lock] "=&r"(lock)
		: [addr] "r"(addr), [val] "r"(val)
		: "cc", "memory"
	);
	return lock;
}

/*----------------------------------------------------------*/
/** Atomic read byte
 * Write and read to selected address. Function
 * should be called from main context
 * @param[out] addr Address with location to write
 * @return Value read from specified location
 */
static inline uint8_t sys_atomic_read_uint8_t( const volatile uint8_t *addr )
{
	uint8_t ret;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrexb %[ret],[%[addr]]\n"
	   "strexb %[lock],%[ret],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"+&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** Atomic write word
 * Write and read to selected address. Function
 * @param[out] addr Address with location to write
 * @param[in] val Value to write
 * @return Value read from specified location
 */
static inline uint32_t sys_atomic_write_uint32_t( volatile uint32_t *addr, uint32_t val )
{
	uint32_t ret;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrex %[ret],[%[addr]]\n"
	   "strex %[lock],%[val],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr), [val]"r"(val)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** Atomic read uint32_t
 * Write and read to selected address. Function
 * should be called from main context
 * @param[out] addr Address with location to write
 * @return Value read from specified location
 */
static inline uint32_t sys_atomic_read_uint32_t( const volatile uint32_t *addr )
{
	uint32_t ret;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrex %[ret],[%[addr]]\n"
	   "strex %[lock],%[ret],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"+&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** Try write atomic into specified location
 * This function should be called from interrupt context
 * @param[out] addr Atomic location to write
 * @param[in] val Value to write into selected addr
 * @return 1 if unable to access memory
 */
static inline long sys_atomic_try_write_uint32_t( volatile uint32_t *addr, uint32_t val )
{
	long lock;
	asm volatile
	(
		"ldrex %[lock],[%[addr]]\n"
		"strex %[lock],%[val],[%[addr]]\n"
		: [lock] "=&r"(lock)
		: [addr] "r"(addr), [val] "r"(val)
		: "cc", "memory"
	);
	return lock;
}
//*----------------------------------------------------------*/
/** Atomic write half word
 * Write and read to selected address. Function
 * @param[out] addr Address with location to write
 * @param[in] val Value to write
 * @return Value read from specified location
 */
static inline uint16_t sys_atomic_write_uint16_t( volatile uint16_t *addr, uint16_t val )
{
	uint16_t ret;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrexh %[ret],[%[addr]]\n"
	   "strexh %[lock],%[val],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr), [val]"r"(val)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** Atomic write 
 * Write and read to selected address. Function
 * should be called from main context
 * @param[out] addr Address with location to write
 * @return Value read from specified location
 */
static inline uint16_t sys_atomic_read_uint16_t( const volatile uint16_t *addr )
{
	uint16_t ret;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrexh %[ret],[%[addr]]\n"
	   "strexh %[lock],%[ret],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"+&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** Try write atomic into specified location
 * This function should be called from interrupt context
 * @param[out] addr Atomic location to write
 * @param[in] val Value to write into selected addr
 * @return 1 if unable to access memory
 */
static inline long sys_atomic_try_write_uint16_t( volatile uint16_t *addr, uint16_t val )
{
	long lock;
	asm volatile
	(
		"ldrexh %[lock],[%[addr]]\n"
		"strexh %[lock],%[val],[%[addr]]\n"
		: [lock] "=&r"(lock)
		: [addr] "r"(addr), [val] "r"(val)
		: "cc", "memory"
	);
	return lock;
}
/*----------------------------------------------------------*/
/** Signed version of int32_t */
static inline int32_t sys_atomic_write_int32_t( volatile int32_t *addr, int32_t val )
{	
	return (int32_t)sys_atomic_write_uint32_t( (volatile uint32_t*)addr, (uint32_t)val );
}
/*----------------------------------------------------------*/
/** Signed version of int32_t  */
static inline int32_t sys_atomic_read_int32_t( const volatile int32_t *addr )
{
	return (int32_t)sys_atomic_read_uint32_t( (volatile uint32_t*)addr );	
}
/*----------------------------------------------------------*/
/** Signed version of int32_t  */
static inline long sys_atomic_try_write_int32_t( volatile int32_t *addr, int32_t val )
{
	return sys_atomic_try_write_uint32_t( (volatile uint32_t*)addr, (uint32_t)val );
}
/*----------------------------------------------------------*/
/** Signed version of int16_t */
static inline int16_t sys_atomic_write_int16_t( volatile int16_t *addr, int16_t val )
{	
	return (int16_t)sys_atomic_write_uint16_t( (volatile uint16_t*)addr, (uint16_t)val );
}
/*----------------------------------------------------------*/
/** Signed version of int16_t  */
static inline int16_t sys_atomic_read_int16_t( const volatile int16_t *addr )
{
	return (int16_t)sys_atomic_read_uint16_t( (volatile uint16_t*)addr );	
}
/*----------------------------------------------------------*/
/** Signed version of int16_t  */
static inline long sys_atomic_try_write_int16_t( volatile int16_t *addr, int16_t val )
{
	return sys_atomic_try_write_uint16_t( (volatile uint16_t*)addr, (uint16_t)val );
}
/*----------------------------------------------------------*/
/** Signed version of int8_t */
static inline int8_t sys_atomic_write_int8_t( volatile int8_t *addr, int8_t val )
{	
	return (int8_t)sys_atomic_write_uint8_t( (volatile uint8_t*)addr, (uint8_t)val );
}
/*----------------------------------------------------------*/
/** Signed version of int8_t  */
static inline int8_t sys_atomic_read_int8_t( const volatile int8_t *addr )
{
	return (int8_t)sys_atomic_read_uint8_t( (volatile uint8_t*)addr );	
}
/*----------------------------------------------------------*/
/** Signed version of int8_t  */
static inline long sys_atomic_try_write_int8_t( volatile int8_t *addr, int8_t val )
{
	return sys_atomic_try_write_uint8_t( (volatile uint8_t*)addr, (uint8_t)val );
}
/*----------------------------------------------------------*/
//Undefine addres off
#undef address_of
/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------------------*/

#endif   /* ----- Atomic locks  ----- */
