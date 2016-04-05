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

namespace {
	//!Integer max error
	constexpr auto maxerr = 1E-12;
	constexpr auto imaxerr = 0.0005;
	namespace num {
		constexpr auto imaxerr =  std::numeric_limits<short>::max() * (::imaxerr);
	}
}

// FFT basic test
TEST( fft_test, real_impulse_response ) 
{
	for( auto len=16UL; len<16384; len<<=1 ) 
	{
		const int m  = std::log2( len );
		std::complex<double> input[len] { {1.0,0.0} };
		double reinput[len] { 1.0 };
		std::complex<double> output[len] {};
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
	for( auto len=16UL; len<16384; len<<=1 ) 
	{
		const int m  = std::log2( len );
		std::complex<double> input[len] { {0.0,0.0},{1.0,0.0} };
		double reinput[len] { 0.0, 1.0 };
		std::complex<double> output[len] {};
		// fft complex test
		dsp::refft::fft_complex( output, input, m );
		for( const auto i : output ) {
			ASSERT_NEAR( std::abs(i), 1.0, maxerr );
		}
		//! fft real test
		for( auto& i : output ) i=0;
		dsp::refft::fft_real( output, reinput, m );
		for( const auto i : output ) {
			ASSERT_NEAR( std::abs(i), 1.0, maxerr );
		}
	}
}

// FFT basic test
TEST( fft_test, int_impulse_response ) 
{
	for( auto len=16UL; len<16384; len<<=1 ) 
	{
		const int m  = std::log2( len );
		std::complex<short> input[len] { {std::numeric_limits<short>::max(),0} };
		short reinput[len] { std::numeric_limits<short>::max() };
		std::complex<short> output[len] {};
		// fft complex test
		dsp::refft::fft_complex( output, input, m );
		//! Impulse response leak over whole spectrum and value is not scalled
		const auto expected  =  std::numeric_limits<short>::max()/len;
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

//Todo tweak to better accuracy
// FFT basic test
TEST( fft_test, int_shifted_impulse_response ) 
{
	for( auto len=16UL; len<16384; len<<=1 ) 
	{
		const int m  = std::log2( len );
		std::complex<short> input[len] { {0,0},{std::numeric_limits<short>::max(),0} };
		short reinput[len] { 0, std::numeric_limits<short>::max() };
		std::complex<short> output[len] {};
		// fft complex test
		dsp::refft::fft_complex( output, input, m );
		const auto expected  =  std::numeric_limits<short>::max()/len;
		for( const auto i : output ) {
			const auto ii = std::complex<double>(i.real(), i.imag() );
			ASSERT_NEAR( std::abs(ii), expected, num::imaxerr );
		}
		//! fft real test
		for( auto& i : output ) i=0;
		dsp::refft::fft_real( output, reinput, m );
		for( const auto i : output ) {
			const auto ii = std::complex<double>(i.real(), i.imag() );
			ASSERT_NEAR( std::abs(ii), expected, num::imaxerr );
		}
	}
}
