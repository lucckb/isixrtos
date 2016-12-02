#pragma once

#include <cstddef>
#include <cmath>
#include <limits>
#include <type_traits>
#include <cstring>
#include <complex>
#include "array_sinus.hpp"
#include "_fft_helper_internal.hpp"


namespace dsp {
namespace refft {

namespace _internal {
namespace {
		//FFT STAGE FLOAT VERSION
		template<typename T>
		typename std::enable_if<std::is_floating_point<T>::value, std::complex<T>*>::type
		inline fft_stage_3_n( std::complex<T> *x, int l, int m )
		{
			const int le = 1 << l;
			const int le2 = le >> 1;
			const int n = 1 << m;
			std::complex<T> u( 1.0, 0.0 );
			const std::complex<T> s (fft_sincos_get<T>(l-1));
			//Each spectrum
			for(int j=0;j<le2;j++)
			{
				//Butterfly
				for(int i=j;i<n;i+=le)
				{
					const int ip = i + le2;
					const std::complex<T> t(
						x[ip].real() * u.real() -  x[ip].imag() * u.imag(),
						x[ip].imag() * u.real() +  x[ip].real() * u.imag()
					);
					x[ip] = x[i] - t;
					x[i] =  x[i] + t;
				}
				u = {
						u.real() * s.real() - u.imag() * s.imag(),
						u.real() * s.imag() + u.imag() * s.real()
				};
			}
			return x;
		}

		//FFT stage INTEGER VERSION
		template<typename T>
		typename std::enable_if<std::is_integral<T>::value, std::complex<T>*>::type
		inline fft_stage_3_n( std::complex<T> *x, int l, int m )
		{
			const int le = 1 << l;
			const int le2 = le >> 1;
			const int n = 1 << m;
			//Kazde widmo
			for(int j=0;j<le2;j++)
			{
				const std::complex<T> s( fft_sin_cos_int<T>(le2, j) );
				//Butterfly
				for(int i=j;i<n;i+=le)
				{
					const int ip = i + le2;
					const std::complex<T> t(
						impy<T>(x[ip].real() , fft_scale(s.real())) -
							impy<T>( x[ip].imag() , fft_scale(s.imag()) ),
						impy<T>(x[ip].imag() , fft_scale(s.real())) +
							impy<T>( x[ip].real() , fft_scale(s.imag()) )
					);
					x[ip] = fft_scale(x[i]) - t;
					x[i] =  fft_scale(x[i]) + t;
				}
			}
			return x;
		}
}}	//Internal namespace end


//Calculate complex fft
template<typename T>
void fft_complex( std::complex<T>* x, const std::complex<T>* const in, int m )
{
	using namespace _internal;
	const int n = 1 << m;
	//Bit reverse
	if( x != in ) std::memcpy( x, in, n*sizeof(std::complex<T>) );
	for (int s=0,i=0;s<n-1;s++)
	{
		i = bitrev( s, m );
		if (s<i)
		{
			const std::complex<T> t( x[i] );
			x[i] = x[s];
			x[s] = t;
		}
	}
	// First pass
	for (int i=0;i<n;i+=2)
	{
		const std::complex<T> t(x[i]);
		x[i] = fft_scale(t) + fft_scale(x[i+1]);
		x[i+1] = fft_scale(t) - fft_scale(x[i+1]);
	}
	// Second pass
	for (int i=0;i<n;i+=4)
	{
		std::complex<T> t = fft_scale(x[i]);
		x[i]   = t + fft_scale(x[i+2]);
		x[i+2] = t - fft_scale(x[i+2]);
		t = fft_scale(x[i+1]);
		const std::complex<T> z ( fft_scale(x[i+3]) );
		x[i+1].real(  t.real() + z.imag() );
		x[i+1].imag(  t.imag() - z.real() );
		x[i+3].real(  t.real() - z.imag()  );
		x[i+3].imag(  t.imag() + z.real() );
	}
	//Next passes
	for(int l=3;l<=m;l++)
	{
		fft_stage_3_n<T>( x, l, m );
	}
}

// Real fft
// Input: 2^m real points in time domain
// Output: (2^m)*2 complex points in frequency domain
template<typename T>
void fft_real( std::complex<T> *xc, const T * const x, int m )
{
	 using namespace _internal;
	 //Odd even extraction in the time domain
	 fft_complex<T>( xc, reinterpret_cast<const std::complex<T>* const>(x), m-1 );
	 //Odd even extraction in the frequency domain
	 for(int i=1;i<((1<<m)/4);i++)
	 {
	    const int im = (1<<m)/2 - i;
	    const int ip2 = i + (1<<m)/2;
	    const int ipm = im + (1<<m)/2;
	    xc[ip2].real( (xc[i].imag() + xc[im].imag())/2);
	    xc[ipm].real( xc[ip2].real() );
	    xc[ip2].imag( -(xc[i].real() - xc[im].real())/2 );
	    xc[ipm].imag( -xc[ip2].imag() );
	    xc[i].real( (xc[i].real() + xc[im].real())/2 );
	    xc[im].real( xc[i].real() );
	    xc[i].imag( (xc[i].imag() -xc[im].imag())/2 );
	    xc[im].imag( -xc[i].imag() );
	 }
	 xc[((1<<m)*3)/4].real( xc[(1<<m)/4].imag() );
	 xc[(1<<m)/2].real( xc[0].imag() );
	 xc[((1<<m)*3)/4].imag( 0 );
	 xc[(1<<m)/2].imag( 0 );
	 xc[(1<<m)/4].imag( 0 );
	 xc[0].imag( 0 );
	 //Last fft butterfly ( need to be recalculated again )
	 fft_stage_3_n<T>( xc, m, m );
}

}}

