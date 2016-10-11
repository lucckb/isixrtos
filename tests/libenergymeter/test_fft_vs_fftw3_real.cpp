/*
 * =====================================================================================
 *
 *       Filename:  test_fft_vs_fftw3.cpp
 *
 *    Description:  FFT vs FFTW3
 *
 *        Version:  1.0
 *        Created:  06.04.2016 22:29:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 *
 */

#include <gtest/gtest.h>
#include <foundation/dsp/fft.hpp>
#include <limits>
#include "gprint.hpp"
#include "fft_tests_common.hpp"
#include <fftw3.h>

namespace {

	template <typename T>
		void check_vs_dft( T const * const in,
				std::complex<T> const * const out,
				std::complex<double> * const dftout,
				std::size_t nfft )
		{
			auto inf = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*nfft);
			double difr,difi;
			double errpow=0,sigpow=0;
			//Convert to fftw format
			for( std::size_t i=0; i<nfft;++i ) {
				inf[i][0] = in[i];
				inf[i][1] = 0;
			}
			auto outf = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*nfft);
			auto plan = fftw_plan_dft_1d( nfft, inf, outf, FFTW_FORWARD, FFTW_ESTIMATE );
			fftw_execute( plan );
			//Convert back again
			for( std::size_t i=0; i<nfft;++i ) {
				if( std::is_integral<T>() ){
					outf[i][0] /= nfft;
					outf[i][1] /= nfft;
				}
				dftout[i].real( outf[i][0] );
				dftout[i].imag( outf[i][1] );
				difr = outf[i][0] - out[i].real();
				difi = outf[i][1] - out[i].imag();
				errpow += difr*difr + difi*difi;
				sigpow += outf[i][0]*outf[i][0]+outf[i][1]*outf[i][1];
			}

			fftw_destroy_plan( plan );
			fftw_free(inf);
			fftw_free(outf);
			PRINTF("FFTW3vsFFT_REAL nfft=%d snr = %f\n",nfft,10*log10(sigpow/errpow) );
		}

	template <typename T> void do_test( std::size_t nfft )
	{
		constexpr double maxerr = cfg::ans<T>::fft_res_cmp_err;
		T input[nfft] {};
		std::complex<T> output[nfft] {};
		std::complex<double> dftout[nfft] {};
		const int m  = std::log2( nfft );
		for( auto& in : input ) {
			in = get_rand<T>();
		}
		dsp::refft::fft_real( output, input, m );
		check_vs_dft( input, output, dftout, nfft );
		for( std::size_t i=0; i<nfft; ++i ) {
			ASSERT_NEAR( output[i].real(), dftout[i].real(), maxerr );
			ASSERT_NEAR( output[i].imag(), dftout[i].imag(), maxerr );
		}
	}

}

TEST( fft_test, double_fftw_fft_real )
{
	for( auto len=32UL; len<cfg::fft_max+1; len<<=1 )
	{
		do_test<rfft_t>( len );
	}
}

TEST( fft_test, float_fftw_fft_real )
{
	for( auto len=32UL; len<cfg::fft_max+1; len<<=1 )
	{
		do_test<ffft_t>( len );
	}
}

TEST( fft_test, int16_fftw_fftw_real )
{
	for( auto len=32UL; len<cfg::fft_max+1; len<<=1 )
	{
		do_test<ifft_t>( len );
	}
}


TEST( fft_test, int32_fftw_fftw_real )
{
	for( auto len=32UL; len<cfg::fft_max+1; len<<=1 )
	{
		do_test<lfft_t>( len );
	}
}

TEST( fft_test, fft_inplace_test ) 
{
	using T = float;
	constexpr auto nfft = 256;
	constexpr double maxerr = cfg::ans<T>::fft_res_cmp_err;
	T input[nfft*8] {};
	std::complex<double> dftout[nfft] {};
	const int m  = std::log2( nfft );
	for( auto& in : input ) {
		in = get_rand<T>();
	}
	T input2[nfft];
	std::copy( input, input+nfft, input2 );
	auto inout = reinterpret_cast<std::complex<T>*>(input);
	dsp::refft::fft_real( inout, input, m );
	check_vs_dft( input2, inout, dftout, nfft );
	for( std::size_t i=0; i<nfft; ++i ) {
		ASSERT_NEAR( inout[i].real(), dftout[i].real(), maxerr );
		ASSERT_NEAR( inout[i].imag(), dftout[i].imag(), maxerr );
	}
}

