/*
 * array_sinus.hpp
 *
 *  Created on: 28-03-2013
 *      Author: lucck
 */

#pragma once

#include <cstddef>
#include <limits>
#include <cmath>

namespace dsp {
namespace integer {
  namespace _internal {


	  template<unsigned... Is> struct seq{};
	  template<unsigned N, unsigned... Is>
		  struct gen_seq : gen_seq<N-1, N-1, Is...>{};
	  template<unsigned... Is>
		  struct gen_seq<0, Is...> : seq<Is...>{};

	  template <typename T, std::size_t S>
		  class SinInternal
		  {
			  private:
				  static constexpr std::size_t elm = S -1;
				  constexpr int quad( const T phase ) const
				  {
					  return (phase / elm ) %4;
				  }
				  constexpr std::size_t angle( const T phase ) const
				  {
					  return phase % elm;
				  }
			  public:
				  constexpr T operator()( const T phase ) const
				  {
					  return ((quad(phase)==0)?(sine_array[angle(phase)]) :
							  ((quad(phase)==1)?(sine_array[elm - angle(phase)]) :
							   ((quad(phase)==2)?(-sine_array[angle(phase)]) :
								((quad(phase)==3)?(-sine_array[elm-angle(phase)]):(0)))));
				  }
				  T sine_array[S];
				  static constexpr T pi2() { return  4 * elm; }
		  };
	  template <typename T>
		  constexpr T genSin(T v, std::size_t s, T max_value)
		  {
			  return std::sin( v * (2.0 * std::atan(1.0)) / double(s-1) ) * double(max_value) + 0.5;
		  }

	  template<typename T, std::size_t S, unsigned... Is>
		  constexpr SinInternal<T,S> CreateSinSinInternal(T max_value, seq<Is...>)
		  {
			  return {{ genSin<T>(Is, S, max_value)... }};
		  }

	  template <typename T, std::size_t S >
		  constexpr SinInternal<T,S> CreateSinSinInternal( T max_value = std::numeric_limits<T>::max() )
		  {
			  return CreateSinSinInternal<T,S>(max_value,gen_seq<S>{});
		  }

      }
      namespace trig
      {
		  template < typename T, std::size_t S > constexpr T sin_arg_max()
		  {
			  return _internal::SinInternal<T,S>::pi2();
		  }

	  template < typename T, std::size_t S, T MAX = std::numeric_limits<T>::max() > T sin( const T angle )
		  {
			static constexpr _internal::SinInternal<T,S> sin_tbl = _internal::CreateSinSinInternal<T,S>( MAX );
			constexpr auto mod = sin_arg_max<T,S>();
			return sin_tbl( angle % mod );
		  }


      }
 }}


