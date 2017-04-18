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

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* ITM: Instrumentation Trace Macrocell */
#define ITM_BASE                        (PPBI_BASE + 0x0000)

/* DWT: Data Watchpoint and Trace unit */
#define DWT_BASE                        (PPBI_BASE + 0x1000)

/* FPB: Flash Patch and Breakpoint unit */
#define FPB_BASE                        (PPBI_BASE + 0x2000)
#endif

/* PPBI_BASE + 0x3000 (0xE000 3000 - 0xE000 DFFF): Reserved */

#define SCS_BASE                        (PPBI_BASE + 0xE000)

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
#define NVIC_BASE                       (SCS_BASE + 0x0100)

/* SCB: System Control Block */
#define SCB_BASE                        (SCS_BASE + 0x0D00)

/* MPU: Memory protection unit */
#define MPU_BASE                        (SCS_BASE + 0x0D90)

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

