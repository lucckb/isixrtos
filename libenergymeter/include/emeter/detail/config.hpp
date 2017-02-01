/*
 * =====================================================================================
 *
 *       Filename:  config.hpp
 *
 *    Description:  Emeter configuration tags
 *
 *        Version:  1.0
 *        Created:  11.10.2016 20:03:34
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include "../types.hpp"

namespace emeter {
namespace config {
namespace {
	//! Default FFT size
	constexpr auto fft_bits = 8U;
	constexpr int fft_size = 1<<fft_bits;
	constexpr auto fftbuf_size = fft_size + fft_size/2;
	//! Number of phases to measure
	constexpr auto n_phases = 3U;
	//! Sample rate
	constexpr auto sample_rate = 3200;
	//! Energy measure treshold
	constexpr measure_t min_power = 0.5;
	constexpr measure_t min_current = 0.005;

}}}

