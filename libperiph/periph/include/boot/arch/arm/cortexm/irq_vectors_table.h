/*
 * =====================================================================================
 *
 *       Filename:  irq_vectors_symbol.h
 *
 *    Description:  IRQ vectors symbol declaration
 *
 *        Version:  1.0
 *        Created:  19.04.2017 21:51:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

typedef void(*_vect_fun_t_)(void);
extern "C" {
	extern __attribute__ ((section(".isr_vector")))
	const _vect_fun_t_ _exceptions_vectors[];
}
