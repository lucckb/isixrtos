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
#include <complex>
#include <foundation/dsp/fft.hpp>

	using ifft_t = short;
	using rfft_t  = double;

namespace {
//Configuration namespace
namespace cfg {
	//! Maximum Nth points used in tests
	constexpr auto fft_max = 16384;
	//!Floating point match error
	constexpr auto max_shifted_resp = 1E-12;
	//! Integer maximum error
	constexpr auto max_shifted_resp_int = 3;
	//! Symetry check max error for floating point
	constexpr auto symetry_err = 0.01;
	constexpr auto symetery_err_int = 256;

	//! Signal to noise ratio compare
	constexpr auto snr_err = 200.0;
	constexpr auto snr_err_int = 42;

	//! Compare two ffts err
	constexpr auto fft_res_cmp_err = 0.0001;
	constexpr auto fft_res_cmp_err_int = 20;

}
}
