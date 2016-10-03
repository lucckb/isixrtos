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

	using ifft_t = int16_t;
	using rfft_t  = double;

namespace {
//Configuration namespace
namespace cfg {

		//! Maximum Nth points used in tests
		constexpr auto fft_max = 16384;

		template< typename T > struct ans {
		};
		//! Type IFFFT
		template<> struct ans<ifft_t> {
			//Maximum difference for shifted_impulse_response
			static constexpr auto max_shifted_resp = 3;
			//Tone test fft real symetry error check
			static constexpr auto symetry_err = 256;
			//! Signal to noise ratio tolerance
			static constexpr auto snr_err = 42;
			//! Compare two ffts err
			static constexpr auto fft_res_cmp_err = 20;
		};
		//! Type RFFT
		template<> struct ans<rfft_t> {
			//Maximum difference for shifted_impulse_response
			static constexpr auto max_shifted_resp = 1E-12;
			//Tone test fft real symetry error check
			static constexpr auto symetry_err = 0.01;
			//! Signal to noise ratio tolerance
			static constexpr auto snr_err = 200.0;
			//! Compare two ffts err
			static constexpr auto fft_res_cmp_err = 0.0001;
		};

}

	namespace cnf {
	}

}

