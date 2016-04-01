/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:  Open EFM types
 *
 *        Version:  1.0
 *        Created:  30.03.2016 19:54:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

namespace emeter {
	//! Accumulative type
	using accum_t = unsigned long long;
	//! Sample type
	using sample_t =  unsigned short;
	//! Simple measure type
	using measure_t = float;
}
