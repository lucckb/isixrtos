/*
 * =====================================================================================
 *
 *       Filename:  nvic_memmap.h
 *
 *    Description:  NVIC memory map
 *
 *        Version:  1.0
 *        Created:  15.04.2017 18:48:20
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include "memorymap.h"


/* ISER: Interrupt Set Enable Registers */
/* Note: 8 32bit Registers */
/* Note: Single register on CM0 */
#define NVIC_ISER(iser_id)		MMIO32(NVIC_BASE + 0x00 + \
						((iser_id) * 4))

/* NVIC_BASE + 0x020 (0xE000 E120 - 0xE000 E17F): Reserved */

/* ICER: Interrupt Clear Enable Registers */
/* Note: 8 32bit Registers */
/* Note: Single register on CM0 */
#define NVIC_ICER(icer_id)		MMIO32(NVIC_BASE + 0x80 + \
						((icer_id) * 4))

/* NVIC_BASE + 0x0A0 (0xE000 E1A0 - 0xE000 E1FF): Reserved */

/* ISPR: Interrupt Set Pending Registers */
/* Note: 8 32bit Registers */
/* Note: Single register on CM0 */
#define NVIC_ISPR(ispr_id)		MMIO32(NVIC_BASE + 0x100 + \
						((ispr_id) * 4))

/* NVIC_BASE + 0x120 (0xE000 E220 - 0xE000 E27F): Reserved */

/* ICPR: Interrupt Clear Pending Registers */
/* Note: 8 32bit Registers */
/* Note: Single register on CM0 */
#define NVIC_ICPR(icpr_id)		MMIO32(NVIC_BASE + 0x180 + \
						((icpr_id) * 4))

/* NVIC_BASE + 0x1A0 (0xE000 E2A0 - 0xE00 E2FF): Reserved */

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* IABR: Interrupt Active Bit Register */
/* Note: 8 32bit Registers */
#define NVIC_IABR(iabr_id)		MMIO32(NVIC_BASE + 0x200 + \
						((iabr_id) * 4))
#endif

/* NVIC_BASE + 0x220 (0xE000 E320 - 0xE000 E3FF): Reserved */

/* IPR: Interrupt Priority Registers */
/* Note: 240 8bit Registers */
/* Note: 32 8bit Registers on CM0 */
#define NVIC_IPR(ipr_id)		MMIO8(NVIC_BASE + 0x300 + \
						(ipr_id))

#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* STIR: Software Trigger Interrupt Register */
#define NVIC_STIR			MMIO32(STIR_BASE)
#endif
