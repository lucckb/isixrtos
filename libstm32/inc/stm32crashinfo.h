/*
 * stm32crashinfo.h
 *
 *  Created on: 21-11-2012
 *      Author: lucck
 */

#ifndef STM32CRASHINFO_H_
#define STM32CRASHINFO_H_

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


#define _cm3_hard_hault_entry_fn(hfault_fn) do {\
	unsigned long *sp; \
	enum crash_mode cmode; \
	asm( \
		"TST LR, #4\n" \
		"ITTEE EQ\n" \
	    "MRSEQ %[stackptr], MSP\n" \
		"MOVEQ %[crashm],%[tsystem]\n" \
		"MRSNE %[stackptr], PSP\n" \
		"MOVNE %[crashm],%[tuser]\n" \
			: [stackptr] "=r"(sp), [crashm] "=r"(cmode): \
			  [tuser]"I"(CRASH_TYPE_USER),[tsystem]"I"(CRASH_TYPE_SYSTEM)); \
			 hfault_fn( cmode, sp ); \
   } while(0)

#define cm3_hard_hault_regs_dump() _cm3_hard_hault_entry_fn(cortex_cm3_print_core_regs)

#ifdef __cplusplus
}
#endif

#endif /* STM32CRASHINFO_H_ */

