/*
 * =====================================================================================
 *
 *       Filename:  io.h
 *
 *    Description:  Memory mapped IO
 *
 *        Version:  1.0
 *        Created:  10.04.2017 21:57:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <stdint.h>

//! IOMAP read 8 bits
static inline __attribute__((always_inline))
unsigned int ioread8(uintptr_t addr)
{
	uint8_t val;
	asm volatile("ldrb %0, %1"
		     : "=r" (val)
		     : "Qo" (*(volatile uint8_t *)addr));
	return val;
}

//! IOMAP read 16 bits
static inline __attribute__((always_inline))
unsigned int ioread16(uintptr_t addr)
{
	uint16_t val;
	asm volatile("ldrh %0, %1"
		     : "=r" (val)
		     : "Q" (*(volatile uint16_t *)addr));
	return val;
}


//! IOMAP read 32 bits
static inline __attribute__((always_inline))
unsigned int ioread32(uintptr_t addr)
{
	uint32_t val;
	asm volatile("ldr %0, %1"
		     : "=r" (val)
		     : "Qo" (*(volatile uint32_t *)addr));
	return val;
}


//! IOMAP write 8 bits
static inline __attribute__((always_inline))
void iowrite8(uintptr_t addr, uint8_t value)
{
	asm volatile("strb %1, %0"
		: : "Qo" (*(volatile uint8_t *)addr), "r" (value));
}


//! IOMAP write 16 bits
static inline __attribute__((always_inline))
void iowrite16(uintptr_t addr, uint16_t value)
{
	asm volatile("strh %1, %0"
		: : "Q" (*(volatile uint16_t*)addr), "r" (value));
}


//! IOMAP write 32 bits
static inline __attribute__((always_inline))
void iowrite32(uintptr_t addr, uint32_t value)
{
	asm volatile("str %1, %0"
		: : "Qo" (*(volatile uint32_t *)addr), "r" (value));
}

