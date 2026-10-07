/*
 * stm32crashinfo.h
 *
 *  Created on: 21-11-2012
 *      Author: lucck
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


enum crash_mode
{
	CRASH_TYPE_USER=1,
	CRASH_TYPE_SYSTEM
};

//! Set position on stack
enum stk_regs {
	stk_r0,
	stk_r1,
	stk_r2,
	stk_r3,
	stk_r12,
	stk_lr,
	stk_pc,
	stk_psr,
	stk_data	//!User data on stack
};

//Cortex CM3 print core regs
void cortex_cm3_print_core_regs(enum crash_mode crash_type, unsigned long * SP);


//! Hard fault entry for a naked handler: asm only, selects the faulted stack and jumps to hfault_fn
#define _cm3_hard_hault_entry_fn(hfault_fn) \
	asm volatile( \
		"tst lr, #4\n" \
		"itete eq\n" \
		"mrseq r1, msp\n" \
		"mrsne r1, psp\n" \
		"moveq r0, %[tsystem]\n" \
		"movne r0, %[tuser]\n" \
		"b %c[fn]\n" \
		:: [fn] "i"(hfault_fn), \
		   [tuser] "I"(CRASH_TYPE_USER), [tsystem] "I"(CRASH_TYPE_SYSTEM))

#define cm3_hard_hault_regs_dump() _cm3_hard_hault_entry_fn(cortex_cm3_print_core_regs)

#ifdef __cplusplus
}
#endif

