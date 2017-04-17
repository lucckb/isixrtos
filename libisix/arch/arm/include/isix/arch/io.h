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

/* Generic memory-mapped I/O accessor functions */
#define MMIO8(addr)		(*(volatile uint8_t *)((uintptr_t)(addr)))
#define MMIO16(addr)		(*(volatile uint16_t *)((uintptr_t)(addr)))
#define MMIO32(addr)		(*(volatile uint32_t *)((uintptr_t)(addr)))
#define MMIO64(addr)		(*(volatile uint64_t *)((uintptr_t)(addr)))

