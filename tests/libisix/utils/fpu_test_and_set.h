/*
 * =====================================================================================
 *
 *       Filename:  fpu_test_and_set.h
 *
 *    Description:  FPU test and set
 *
 *        Version:  1.0
 *        Created:  10.07.2017 21:02:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

namespace tests {

	namespace fpu_sp {
		//Save all fpu regs inc initial by one
		__attribute__((noinline))
		__attribute__((used))
		void fill_and_add( int start );
		// Test context checking after adding ones
		__attribute__((noinline))
		__attribute__((used))
		int fill_and_add_check( int start );
		// Fill basics regs from irq
		__attribute__((noinline))
		__attribute__((used))
		void base_regs_fill( int start );
		//Check basic regs filling from irq
		__attribute__((noinline))
		__attribute__((used))
		int base_regs_check( int start );
	}
}

