/*
 * =====================================================================================
 *
 *       Filename:  dbprintf.h
 *
 *    Description:  Fake dbprintf
 *
 *        Version:  1.0
 *        Created:  08.03.2014 18:32:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <cstdio>

#pragma GCC system_header

#define dbprintf(...) do { \
	std::printf(__VA_ARGS__); \
	std::puts(""); \
} while(0)

#define dbg_warn dbprintf
#define dbg_err dbprintf
#define dbg_debug dbprintf
#define dbg_info dbprintf
