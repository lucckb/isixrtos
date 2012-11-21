/*
 * stm32crashinfo.h
 *
 *  Created on: 21-11-2012
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef STM32CRASHINFO_H_
#define STM32CRASHINFO_H_
/* ------------------------------------------------------------------ */
#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
enum crash_mode
{
	CRASH_TYPE_USER=1,
	CRASH_TYPE_SYSTEM
};
/* ------------------------------------------------------------------ */
//Cortex CM3 print core regs
void cortex_cm3_print_core_regs(enum crash_mode crash_type, unsigned long * SP);
/* ------------------------------------------------------------------ */
#define cm3_hard_hault_regs_dump() do {\
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
	 cortex_cm3_print_core_regs( cmode, sp ); \
   } while(0)

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
}
#endif
/* ------------------------------------------------------------------ */
#endif /* STM32CRASHINFO_H_ */
/* ------------------------------------------------------------------ */
