/*
 * =====================================================================================
 *
 *       Filename:  isr_bootvect.c
 *
 *    Description:  Interrupt boot core vectors
 *
 *        Version:  1.0
 *        Created:  24.02.2016 23:14:48
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

extern unsigned long _estack;	   /* init value for the stack pointer. defined in linker script */

#define ISR_VECTOR( handler_name ) void handler_name(void) \
	__attribute__ ((interrupt, weak, alias("unused_vector")))
#define ISR_VECTOR_FORCED( handler_name )  \
	void handler_name(void) __attribute__ ((interrupt))


extern void _mcu_reset_handler_(void) __attribute__((interrupt,noreturn));
static void unused_vector(void) __attribute__((interrupt,noreturn));
static void unused_vector() { while(1); }
typedef void(*vect_fun_t)(void);


#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
#include "vectors/f2_4v.h"
#elif defined(STM32MCU_MAJOR_TYPE_F3)
#include "vectors/f3xx.h"
#elif defined(STM32MCU_MAJOR_TYPE_F37)
#include "vectors/f37x.h"
#elif defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
#include "vectors/f100.h"
#elif defined(STM32MCU_MAJOR_TYPE_F7)
#include "vectors/f76x.h"
#elif defined(STM32MCU_MAJOR_TYPE_F3)
#if defined(STM32F334x8)
#include "vectors/f33x.h"
#else
#error Unknown F33 MCU subtype
#endif
#else /*  !(defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || (defined STM32F10X_HD_VL)) */
#include "vectors/f101_3.h"
#endif

//		EXTI pending register for route interrupt
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
#define EXTI_PR (*((volatile unsigned long*)(0x40000000 + 0x00010000 + 0x3C00 + 0x14)))
#else
#define EXTI_PR (*((volatile unsigned long*)(0x40010400+0x14)))
#endif
#define EXTI_PEND( flags, inp ) ((flags) & (1<<(inp)))


/* emulated exti handlers demuxed in CRT0 */
ISR_VECTOR(exti5_isr_vector);
ISR_VECTOR(exti6_isr_vector);
ISR_VECTOR(exti7_isr_vector);
ISR_VECTOR(exti8_isr_vector);
ISR_VECTOR(exti9_isr_vector);
ISR_VECTOR(exti10_isr_vector);
ISR_VECTOR(exti11_isr_vector);
ISR_VECTOR(exti12_isr_vector);
ISR_VECTOR(exti13_isr_vector);
ISR_VECTOR(exti14_isr_vector);
ISR_VECTOR(exti15_isr_vector);

void exti9_5_isr_vector(void)
{
	unsigned long flags = EXTI_PR;
	if( EXTI_PEND(flags, 5) )
	{
		exti5_isr_vector();
	}
	if( EXTI_PEND(flags, 6) )
	{
		exti6_isr_vector();
	}
	if( EXTI_PEND(flags, 7) )
	{
		exti7_isr_vector();
	}
	if( EXTI_PEND(flags, 8) )
	{
		exti8_isr_vector();
	}
	if( EXTI_PEND(flags, 9) )
	{
		exti9_isr_vector();
	}
}

void exti15_10_isr_vector(void)
{
	unsigned long flags = EXTI_PR;
	if( EXTI_PEND(flags, 10) )
	{
		exti10_isr_vector();
	}
	if( EXTI_PEND(flags, 11) )
	{
		exti11_isr_vector();
	}
	if( EXTI_PEND(flags, 12) )
	{
		exti12_isr_vector();
	}
	if( EXTI_PEND(flags, 13) )
	{
		exti13_isr_vector();
	}
	if( EXTI_PEND(flags, 14) )
	{
		exti14_isr_vector();
	}
	if( EXTI_PEND(flags, 15) )
	{
		exti15_isr_vector();
	}
}
#undef EXTI_PR
#undef EXTI_PEND




#undef ISR_VECTOR
#undef ISR_VECTOR_FORCED
