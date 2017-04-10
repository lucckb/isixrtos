/*
 * =====================================================================================
 *
 *       Filename:  core.h
 *
 *    Description:  Port core specific macros
 *
 *        Version:  1.0
 *        Created:  09.04.2017 21:39:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once


//! Minimum stack depth
enum {
	ISIX_PORT_SCHED_MIN_STACK_DEPTH = 160 + 32*sizeof(long)
};



