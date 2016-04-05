/*
 * =====================================================================================
 *
 *       Filename:  fft_tests_common.hpp
 *
 *    Description:  FFT test common
 *
 *        Version:  1.0
 *        Created:  05.04.2016 21:23:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <limits>

namespace {
	constexpr auto config_fft_max = 16384;
	using ifft_t = short;
	//!Floating point match error
	constexpr auto fmaxerr = 1E-12;
	//! Integer maximum error
	constexpr auto imaxerr = 0.0005;
	//! Symetry check max error for floating point
	constexpr auto fsymetryerr = 0.0001;
	//! Symetry bean test2
	constexpr auto fsymetryerr2 = 0.01;
	namespace num {
		constexpr auto imaxerr =  std::numeric_limits<short>::max() * (::imaxerr);
	}
}
