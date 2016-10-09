/*
 * =====================================================================================
 *
 *       Filename:  _fft_helper_internal.hpp
 *
 *    Description:  FFT helper internal class
 *
 *        Version:  1.0
 *        Created:  06.10.2016 18:41:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <cstddef>
#include <cmath>
#include <limits>
#include <type_traits>
#include <cstring>
#include <complex>
#include "array_sinus.hpp"
#include "common.hpp"

namespace dsp {
namespace refft {

namespace _internal {
namespace {


		template <typename T, std::size_t S>
	    class fft_sin_cos {
	    public:
			constexpr std::complex<T> operator()( const size_t m ) const
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
		constexpr fft_sin_cos<T,S> create_fft_sin_cos( common::seq<Is...>)
		{
			return {{ get_mfft_tab<T>(Is)... }};
		}

		template <typename T, size_t S>
		constexpr fft_sin_cos<T,S> create_fft_sin_cos(  )
		{
		    return create_fft_sin_cos<T,S>(common::gen_seq<S>{});
		}
		template<typename T>
		inline std::complex<T>  fft_sincos_get( const size_t idx )
		{
			constexpr std::size_t S=17;
			static constexpr fft_sin_cos<T,S> sin_tab = create_fft_sin_cos<T,S>( );
			return ( sin_tab(idx) );
		}
		//Bit reverseW
#	ifndef __arm__
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
#	else
		inline unsigned bitrev( unsigned value, int m )
		{
			unsigned result;
			asm ("rbit %0, %1" : "=r" (result) : "r" (value) );
			return result >> ( 32 - m );
		}
#	endif

		//Multiply integer
		template<typename T,typename A=typename common::types<T>::acc_t >
		typename std::enable_if<std::is_integral<T>::value, T>::type
		inline impy( T x, T y )
		{
			static constexpr A scale = A(std::numeric_limits<T>::max()) + 1;
			static constexpr T K = scale>>1;
			return (A(x) * A(y) + K) / scale;
		}
		//Multiply floating
		template<typename T, typename A>
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
		//Scale for integer value
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
		// Fractional Q21.10 for sinus linear interpolation
		namespace q_arithmetic_for_sin {
			constexpr auto Q = 10U;
			inline uint32_t qdiv( uint32_t a, uint32_t b ) {
				uint64_t temp = uint64_t(a) << Q;
				temp += b / 2;
				return temp / b;
			}
			inline uint32_t qmul( uint32_t a, uint32_t b ) {
				auto temp = uint64_t(a) * uint64_t(b);
				temp += 1 << (Q - 1);
				return temp >> Q;
			}
			constexpr inline uint32_t ui2q( uint32_t c ) {
				return c << Q;
			}
			constexpr inline uint32_t q2ui( uint32_t c ) {
				return c >> Q;
			}
			constexpr inline uint32_t qfract( uint32_t c ) {
				return c & ((1<<Q)-1);
			}
			//Multiply unsigned fractional by integer
			inline uint32_t mulqi( uint32_t a, int32_t b ) {
				auto temp = int64_t(a) * int64_t(b);
				return temp >> Q;
			}
		}

		//Internal SINUS integer version
		template <typename T>
		inline std::complex<T> fft_sin_cos_int( uint32_t le2, uint32_t i )
		{
			using namespace q_arithmetic_for_sin;
			constexpr size_t SINSIZE = 256;
			constexpr uint32_t CPI = dsp::integer::trig::sin_arg_max<T,SINSIZE>()/2;
			auto angle = qdiv(ui2q(CPI*i),ui2q(le2));
			const auto a2 = qfract( angle );
			const auto a1 = ui2q(1) - a2;
			angle = q2ui( angle );
			constexpr auto cos = dsp::integer::trig::cos<T,SINSIZE>;
			constexpr auto sin = dsp::integer::trig::sin<T,SINSIZE>;
			return {
				  T( mulqi(a1,cos(angle)) + mulqi(a2,cos(angle+1)) ),
			      T(-mulqi(a1,sin(angle)) - mulqi(a2,sin(angle+1)) )
			};
		}

//ns internal
}}


}}
