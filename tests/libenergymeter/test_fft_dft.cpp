/*
 * =====================================================================================
 *
 *       Filename:  test_fft_dft.cpp
 *
 *    Description:  DFT vs FFT test cases
 *
 *        Version:  1.0
 *        Created:  06.04.2016 20:51:16
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

namespace {

	template <typename T>
	void check_vs_dft( std::complex<T> const * const in,
		std::complex<T> const * const out,
		std::complex<double> * const dftout, size_t nfft )
	{
		size_t bin,k;
		double errpow=0,sigpow=0;

		for (bin=0;bin<nfft;++bin) {
			double ansr = 0;
			double ansi = 0;
			double difr;
			double difi;

			for (k=0;k<nfft;++k) {
				double phase = -2*M_PI*bin*k/nfft;
				double re = std::cos(phase);
				double im = std::sin(phase);
				if( std::is_integral<T>() ) {
					re /= nfft;
					im /= nfft;
				}
				ansr += in[k].real() * re - in[k].imag() * im;
				ansi += in[k].real() * im + in[k].imag() * re;
			}
			dftout[bin].real( ansr );
			dftout[bin].imag( ansi );
			difr = ansr - out[bin].real();
			difi = ansi - out[bin].imag();
			errpow += difr*difr + difi*difi;
			sigpow += ansr*ansr+ansi*ansi;
		}
		PRINTF("FFTvsDFT nfft=%d snr = %f\n",nfft,10*log10(sigpow/errpow) );
	}

	template <typename T>
	void do_test( size_t nfft, bool print = false )
	{
		constexpr double maxerr = cfg::ans<T>::fft_res_cmp_err;
		std::complex<T> input[nfft] {};
		std::complex<T> output[nfft] {};
		std::complex<double> dftout[nfft] {};
		const int m  = std::log2( nfft );
		for( auto& in : input ) {
			in.real( get_rand<T>() );
			in.imag( get_rand<T>() );
		}
		dsp::refft::fft_complex( output, input, m );
		check_vs_dft( input, output, dftout, nfft );
		for( size_t i=0; i<nfft; ++i ) {
			ASSERT_NEAR( output[i].real(), dftout[i].real(), maxerr );
			ASSERT_NEAR( output[i].imag(), dftout[i].imag(), maxerr );
			if( print )
				PRINTF("SMPL %i\n", i );
		}
	}

}

//Real signal type1
TEST( fft_test, real_fft_vs_dft )
{
	for( auto len=64LU; len<cfg::fft_max; len<<=1 )
	{
		do_test<rfft_t>( len );
	}
}

//Real signal type1
TEST( fft_test, int16_fft_vs_dft )
{
	for( auto len=64LU; len<cfg::fft_max; len<<=1 )
	{
		do_test<ifft_t>( len );
	}
}


//Real signal type1
TEST( fft_test, int32_fft_vs_dft )
{
	for( auto len=64LU; len<cfg::fft_max; len<<=1 )
	{
		do_test<lfft_t>( len );
	}
}


//Real signal type1
TEST( fft_test, int32_fft_vs_dft_256p )
{
	do_test<lfft_t>( 256, false  );
}


//Real signal type1
TEST( fft_test, int16_fft_vs_dft_256p )
{
	do_test<ifft_t>( 256, false );
}

