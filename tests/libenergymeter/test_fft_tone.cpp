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

	//Generate dual tone at selected bins
	template<typename T>
		void gen_tone( T* buf, int bin1, int bin2, size_t nfft, double max )
		{
			const double f1 = bin1*2*M_PI/nfft;
			const double f2 = bin2*2*M_PI/nfft;
			for( size_t i=0; i<nfft; ++i ) {
				buf[i] = (max/2)*std::cos( f1*i )
					+ (max/2)*std::cos( f2*i );
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

	struct min_max {
		void operator()( double snr ) {
			min = std::min( min, snr );
			max = std::max( max, snr );
		}
		void clear() {
			min = std::numeric_limits<double>::max();
			max = std::numeric_limits<double>::min();
		}
		double min { std::numeric_limits<double>::max() };
		double max { std::numeric_limits<double>::min() };
	};

	template<typename T>
		void fft_tone_real_bin_iter( int bin1, int bin2, size_t len,
				double err, double snr_err, min_max& snr )
		{
			const double max_range = std::is_integral<T>()?
					std::numeric_limits<T>::max():1.0;
			const int m  = std::log2( len );
			std::complex<T> input[len] {};
			T reinput[len] {};
			std::complex<T> output[len] {};
			std::complex<T> output2[len] {};
			//Twp tones bean at  10 and 20
			gen_tone( input, bin1, bin2, len, max_range );
			gen_tone( reinput, bin1, bin2, len, max_range );
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
			ASSERT_NEAR( reout[bin1], max_range/2, err );
			ASSERT_NEAR( reout[bin2], max_range/2, err );

			//Magnitude of real fft
			T reout2[len/2+1] {};
			fft_magnitude( reout2, output2, len );
			ASSERT_NEAR( reout2[bin1], max_range/2, err );
			ASSERT_NEAR( reout2[bin2], max_range/2, err );
			//Double SNR should be greater than 200db
			const auto snr_ = fft_snr( output, len, bin1, bin2 );
			const auto snr2_ = fft_snr( output2, len, bin1, bin2 );
			ASSERT_GT( snr_ , snr_err );
			ASSERT_GT( snr2_, snr_err );
			snr( snr_ );
			snr( snr2_ );
		}

}

template <typename T> min_max _bin_points_type1()
{
	min_max snr;
	for( auto len=64LU; len<cfg::fft_max; len<<=1 )
	{
		fft_tone_real_bin_iter<T>( 10, 22, len, cfg::ans<T>::symetry_err, cfg::ans<T>::snr_err, snr );
	}
	return snr;
}

template <typename T> min_max _bin_points_type2()
{
	min_max snr;
	constexpr auto nfft = cfg::fft_max;
	for (size_t i=0;i<nfft/2;i+= (nfft>>4)+1) {
		for (size_t j=i;j<nfft/2;j+=(nfft>>4)+7) {
			if( i!=j )
				fft_tone_real_bin_iter<T>( i, j, nfft, cfg::ans<T>::symetry_err,
					cfg::ans<T>::snr_err, snr );
		}
	}
	return snr;
}


//Real signal type1
TEST( fft_test, real_bin_points_type1 )
{
	auto snr = _bin_points_type1<rfft_t>();
	PRINTF("double#1 SNR min %f max %f\n", snr.min, snr.max  );
	snr = _bin_points_type1<ffft_t>();
	PRINTF("float#1 SNR min %f max %f\n", snr.min, snr.max  );
}

//Real signal type1
TEST( fft_test, real_bin_points_type2 )
{
	auto snr = _bin_points_type2<rfft_t>();
	PRINTF("double#2 SNR min %f max %f\n", snr.min, snr.max  );
	snr = _bin_points_type2<ffft_t>();
	PRINTF("float#2 SNR min %f max %f\n", snr.min, snr.max  );
}



//Real signal type1
TEST( fft_test, integer_bin_points_type1_int16 )
{
	auto snr = _bin_points_type1<ifft_t>();
	PRINTF("int16_t#1 SNR min %f max %f\n", snr.min, snr.max  );
}

//Real signal type1
TEST( fft_test, integer_bin_points_type1_int32 )
{
	auto snr = _bin_points_type1<lfft_t>();
	PRINTF("int32_t#1 SNR min %f max %f\n", snr.min, snr.max  );
}


//Real signal type1
TEST( fft_test, integer_bin_points_type2_int16 )
{
	auto snr = _bin_points_type2<ifft_t>();
	PRINTF("integer#2 SNR min %f max %f\n", snr.min, snr.max  );
}

//Real signal type1
TEST( fft_test, integer_bin_points_type2_int32 )
{
	auto snr = _bin_points_type2<lfft_t>();
	PRINTF("int32_t#2 SNR min %f max %f\n", snr.min, snr.max  );
}

