/*
 * =====================================================================================
 *
 *       Filename:  test_fft_proc.cpp
 *
 *    Description:  BASE test vectors for fft
 *
 *        Version:  1.0
 *        Created:  04.04.2016 22:30:02
 *       Revision:  none
 *       Compiler:  gcc
 *		 Based on test vectors from: http://www.sccon.ca/sccon/fft/fft3.htm
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gtest/gtest.h>
#include <foundation/dsp/fft.hpp>
#include <array>
#include <limits>
#include "fft_tests_common.hpp"




// FFT basic test
template <typename T> void impulse_response()
{
	const auto scale = std::is_integral<T>()?std::numeric_limits<T>::max():1.0;
	for( auto len=16UL; len<cfg::fft_max+1; len<<=1 )
	{
		const int m  = std::log2( len );
		std::complex<T> input[len] { {T(scale) ,0} };
		T reinput[len] { T(scale) };
		std::complex<T> output[len] {};
		// fft complex test
		dsp::refft::fft_complex( output, input, m );
		//! Impulse response leak over whole spectrum and value is not scalled
		const auto expected  =  std::is_integral<T>()?
			std::numeric_limits<T>::max()/len : 1.0;
		for( const auto i : output ) {
			ASSERT_EQ( i.real(), expected );
			ASSERT_EQ( i.imag(), 0 );
		}
		//! fft real test
		for( auto& i : output ) i=0;
		dsp::refft::fft_real( output, reinput, m );
		for( const auto i : output ) {
			ASSERT_EQ( i.real(), expected );
			ASSERT_EQ( i.imag(), 0 );
		}
	}
}

//! Shifted impulse response
template <typename T> void shifted_impulse_response()
{
	const auto scale = std::is_integral<T>()?std::numeric_limits<T>::max():1.0;
	for( auto len=16UL; len<cfg::fft_max+1; len<<=1 )
	{
		const int m  = std::log2( len );
		std::complex<T> input[len] { {0,0},{T(scale),0} };
		T reinput[len] { 0, T(scale) };
		std::complex<T> output[len] {};
		// fft complex test
		dsp::refft::fft_complex( output, input, m );
		const auto expected  =  std::is_integral<T>()?
			std::numeric_limits<T>::max()/len : 1.0;
		for( const auto i : output ) {
			const auto ii = std::complex<double>(i.real(), i.imag() );
			ASSERT_NEAR( std::abs(ii), expected, cfg::ans<T>::max_shifted_resp );
		}
		//! fft real test
		for( auto& i : output ) i=0;
		dsp::refft::fft_real( output, reinput, m );
		for( const auto i : output ) {
			const auto ii = std::complex<double>(i.real(), i.imag() );
			ASSERT_NEAR( std::abs(ii), expected, cfg::ans<T>::max_shifted_resp );
		}
	}
}




// FFT basic test
TEST( fft_test, real_impulse_response )
{
	impulse_response<rfft_t>();
	impulse_response<ffft_t>();
}



// FFT basic test
TEST( fft_test, real_shifted_impulse_response )
{
	shifted_impulse_response<rfft_t>();
	shifted_impulse_response<ffft_t>();
}
;

// Integer impulse response
TEST( fft_test, int_impulse_response )
{
	impulse_response<ifft_t>();
	impulse_response<lfft_t>();
}

// FFT basic test
TEST( fft_test, int_shifted_impulse_response )
{
	shifted_impulse_response<ifft_t>();
	shifted_impulse_response<lfft_t>();
}


