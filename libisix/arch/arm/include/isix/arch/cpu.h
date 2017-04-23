/*
 * =====================================================================================
 *
 *       Filename:  cpu.h
 *
 *    Description:  CPU specific API
 *
 *        Version:  1.0
 *        Created:  10.04.2017 19:49:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once


#ifdef __cplusplus
extern "C" {
#endif



//Idle task additional
static inline __attribute__((always_inline))
void _isix_port_idle_cpu( void )
{
#ifndef PDEBUG
	asm volatile("wfi\t\n");
#else
	asm volatile("nop\t\n");
#endif
}

//!Flush the memory
static inline __attribute__((always_inline))
	void _isix_port_flush_memory( void )
{
	asm volatile(
		"dsb 0xF\t\n"
		"isb 0xF\t\n"
		:::"memory"
	);
}


#ifdef __cplusplus
}
#endif
