/*
 * =====================================================================================
 *
 *       Filename:  types.h
 *
 *    Description:  Private isix types
 *
 *        Version:  1.0
 *        Created:  06.04.2015 22:34:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <isix/types.h>

//! Message type
typedef int osmsg_t;

enum osobject_type_e {
	osobject_type_semaphore = 1,
	osobject_type_fifo = 2
};
//OS object type
typedef uint8_t osobject_type_t; 
