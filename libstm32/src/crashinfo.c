/*
 * crashinfo.c
 *
 *  Created on: 21-11-2012
 *      Author: lucck
 */

#include <foundation/sys/tiny_printf.h>
#include <stm32system.h>
#include "stm32crashinfo.h"


//! Access to the registry 
#define reg32(addr) *((volatile unsigned long*)(addr))
#define reg8(addr) *((volatile unsigned char*)(addr))
#define reg16(addr) *((volatile unsigned short*)(addr))

#define MMSR 0xE000ED28	//! Mem manage fault status register Byte
#define BFSR 0xE000ED29	//! Bus fault status register Byte
#define UFSR 0xE000ED2A //! Usage fault status register HalfWord
#define HFSR 0xE000ED2C //! Hard fault status register Word
#define DFSR 0xE000ED30 //! Debug fault status register Word
#define AFSR 0xE000ED3C //! Auxilary fault status register Word
#define MMAR 0xE000ED34	//! Mem manage fault address register Word
#define BFAR 0xE000ED38 //! Bus fault address register Word


#define HFSR_CLEAR ((1U<<1)|(1U<<30)|(1U<<31))
#define MMSR_BFSR_STACK_ERR_MASK ((1U<<4)|(1U<<3))
#define BFSR_CLEAR 0x9f
#define MMSR_CLEAR 0x9b
#define UFSR_CLEAR 0x030f
#define HSFR_BFORCED (1U<<30)
#define AR_VALID (1U<<7)


//! Register descrition
struct reg_desc {
	const char * const desc;
	unsigned long mask;
	unsigned char shift;
};

//! Program status register bits
static const struct reg_desc psr_bits[] = 
{
	{ "EXCNO", 0xff , 0 },
	{ "ICI10", 0xfc00, 10 },
	{ "T", 1<<24, 24 },
	{ "ICI25", 0x6000000, 25 },
	{ "Q", 1<<27, 27 },
	{ "V", 1<<28, 28 },
	{ "C", 1<<29, 29 },
	{ "Z", 1<<30, 30 },
	{ "N", 1<<31, 31},
	{ NULL, 0, 0 }
};
	
//HARD fault status register bits
static const struct reg_desc hsfr_bits[] =
{
 	{ "DEBUGEVT", 1U<<31, 31 },
 	{ "FORCED",   1U<<30, 30 },
 	{ "VECTBL",   1U<<1, 1 },
	{ NULL, 0, 0 }
};

//BUS fault status register bits
static const struct reg_desc bfsr_bits[] = 
{
	{ "BFARVALID", 		1U<<7, 7 },
	{ "STKERR", 		1U<<4, 4 },
	{ "UNSTKERR", 		1U<<3, 3 },
	{ "IMPREISERR", 	1U<<2, 2 },
	{ "PRECISERR", 		1U<<1, 1 },
	{ "IBUSERR", 		1U<<0, 0 },
	{ NULL, 0, 0 }
};

//! Memory management fault register bits
static const struct reg_desc mmsr_bits[] = 
{
	{ "MMARVALID", 		1U<<7, 7 },
	{ "STKERR", 		1U<<4, 4 },
	{ "UNSTKERR", 		1U<<3, 3 },
	{ "DACCVIOL", 		1U<<1, 1 },
	{ "IACCVIOL", 		1U<<0, 0 },
	{ NULL, 0, 0 }
};

//! Usage fault register trap
static const struct reg_desc ufsr_bits[] = 
{
	{ "DIVBYZERO", 		1U<<9, 9 },
	{ "UNALIGNED", 		1U<<8, 8 },
	{ "NOCP", 			1U<<3, 3 },
	{ "INVPC", 			1U<<2, 2 },
	{ "INVSTATE", 		1U<<1, 1 },
	{ "UNDEFINSTR", 	1U<<0, 0 },
	{ NULL, 0, 0 }
};

//! Print bits regs according to the table number
static void print_bits( const struct reg_desc *table, unsigned long bits )
{
	short pos = 0;
	tiny_printf("\t");
	for( const struct reg_desc* r = table; r->mask; ++r ) 
	{
		pos +=  tiny_printf( "%s:%lu ", r->desc, (bits&r->mask)>>r->shift );
		if( pos > 75 ) {
			tiny_printf("\t\r\n");
			pos = 0;
		}
	}
	if( pos != 0 ) {
		tiny_printf("\t\r\n");
	}
}

#if !CONFIG_ISIX_WITHOUT_KERNEL
void* isix_task_self(void);
#endif



//Cortex CM3 print core regs
void cortex_cm3_print_core_regs(enum crash_mode crash_type, unsigned long * SP)
{
	//Disable interrupt
	irq_disable();
	//Initialize usart simple no interrupt
	tiny_printf("\r\n\r\nISIX panic! Exception in [%s] mode.\r\n",
			crash_type==CRASH_TYPE_USER?"USER":"SYSTEM" );
#if !CONFIG_ISIX_WITHOUT_KERNEL
	tiny_printf("Last executed task TCB is %p\r\n", isix_task_self() );
#endif
	tiny_printf("CPU core regs: \r\n");
	tiny_printf("\t[R0=%08lx]\t[R1=%08lx]\t[R2=%08lx]\t[R3=%08lx]\r\n",
			SP[stk_r0],SP[stk_r1],SP[stk_r2],SP[stk_r3]);
	tiny_printf("\t[R12=%08lx]\t[LR=%08lx]\t[PC=%08lx]\r\n",
			SP[stk_r12],SP[stk_lr],SP[stk_pc]);

	tiny_printf("CPSR flags:\r\n");
	print_bits( psr_bits, SP[stk_psr] );
	tiny_printf("HFSR Hard Fault Status Register bits:\r\n");
	print_bits( hsfr_bits, reg32(HFSR) );
	bool stk_err = false;
	if( reg32(HFSR) & HSFR_BFORCED )
	{
		//Try to print BUS fault ADDRESS register
		if( reg8(BFSR) )
		{
			unsigned long ar = reg32(BFAR);
			tiny_printf("Bus Fault exception occured. BFSR status:\r\n");
			print_bits( bfsr_bits, reg8(BFSR) );
			if( reg8(BFSR) & AR_VALID ) {
				tiny_printf("\tBus Fault address BFAR: %08lx\r\n", ar );
			}
			stk_err = reg8(BFSR)&MMSR_BFSR_STACK_ERR_MASK;
			reg8(BFSR) = BFSR_CLEAR;
		}
		//Memory management fault handling
		if( reg8(MMSR) )
		{
			unsigned long ar = reg32(MMAR);
			tiny_printf("Memory Management Fault exception occured. MMAR status:\r\n" );
			print_bits( mmsr_bits, reg8(MMSR) );
			if( reg8(MMSR) & AR_VALID ) {
				tiny_printf("\tBus Fault address BFAR: %08lx\r\n", ar );
			}
			stk_err = reg8(MMSR)&MMSR_BFSR_STACK_ERR_MASK;
			reg8(MMSR) = MMSR_CLEAR;
		}
		// Usage fault handling
		if( reg16(UFSR) )
		{
			tiny_printf("Usage Fault exception occured. UFSR status:\r\n" );
			print_bits( ufsr_bits, reg16(UFSR) );
			reg16(UFSR) = UFSR_CLEAR;
		}
		reg32(HFSR) = HFSR_CLEAR;
	}
	//! Print stack frame only when no stacking error
	if( !stk_err )
	{
		tiny_printf("Stack dump:\r\n\t" );
		for( int i=0; i<32; ++i )
		{
			if( i%4==0 )  {
				tiny_printf( "%08lx: ", (unsigned long)&SP[stk_data+i] );
			}
			tiny_printf( "%08lx ", SP[stk_data+i] );
			if( i%4==3 ) {
				tiny_printf("\r\n\t");
			}
		}
		tiny_printf("\r\n");
	}
	for(;;) wfi();
}

