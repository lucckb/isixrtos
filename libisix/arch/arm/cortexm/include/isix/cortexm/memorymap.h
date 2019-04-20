/*
 * =====================================================================================
 *
 *       Filename:  memorymap.h
 *
 *    Description:  Standard isix memory MAP
 *
 *        Version:  1.0
 *        Created:  15.04.2017 18:27:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

/* Private peripheral bus - Internal */
#define PPBI_BASE                       (0xE0000000U)

/* User peripheral base */
#define PBI_BASE						(0x40000000U)

/** Region size */
#define PBI_SIZE						(0x20000000U-1U)
#define PPBI_SIZE						(0x20000000U-1U)

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* ITM: Instrumentation Trace Macrocell */
#ifndef ITM_BASE
#define ITM_BASE                        (PPBI_BASE + 0x0000)
#endif

/* DWT: Data Watchpoint and Trace unit */
#ifndef DWT_BASE
#define DWT_BASE                        (PPBI_BASE + 0x1000)
#endif

/* FPB: Flash Patch and Breakpoint unit */
#define FPB_BASE                        (PPBI_BASE + 0x2000)
#endif

/* PPBI_BASE + 0x3000 (0xE000 3000 - 0xE000 DFFF): Reserved */

#ifndef SCS_BASE
#define SCS_BASE                        (PPBI_BASE + 0xE000)
#endif

/* PPBI_BASE + 0xF000 (0xE000 F000 - 0xE003 FFFF): Reserved */

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#define TPIU_BASE                       (PPBI_BASE + 0x40000)
#endif

/* --- SCS: System Control Space --- */

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* ITR: Interrupt Type Register */
#define ITR_BASE                        (SCS_BASE + 0x0000)
#endif

/* SYS_TICK: System Timer */
#define SYS_TICK_BASE                   (SCS_BASE + 0x0010)

/* NVIC: Nested Vector Interrupt Controller */
#ifndef NVIC_BASE
#define NVIC_BASE                       (SCS_BASE + 0x0100)
#endif

/* SCB: System Control Block */
#ifndef SCB_BASE
#define SCB_BASE                        (SCS_BASE + 0x0D00)
#endif

/* MPU: Memory protection unit */
#ifndef MPU_BASE
#define MPU_BASE                        (SCS_BASE + 0x0D90)
#endif

/* Those defined only on CM0*/
#if defined(__ARM_ARCH_6M__)
/* DEBUG: Debug control and configuration */
#define DEBUG_BASE                      (SCS_BASE + 0x0DF0)
#endif

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* STE: Software Trigger Interrupt Register */
#define STIR_BASE                       (SCS_BASE + 0x0F00)
/* ID: ID space */
#define ID_BASE                         (SCS_BASE + 0x0FD0)
#endif

static inline int memmap_is_periph_addr(unsigned long addr) {
		return( (addr>=PPBI_BASE&&addr<=(PPBI_BASE+PPBI_SIZE)) ||
			    (addr>=PBI_BASE&&addr<=(PBI_BASE+PBI_SIZE)) );
	}

