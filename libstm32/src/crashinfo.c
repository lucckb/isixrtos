/*
 * crashinfo.c
 *
 *  Created on: 21-11-2012
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <foundation/tiny_printf.h>
#include <stm32system.h>
#include "stm32crashinfo.h"

/* ------------------------------------------------------------------ */
//Cortex CM3 print core regs
void cortex_cm3_print_core_regs(enum crash_mode crash_type, unsigned long * SP)
{
	//Disable interrupt
	irq_disable();
	//Initialize usart simple no interrupt
	tiny_printf("\r\n\r\n ^^^^^^^^^^ CPU Crashed in [%s] mode!!! ARMv7m core regs: ^^^^^^^^^\r\n",
			crash_type==CRASH_TYPE_USER?"USER":"SYSTEM" );
	tiny_printf("[R0=%08lx]\t[R1=%08lx]\t[R2=%08lx]\t[R3=%08lx]\r\n", SP[0],SP[1],SP[2],SP[3]);
	tiny_printf("[R12=%08lx]\t[LR=%08lx]\t[PC=%08lx]\t[PSR=%08lx]\r\n",SP[4],SP[5],SP[6],SP[7]);
	const unsigned long rBFAR = (*((volatile unsigned long *)(0xE000ED38)));
	const unsigned long rCFSR = (*((volatile unsigned long *)(0xE000ED28)));
	const unsigned long rHFSR = (*((volatile unsigned long *)(0xE000ED2C)));
	const unsigned long rDFSR = (*((volatile unsigned long *)(0xE000ED30)));
	const unsigned long rAFSR = (*((volatile unsigned long *)(0xE000ED3C)));
	tiny_printf("[BAFR=%08lx]\t[CFSR=%08lx]\t[HFSR=%08lx]\t[DFSR=%08lx]\r\n",rBFAR,rCFSR,rHFSR,rDFSR);
	tiny_printf("[AFSR=%08lx]\r\n", rAFSR);
	for(;;) wfi();
}
/* ------------------------------------------------------------------ */

