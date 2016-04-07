/*
 * =====================================================================================
 *
 *       Filename:  test_fft_proc.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04.04.2016 22:30:02
 *       Revision:  none
 *       Compiler:  gcc
 *
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
TEST( fft_test, real_impulse_response ) 
{
	for( auto len=16UL; len<cfg::fft_max+1; len<<=1 ) 
	{
		const int m  = std::log2( len );
		std::complex<rfft_t> input[len] { {1.0,0.0} };
		rfft_t reinput[len] { 1.0 };
		std::complex<rfft_t> output[len] {};
		// fft complex test
		dsp::refft::fft_complex( output, input, m );
		for( const auto i : output ) {
			ASSERT_DOUBLE_EQ( i.real(), 1.0 );
			ASSERT_DOUBLE_EQ( i.imag(), 0.0 );
		}
		//! fft real test
		for( auto& i : output ) i=0;
		dsp::refft::fft_real( output, reinput, m );
		for( const auto i : output ) {
			ASSERT_DOUBLE_EQ( i.real(), 1.0 );
			ASSERT_DOUBLE_EQ( i.imag(), 0.0 );
		}
	}
}



// FFT basic test
TEST( fft_test, real_shifted_impulse_response ) 
{
	for( auto len=16UL; len<cfg::fft_max+1; len<<=1 ) 
	{
		const int m  = std::log2( len );
		std::complex<rfft_t> input[len] { {0.0,0.0},{1.0,0.0} };
		rfft_t reinput[len] { 0.0, 1.0 };
		std::complex<rfft_t> output[len] {};
		// fft complex test
		dsp::refft::fft_complex( output, input, m );
		for( const auto i : output ) {
			ASSERT_NEAR( std::abs(i), 1.0, cfg::max_shifted_resp );
		}
		//! fft real test
		for( auto& i : output ) i=0;
		dsp::refft::fft_real( output, reinput, m );
		for( const auto i : output ) {
			ASSERT_NEAR( std::abs(i), 1.0, cfg::max_shifted_resp );
		}
	}
}

// FFT basic test
TEST( fft_test, int_impulse_response ) 
{
	for( auto len=16UL; len<cfg::fft_max+1; len<<=1 ) 
	{
		const int m  = std::log2( len );
		std::complex<ifft_t> input[len] { {std::numeric_limits<ifft_t>::max(),0} };
		ifft_t reinput[len] { std::numeric_limits<ifft_t>::max() };
		std::complex<ifft_t> output[len] {};
		// fft complex test
		dsp::refft::fft_complex( output, input, m );
		//! Impulse response leak over whole spectrum and value is not scalled
		const auto expected  =  std::numeric_limits<ifft_t>::max()/len;
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

//TODO tweak to better accuracy
// FFT basic test
TEST( fft_test, int_shifted_impulse_response ) 
{
	for( auto len=16UL; len<cfg::fft_max+1; len<<=1 ) 
	{
		const int m  = std::log2( len );
		std::complex<ifft_t> input[len] { {0,0},{std::numeric_limits<ifft_t>::max(),0} };
		ifft_t reinput[len] { 0, std::numeric_limits<ifft_t>::max() };
		std::complex<ifft_t> output[len] {};
		// fft complex test
		dsp::refft::fft_complex( output, input, m );
		const auto expected  =  std::numeric_limits<ifft_t>::max()/len;
		for( const auto i : output ) {
			const auto ii = std::complex<double>(i.real(), i.imag() );
			ASSERT_NEAR( std::abs(ii), expected, cfg::max_shifted_resp_int );
		}
		//! fft real test
		for( auto& i : output ) i=0;
		dsp::refft::fft_real( output, reinput, m );
		for( const auto i : output ) {
			const auto ii = std::complex<double>(i.real(), i.imag() );
			ASSERT_NEAR( std::abs(ii), expected, cfg::max_shifted_resp_int );
		}
	}
}
