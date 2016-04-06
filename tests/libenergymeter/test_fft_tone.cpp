/*
 * =====================================================================================
 *
 *       Filename:  test_fft_tone.cpp
 *
 *    Description:  TONE FFT test
 *
 *        Version:  1.0
 *        Created:  05.04.2016 21:22:30
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
#include <limits>
#include "gprint.hpp"
#include "fft_tests_common.hpp"
#include <algorithm>

namespace {
	//Maximum tone value
	constexpr auto max_range = std::numeric_limits<ifft_t>::max();

	//Generate dual tone at selected bins
	template<typename T>
		void gen_tone( T* buf, int bin1, int bin2, size_t nfft ) 
		{
			const double f1 = bin1*2*M_PI/nfft;
			const double f2 = bin2*2*M_PI/nfft;
			for( size_t i=0; i<nfft; ++i ) {
				buf[i] = (max_range>>1)*std::cos( f1*i ) 
					+ (max_range>>1)*std::cos( f2*i );
			}
		}
		
		template <typename T>
		void fft_magnitude( T* output, std::complex<T> const * const input, size_t nfft )
		{
			const auto scale = std::is_integral<T>()?1.0:double(nfft);
			for( size_t i=0; i<nfft/2+1; ++i ) {
				double re = double(input[i].real())/scale;
				double im = double(input[i].imag())/scale;
				double mag = std::sqrt( re*re + im*im );
				if( i!=0 && i!= nfft/2 ) {
					//all bins exept DC and nyqist have symetric counterparts implied
					mag *= 2;
				}
				output[i] = mag;
			}
		}
		template <typename T>
		double fft_snr( std::complex<T> const * const input, size_t nfft, size_t bin1, size_t bin2 ) 
		{
			double noisepow=0, sigpow=0;
			const auto scale = std::is_integral<T>()?1.0:double(nfft);
			for( size_t i=0; i<nfft/2+1; ++i ) {
				double re = double(input[i].real())/scale;
				double im = double(input[i].imag())/scale;
				double mag2 =  re*re + im*im ;
				if( i!=0 && i!= nfft/2 ) {
					//all bins exept DC and nyqist have symetric counterparts implied
					mag2 *= 2;
				}
				/* if there is power in one of the expected bins, it is signal, otherwise noise*/
				if ( i!=bin1 && i != bin2 ) noisepow += mag2;
				else sigpow += mag2;
			}
			return 10.0*std::log10(sigpow/(noisepow+1e-50));
		}
}

struct snr_status {
	void operator()( double snr ) {
		min = std::min( min, snr );
		max = std::max( max, snr );
	}
	double min { std::numeric_limits<double>::max() };
	double max { std::numeric_limits<double>::min() };
};

template<typename T>
static void fft_tone_real_bin_iter( int bin1, int bin2, size_t len, 
		double err, double snr_err, snr_status& snr )
{
	const int m  = std::log2( len );
	std::complex<T> input[len] {};
	T reinput[len] {};
	std::complex<T> output[len] {};
	std::complex<T> output2[len] {};
	//Twp tones bean at  10 and 20
	gen_tone( input, bin1, bin2, len );
	gen_tone( reinput, bin1, bin2, len );
	// fft complex test
	dsp::refft::fft_complex( output, input, m );
	// fft real test
	dsp::refft::fft_real( output2, reinput, m );
	//Compare fft real and fft imag
	for( size_t i=0;i<len;++i ) {
		ASSERT_NEAR( output[i].real(), output2[i].real(), err );
		ASSERT_NEAR( output[i].imag(), output2[i].imag(), err );
	}
	//Check FFT symetry in the real and imag freqencies
	// Omit 0 DC and FS/2 nyquist frequency
	for( size_t i=1,j=len-1;i<len/2-1; ++i,--j ) {
		ASSERT_NEAR( output[i].real(), output[j].real(), err );
		ASSERT_NEAR( output[i].imag(), output[j].imag(), err );
	}
	for( size_t i=1,j=len-1;i<len/2-1; ++i,--j ) {
		ASSERT_NEAR( output2[i].real(), output2[j].real(), err );
		ASSERT_NEAR( output2[i].imag(), output2[j].imag(), err );
	}
	//Calculate the values and frequency beans
	T reout[len/2+1] {};
	fft_magnitude( reout, output, len );
	//Check equality of magnitude
	ASSERT_NEAR( reout[bin1], max_range>>1, err );
	ASSERT_NEAR( reout[bin2], max_range>>1, err );

	//Magnitude of real fft
	T reout2[len/2+1] {};
	fft_magnitude( reout2, output2, len );
	ASSERT_NEAR( reout2[bin1], max_range>>1, err );
	ASSERT_NEAR( reout2[bin2], max_range>>1, err );
	//Double SNR should be greater than 200db
	const auto snr_ = fft_snr( output, len, bin1, bin2 );
	const auto snr2_ = fft_snr( output2, len, bin1, bin2 );
	ASSERT_GT( snr_ , snr_err );
	ASSERT_GT( snr2_, snr_err );
	snr( snr_ );
	snr( snr2_ );
}




//Real signal type1
TEST( fft_test, real_bin_points_type1 ) 
{
	snr_status snr;
	for( auto len=64LU; len<config_fft_max; len<<=1 ) 
	{
		fft_tone_real_bin_iter<double>( 10, 22, len, fsymetryerr, snrerr, snr );
	}
	PRINTF("real#1 SNR min %f max %f\n", snr.min, snr.max  );
}


//Real signal type1
TEST( fft_test, real_bin_points_type2 ) 
{
	snr_status snr;
	constexpr auto nfft = config_fft_max;
	for (size_t i=0;i<nfft/2;i+= (nfft>>4)+1) {
		for (size_t j=i;j<nfft/2;j+=(nfft>>4)+7) {
			if( i!=j )
			fft_tone_real_bin_iter<double>( i, j, nfft, fsymetryerr2, snrerr, snr );
		}
	}
	PRINTF("real#2 SNR min %f max %f\n", snr.min, snr.max  );
}


//Real signal type1
TEST( fft_test, integer_bin_points_type1 )
{
	snr_status snr;
	for( auto len=64LU; len<config_fft_max; len<<=1 ) 
	{
		fft_tone_real_bin_iter<ifft_t>( 10, 22, len, ifsymetryerr, isnrerr, snr );
	}
	PRINTF("integer#1 SNR min %f max %f\n", snr.min, snr.max  );
}


//Real signal type1
TEST( fft_test, integer_bin_points_type2 ) 
{
	snr_status snr;
	constexpr auto nfft = config_fft_max;
	for (size_t i=0;i<nfft/2;i+= (nfft>>4)+1) {
		for (size_t j=i;j<nfft/2;j+=(nfft>>4)+7) {
			if( i!=j )
			fft_tone_real_bin_iter<ifft_t>( i, j, nfft, ifsymetryerr2, isnrerr, snr );
		}
	}
	PRINTF("integer#2 SNR min %f max %f\n", snr.min, snr.max  );
}



