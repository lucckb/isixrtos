/*
 * =====================================================================================
 *
 *       Filename:  test_sin_array.cpp
 *
 *    Description:  Test sinus array
 *
 *        Version:  1.0
 *        Created:  04.10.2016 21:58:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#include <gtest/gtest.h>
#include <foundation/dsp/fft.hpp>
#include <limits>
#include "gprint.hpp"
#include "foundation/dsp/array_sinus.hpp"
#include <cmath>



template <typename T, std::size_t N >
void match_integer_angle() {
	using namespace dsp::integer::trig;
	const auto amax = sin_arg_max<T,N>();
	const auto max = std::numeric_limits<T>::max();
	for( int a=-amax*32; a<amax*32; ++a ) {
		const double d_angle = ( double(a) / double(amax) ) * M_PI * 2.0;
		const auto sv0 =  std::sin( d_angle );
		const auto sv1 = double(sin<T,N>( a ) ) /  max;
		ASSERT_NEAR( sv0, sv1, 0.001 );
	}
}


template <typename T, std::size_t N >
void match_integer_anglec() {
	using namespace dsp::integer::trig;
	const auto amax = sin_arg_max<T,N>();
	const auto max = std::numeric_limits<T>::max();
	for( int a=-amax*32; a<amax*32; ++a ) {
		const double d_angle = ( double(a) / double(amax) ) * M_PI * 2.0;
		const auto sv0 =  std::cos( d_angle );
		const auto sv1 = double(cos<T,N>( a ) ) /  max;
		ASSERT_NEAR( sv0, sv1, 0.01 );
	}
}


TEST( sin_array,  compare_int16 ) {
	using T = int16_t;
	match_integer_angle<T, 8>();
	match_integer_angle<T, 16>();
	match_integer_angle<T, 32>();
	match_integer_angle<T, 64>();
	match_integer_angle<T, 128>();
	match_integer_angle<T, 256>();
	match_integer_angle<T, 512>();
}

#if 1
TEST( sin_array,  cos_compare_int16 ) {
	using T = int16_t;
	match_integer_anglec<T, 8>();
	match_integer_anglec<T, 16>();
	match_integer_anglec<T, 32>();
	match_integer_anglec<T, 64>();
	match_integer_anglec<T, 128>();
	match_integer_anglec<T, 256>();
	match_integer_anglec<T, 512>();
}
#endif



TEST( sin_array,  compare_int32 ) {
	using T = int32_t;
	match_integer_angle<T, 8>();
	match_integer_angle<T, 16>();
	match_integer_angle<T, 32>();
	match_integer_angle<T, 64>();
	match_integer_angle<T, 128>();
	match_integer_angle<T, 256>();
	match_integer_angle<T, 512>();
}


TEST( sin_array,  cos_compare_int32 ) {
	using T = int32_t;
	match_integer_anglec<T, 8>();
	match_integer_anglec<T, 16>();
	match_integer_anglec<T, 32>();
	match_integer_anglec<T, 64>();
	match_integer_anglec<T, 128>();
	match_integer_anglec<T, 256>();
	match_integer_anglec<T, 512>();
}


