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


#pragma once

#include <stdint.h>
#include <assert.h>
/*----------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

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
	   "dmb\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr), [val]"r"(val)
		: "cc", "memory"
	);
	return ret;
}

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
	    "dmb\n"
		: [lock] "=&r"(lock)
		: [addr] "r"(addr), [val] "r"(val)
		: "cc", "memory"
	);
	return lock;
}


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
	   "dmb\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr)
		: "cc", "memory"
	);
	return ret;
}

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
	   "dmb\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr), [val]"r"(val)
		: "cc", "memory"
	);
	return ret;
}

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
	   "dmb\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr)
		: "cc", "memory"
	);
	return ret;
}

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
	    "dmb\n"
		: [lock] "=&r"(lock)
		: [addr] "r"(addr), [val] "r"(val)
		: "cc", "memory"
	);
	return lock;
}

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
	   "dmb\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr), [val]"r"(val)
		: "cc", "memory"
	);
	return ret;
}

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
	   "dmb\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr)
		: "cc", "memory"
	);
	return ret;
}

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
	    "dmb\n"
		: [lock] "=&r"(lock)
		: [addr] "r"(addr), [val] "r"(val)
		: "cc", "memory"
	);
	return lock;
}

/** Signed version of int32_t */
static inline int32_t sys_atomic_write_int32_t( volatile int32_t *addr, int32_t val )
{	
	return (int32_t)sys_atomic_write_uint32_t( (volatile uint32_t*)addr, (uint32_t)val );
}

/** Signed version of int32_t  */
static inline int32_t sys_atomic_read_int32_t( const volatile int32_t *addr )
{
	return (int32_t)sys_atomic_read_uint32_t( (volatile uint32_t*)addr );	
}

/** Signed version of int32_t  */
static inline long sys_atomic_try_write_int32_t( volatile int32_t *addr, int32_t val )
{
	return sys_atomic_try_write_uint32_t( (volatile uint32_t*)addr, (uint32_t)val );
}

/** Signed version of int16_t */
static inline int16_t sys_atomic_write_int16_t( volatile int16_t *addr, int16_t val )
{	
	return (int16_t)sys_atomic_write_uint16_t( (volatile uint16_t*)addr, (uint16_t)val );
}

/** Signed version of int16_t  */
static inline int16_t sys_atomic_read_int16_t( const volatile int16_t *addr )
{
	return (int16_t)sys_atomic_read_uint16_t( (volatile uint16_t*)addr );	
}

/** Signed version of int16_t  */
static inline long sys_atomic_try_write_int16_t( volatile int16_t *addr, int16_t val )
{
	return sys_atomic_try_write_uint16_t( (volatile uint16_t*)addr, (uint16_t)val );
}

/** Signed version of int8_t */
static inline int8_t sys_atomic_write_int8_t( volatile int8_t *addr, int8_t val )
{	
	return (int8_t)sys_atomic_write_uint8_t( (volatile uint8_t*)addr, (uint8_t)val );
}

/** Signed version of int8_t  */
static inline int8_t sys_atomic_read_int8_t( const volatile int8_t *addr )
{
	return (int8_t)sys_atomic_read_uint8_t( (volatile uint8_t*)addr );	
}

/** Signed version of int8_t  */
static inline long sys_atomic_try_write_int8_t( volatile int8_t *addr, int8_t val )
{
	return sys_atomic_try_write_uint8_t( (volatile uint8_t*)addr, (uint8_t)val );
}

/** Atomic compare and exchange  */
static inline uintptr_t sys_cmpxchg( uintptr_t *ptr, uintptr_t old, uintptr_t newv )
{
	uintptr_t oldval, res;

	asm volatile("dmb\n");
	do {
		asm volatile(
				"       ldrex   %1, [%2]\n"
				"       mov     %0, #0\n"
				"       teq     %1, %3\n"
				"		it 		eq\n"
				"       strexeq %0, %4, [%2]\n"
				: "=&r" (res), "=&r" (oldval)
				: "r" (ptr), "Ir" (old), "r" (newv)
				: "memory", "cc");
	} while (res);
	asm volatile("dmb\n");
	return oldval;
}

/** Sys atimic read uintptr_t */
static inline uintptr_t sys_atomic_read_uintptr_t( const uintptr_t* ptr )
{
	static_assert(sizeof(uintptr_t)==sizeof(uint32_t), "ARMv4 support only 4b atomics");
	return (uintptr_t)sys_atomic_read_uint32_t( (volatile uint32_t*)ptr );	
}

#ifdef __cplusplus
}
#endif


