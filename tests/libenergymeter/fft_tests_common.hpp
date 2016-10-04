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
	using ffft_t = float;
	using lfft_t = int32_t;

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
			static constexpr auto symetry_err = 32;
			//! Signal to noise ratio tolerance
			static constexpr auto snr_err = 42;
			//! Compare two ffts err
			static constexpr auto fft_res_cmp_err = 9;
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
		//Float precision floating point
		template<> struct ans<ffft_t> {
			//Maximum difference for shifted_impulse_response
			static constexpr auto max_shifted_resp = 0.01;
			//Tone test fft real symetry error check
			static constexpr auto symetry_err = 5;
			//! Signal to noise ratio tolerance
			static constexpr auto snr_err = 90;
			//! Compare two ffts err
			static constexpr auto fft_res_cmp_err = 0.02;
		};
		//! Type int32 integer aritmetics
		template<> struct ans<lfft_t> {
			//Maximum difference for shifted_impulse_response
			static constexpr auto max_shifted_resp = 3;
			//Tone test fft real symetry error check
			static constexpr auto symetry_err = 32;
			//! Signal to noise ratio tolerance
			static constexpr auto snr_err = 130;
			//! Compare two ffts err
			static constexpr auto fft_res_cmp_err = 8;
		};
}
		template<typename T> T get_rand()
		{
			if( std::is_integral<T>() ) {
				constexpr T max = std::numeric_limits<T>::max();
				return (std::rand()%(long(max)*2) - long(max) );
			} else {
				return 2.0 * double(std::rand())/double(RAND_MAX) - 1.0;
			}
		}
}

