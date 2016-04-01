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
#include <cstdint>

namespace emeter {
	//! Accumulative type
	using accum_t = std::uint64_t;
	//! Sample type
	using sample_t = std::uint16_t;
	//! Simple measure type
	using measure_t = float;
}
