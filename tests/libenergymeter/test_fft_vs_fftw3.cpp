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
		void check_vs_dft( std::complex<T> const * const in,
				std::complex<T> const * const out,
				std::complex<double> * const dftout,
				std::size_t nfft )
		{
			auto inf = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*nfft);
			auto outf = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*nfft);
			std::memset( outf, 0, sizeof(fftw_complex)*nfft );
			double difr,difi;
			double errpow=0,sigpow=0;
			//Convert to fftw format
			for( std::size_t i=0; i<nfft;++i ) {
				inf[i][0] = in[i].real();
				inf[i][1] = in[i].imag();
			}
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
			PRINTF("FFTW3vsFFT nfft=%d snr = %f\n",nfft,10*log10(sigpow/errpow) );
		}



	template <typename T> void do_test( std::size_t nfft )
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
		for( std::size_t i=0; i<nfft; ++i ) {
			ASSERT_NEAR( output[i].real(), dftout[i].real(), maxerr );
			ASSERT_NEAR( output[i].imag(), dftout[i].imag(), maxerr );
		}
	}

} //unnamed NS

TEST( fft_test, double_fftw )
{
	for( auto len=32LU; len<cfg::fft_max+1; len<<=1 )
	{
		do_test<rfft_t>( len );
	}
}


TEST( fft_test, float_fftw )
{
	for( auto len=32LU; len<cfg::fft_max+1; len<<=1 )
	{
		do_test<ffft_t>( len );
	}
}



//Real signal type1
TEST( fft_test, int16_fftw )
{
	for( auto len=32UL; len<cfg::fft_max+1; len<<=1 )
	{
		do_test<ifft_t>( len );
	}
}


//Real signal type1
TEST( fft_test, int32_fftw )
{
	for( auto len=32UL; len<cfg::fft_max+1; len<<=1 )
	{
		do_test<lfft_t>( len );
	}
}

