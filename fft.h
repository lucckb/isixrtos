#ifndef _FFT_H_
#define _FFT_H_
/*-----------------------------------------------------------*/
#include <cstddef>
#include <cmath>
#include <limits>
#include <type_traits>
#include <cstring>
#include <complex>
#include "array_sinus.hpp"

/*-----------------------------------------------------------*/
namespace dsp {
namespace refft {
/*-----------------------------------------------------------*/
namespace _internal
{
		//Generate SIN cos array in C99 lang
		template<unsigned... Is> struct seq{};
		template<unsigned N, unsigned... Is>
		struct gen_seq : gen_seq<N-1, N-1, Is...>{};
		template<unsigned... Is>
		struct gen_seq<0, Is...> : seq<Is...>{};

	    
		template <typename T, size_t S=17>
	    class fft_sin_cos
	    {
	    public:
    		constexpr std::complex<T> operator()( const size_t m )
	    	{
		        return sine_array[m];
		    }
    		std::complex<T> sine_array[S];
	    };
		
		template <typename T>
		constexpr std::complex<T> get_mfft_tab(const size_t idx)
		{
		    return 
		    {
				T(  std::cos( (4.0 * std::atan(1.0)) / double(1<<idx) )  ),
			    T(  -std::sin( (4.0 * std::atan(1.0)) / double(1<<idx) ) )
			};
		}
		template<typename T, size_t S, unsigned... Is>
		constexpr fft_sin_cos<T,S> create_fft_sin_cos( seq<Is...>)
		{
			return {{ get_mfft_tab<T>(Is)... }};
		}

		template <typename T, size_t S>
		constexpr fft_sin_cos<T,S> create_fft_sin_cos(  )
		{
		    return create_fft_sin_cos<T,S>(gen_seq<S>{});
		}
		template<typename T, size_t S = 17>
		inline std::complex<T>  fft_sincos_get( const size_t idx )
		{
			static constexpr fft_sin_cos<T,S> sin_tab = create_fft_sin_cos<T,S>( );
			return ( sin_tab(idx) );
		}
		//Bit reverseW
		#ifndef __arm__
		//Generic bitrev version
		inline unsigned bitrev( unsigned v, int m )
		{
			// swap odd and even bits
			v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
			// swap consecutive pairs
			v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
			// swap nibbles ... 
			v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
			// swap bytes
			v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
			// swap 2-byte long pairs
			v = ( v >> 16             ) | ( v               << 16);
			return v >> (32 - m);
		}
		#else
		//TODO: Cortex M3 biteverse use RBIT AND ASR in CM3
		inline unsigned bitrev( unsigned v, int m )
		{
			
		}
		#error Bit reverse not implemented yet
		#endif
		//Constant class definition
		template<typename T, typename K> class scaledV
		{
		public:
			constexpr scaledV( double re, double im )
			: m_v(
					re * ( ( std::is_floating_point<T>::value)?(1.0):(std::numeric_limits<K>::max())),
					im * ( ( std::is_floating_point<T>::value)?(1.0):(std::numeric_limits<K>::max()))
				  )
			{
			}
		constexpr operator std::complex<T>()
		{
			return m_v;
		}
		private:
			std::complex<T> m_v;
		};
		//Multiply integer
		template<typename T, typename K>
		typename std::enable_if<std::is_integral<T>::value, T>::type
		inline impy( T x, T y )
		{
			static constexpr T scale = std::numeric_limits<K>::max();
			return (x * y)/scale;
		}
		//Multiply floating
		template<typename T, typename K>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type
		inline impy( T x, T y )
		{
			return (x * y);
		}
		//Scale added for floating point arithmetic
		template<class T>
		typename std::enable_if<std::is_floating_point<T>::value, std::complex<T>>::type
		    inline fft_scale(std::complex<T> t )
		{
		    return t;
		}
		template<class T>
		typename std::enable_if<std::is_integral<T>::value, std::complex<T>>::type
			inline fft_scale(std::complex<T> v )
		{
			return std::complex<T> (v.real() >> 1, v.imag() >> 1);
		}
		//Scale added for floating point arithmetic
		template<class T>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type
			inline fft_scale( T t )
		{
			return t;
		}
		template<class T>
		typename std::enable_if<std::is_integral<T>::value, T>::type
			inline fft_scale( T v )
		{
			return v>>1;
		}

		//FFT STAGE FLOAT VERSION
		template<typename T, typename K>
		typename std::enable_if<std::is_floating_point<T>::value, std::complex<T>*>::type
		inline fft_stage_3_n( std::complex<T> *x, int l, int m )
		{
			const int le = 1 << l;
			const int le2 = le >> 1;
			const int n = 1 << m;
			std::complex<T> u( 1.0, 0.0 );
			const std::complex<T> s (fft_sincos_get<T>(l-1));
			//Kazde widmo
			for(int j=0;j<le2;j++)
			{
				//Motylek
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
		//Internal SINUS integer version
		template <typename T, typename K>
		inline std::complex<T> fft_sin_cos_int( int le2, int i )
		{
			constexpr auto SINSIZE = 512;
			constexpr int CPI = dsp::integer::trig::sin_arg_max<K,SINSIZE>();
			const int angle = (CPI/2*i)/le2;
			return {
				  T((dsp::integer::trig::sin<K,SINSIZE>(CPI/4+angle) + dsp::integer::trig::sin<K,SINSIZE>(CPI/4+angle+1))/2),
			      T(-(dsp::integer::trig::sin<K,SINSIZE>(angle) + dsp::integer::trig::sin<K,SINSIZE>(angle+1))/2)
			};
		}
		//FFT stage INTEGER VERSION
		template<typename T, typename K>
		typename std::enable_if<std::is_integral<T>::value, std::complex<T>*>::type
		inline fft_stage_3_n( std::complex<T> *x, int l, int m )
		{
			const int le = 1 << l;
			const int le2 = le >> 1;
			const int n = 1 << m;
			//Kazde widmo
			for(int j=0;j<le2;j++)
			{
				//const std::complex<T> s = scaledV<T,K>( std::cos( (M_PI*j)/double(le2) ), -std::sin( (M_PI*j)/double(le2)) );
				const std::complex<T> s( fft_sin_cos_int<T,K>(le2, j) );
				//Motylek
				for(int i=j;i<n;i+=le)
				{
					const int ip = i + le2;
					const std::complex<T> t(
						impy<T,K>(x[ip].real() , fft_scale(s.real())) - impy<T,K>( x[ip].imag() , fft_scale(s.imag()) ),
						impy<T,K>(x[ip].imag() , fft_scale(s.real())) + impy<T,K>( x[ip].real() , fft_scale(s.imag()) )
					);
					x[ip] = fft_scale(x[i]) - t;
					x[i] =  fft_scale(x[i]) + t;
				}
			}
			return x;
		}
}	//Internal namespace end

/*-----------------------------------------------------------*/
//Calculate complex fft
template<typename T, typename K=short>
void fft_complex( std::complex<T> *x, const std::complex<T> * const in, int m )
{
	using namespace _internal;
	const int n = 1 << m;
	//Odwracanie bitowe
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
		fft_stage_3_n<T,K>( x, l, m );
	}
}
/*-----------------------------------------------------------*/
// Real fft
// Input: 2^m complex points in time domain
// Output: (2^m)*2 complex points in frequency domain
template<typename T, typename K=short>
void fft_real( std::complex<T> *xc, const T * const x, int m )
{
	 using namespace _internal;
	 //Rozklad parzysto-nieparzysty w dziedzinie czasu
	 fft_complex<T,K>( xc, reinterpret_cast<const std::complex<T>* const>(x), m-1 );
	 //Rozklad parzysto nieparzysty w dziedzinie czestotliwosci
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
	 //Ostatni stopien fft (trzeba powtorzyc)
	 fft_stage_3_n<T,K>( xc, m, m );
}
/*-----------------------------------------------------------*/
}}
/*-----------------------------------------------------------*/
#endif
