/*
 * =====================================================================================
 *
 *       Filename:  fft_window.hpp
 *
 *    Description:  FFT window function
 *
 *        Version:  1.0
 *        Created:  08.10.2016 22:26:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <type_traits>
#include <algorithm>
#include <cmath>
#include "common.hpp"

namespace dsp {
namespace window {
namespace _internal {
namespace {
//Internal ns begin
	template<typename Type, std::size_t Size>
	class wnd_array {
	public:
		constexpr Type operator()( const size_t m ) const {
			return m_array[m];
		}
		Type m_array[Size];
	};

	template<typename Type, Type (*Window_Func)(size_t,size_t)>
		constexpr Type get_element( const size_t idx, const size_t max )
		{
			return Window_Func(idx, max);
		}

	template<typename Type, size_t Size, Type (*Window_Func)(size_t,size_t), unsigned ... Is>
		constexpr wnd_array<Type,Size> generate_window_table( common::seq<Is...> )
		{
			return {{ get_element<Type,Window_Func>(Is,Size)... }};
		}

	template<typename Type, size_t Size, Type (*Window_Func)(size_t,size_t)>
		constexpr wnd_array<Type,Size> generate_window_table( )
		{
			return generate_window_table<Type,Size,Window_Func>( common::gen_seq<Size>{} );
		}
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
		template<typename T>
		inline typename std::enable_if<std::is_floating_point<T>::value, T>::type
		impy( T x, T y )
		{
			return (x * y);
		}
		//Multiply int by float return float
		template<typename I, typename F> 
		inline typename std::enable_if<std::is_floating_point<F>::value, F>::type
		impy( I x, F y ) {
			return (F(x)/std::numeric_limits<I>::max()) * y;
		}
		//Scale value
		template<typename T> inline constexpr
		typename std::enable_if<std::is_floating_point<T>::value, T>::type
		scale( double v ) {
			return v;
		}
		template<typename T> inline constexpr
		typename std::enable_if<std::is_integral<T>::value, T>::type
		scale( double v ) {
			constexpr auto max = std::numeric_limits<T>::max();
			return v * double(max) + 0.5;
		}
		template< typename T>
			constexpr T hanning_function( std::size_t n, std::size_t N ) {
				return scale<T>(0.5*(1.0-std::cos(double(2.0*M_PI*n)/double(N-1))));
			}

//Internal ns end
}}

	/** Generic aplay window function
	 * @param[in] input  Input data transform from i
	 * @param[out] output Output data transform to
	 * @return none */
	template <size_t Size, typename In_Type, typename Out_Type,
		typename std::iterator_traits<Out_Type>::value_type (*Func)(size_t,size_t)>
		void apply_generic( In_Type input,  Out_Type output )
	{
			using Dtype = typename std::iterator_traits<Out_Type>::value_type;
			std::size_t i = 0;
			std::transform( input, input+Size, output,
				[&]( auto inp ) {
					static constexpr _internal::wnd_array<Dtype,Size> wnd_tab =
						_internal::generate_window_table<Dtype,Size,Func>();
					return _internal::impy(inp, wnd_tab( i++ ));
				}
			);
	}

	/* CPP compile time sum */
	template <size_t Size, typename Type, Type (*Func)(size_t,size_t)>
		constexpr double sum_generic( const bool ispow2 ) {
			double sum {};
			for( size_t i=0; i<Size; ++i )
				sum += (Func(i,Size) * (ispow2?Func(i,Size):1.0) );
			return sum;
		}


	/** Hanning window */
	template<size_t Size, typename In_Type, typename Out_Type>
	void apply_hanning( In_Type input, Out_Type output )
	{
		apply_generic<Size, In_Type, Out_Type, _internal::hanning_function>
			(std::forward<In_Type>(input),std::forward<Out_Type>(output));
	}
	/** Sum of hanning window */
	template <size_t Size, typename Type>
		constexpr double sum_hanning( const bool ispow2 ) {
			return sum_generic<Size,Type,_internal::hanning_function>(ispow2);
		}
}}


