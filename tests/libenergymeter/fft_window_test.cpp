/*
 * =====================================================================================
 *
 *       Filename:  fft_window_test.cpp
 *
 *    Description:  FFT window test
 *
 *        Version:  1.0
 *        Created:  09.10.2016 15:48:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <gtest/gtest.h>
#include <foundation/dsp/fft_window.hpp>
#include <boost/lexical_cast.hpp>
#include <limits>
#include <algorithm>
#include "gprint.hpp"
#include "fft_tests_common.hpp"
#include <pstreams/pstream.h>

//Test for windowing fcns
namespace {
	std::vector<double> hanning_octave( size_t points, double mul ) {
		redi::ipstream in( 
			std::string("./hanning.sh ") +
			boost::lexical_cast<std::string>(points) + " " +
			boost::lexical_cast<std::string>(mul) 
		);
		std::string str;
		std::vector<double> ret;
		while( in >> str ) {
			ret.push_back( boost::lexical_cast<double>( str ) );
		}
		return ret;
	}
}

TEST( window_test , int16_test ) {
	using namespace dsp::window;
	constexpr auto WS = 256;
	using type = int16_t;
	type array[WS];
	for( auto &a : array ) {
		a = std::numeric_limits<type>::max();
	}
	apply_hanning<WS>( array, array );
	auto res = hanning_octave( WS, std::numeric_limits<type>::max() );
	ASSERT_EQ( WS, res.size() );
	for( size_t i=0;i<WS;++i ) {
		SCOPED_TRACE( i );
		ASSERT_NEAR( res[i], array[i], 1.5 );
	}
}


TEST( window_test , int32_test ) {
	using namespace dsp::window;
	constexpr auto WS = 256;
	using type = int32_t;
	type array[WS];
	for( auto &a : array ) {
		a = std::numeric_limits<type>::max();
	}
	apply_hanning<WS>( array, array );
	auto res = hanning_octave( WS, std::numeric_limits<type>::max() );
	ASSERT_EQ( WS, res.size() );
	for( size_t i=0;i<WS;++i ) {
		SCOPED_TRACE( i );
		ASSERT_NEAR( res[i], array[i], 1.5 );
	}
}

TEST( window_test , double_test ) {
	using namespace dsp::window;
	constexpr auto WS = 256;
	using type = double;
	type array[WS];
	for( auto &a : array ) {
		a = 1.0;
	}
	apply_hanning<WS>( array, array );
	auto res = hanning_octave( WS, 1.0 );
	ASSERT_EQ( WS, res.size() );
	for( size_t i=0;i<WS;++i ) {
		SCOPED_TRACE( i );
		ASSERT_NEAR( res[i], array[i], 0.001 );
	}
}

TEST( window_test , float_test ) {
	using namespace dsp::window;
	constexpr auto WS = 256;
	using type = float;
	type array[WS];
	for( auto &a : array ) {
		a = 1.0;
	}
	apply_hanning<WS>( array, array );
	auto res = hanning_octave( WS, 1.0 );
	ASSERT_EQ( WS, res.size() );
	for( size_t i=0;i<WS;++i ) {
		SCOPED_TRACE( i );
		ASSERT_NEAR( res[i], array[i], 0.001 );
	}
}

TEST( window_test, hann_to_another_array ) {

	using namespace dsp::window;
	constexpr auto WS = 256;
	using type = int32_t;
	type array[WS];
	type dest_array[WS];
	for( auto &a : array ) {
		a = std::numeric_limits<type>::max();
	}
	const auto src = array;
	apply_hanning<WS>( src, dest_array );
	auto res = hanning_octave( WS, std::numeric_limits<type>::max() );
	ASSERT_EQ( WS, res.size() );
	for( size_t i=0;i<WS;++i ) {
		SCOPED_TRACE( i );
		ASSERT_NEAR( res[i], dest_array[i], 1.5 );
		ASSERT_EQ( src[i], std::numeric_limits<type>::max() );
	}
}


TEST( window_test, hann_src_int16_dst_float ) {

	using namespace dsp::window;
	constexpr auto WS = 256;
	using type = int16_t;
	type array[WS];
	float dest_array[WS];
	for( auto &a : array ) {
		a = std::numeric_limits<type>::max();
	}
	const auto src = array;
	apply_hanning<WS>( src, dest_array );
	auto res = hanning_octave( WS, 1.0 );
	ASSERT_EQ( WS, res.size() );
	for( size_t i=0;i<WS;++i ) {
		SCOPED_TRACE( i );
		ASSERT_NEAR( res[i], dest_array[i], 0.01 );
		ASSERT_EQ( src[i], std::numeric_limits<type>::max() );
	}
}

TEST( window_test, hann_sum_of_raw_window )
{
	using namespace dsp::window;
	constexpr auto value = sum_hanning<256,float>(false);
	ASSERT_NEAR( value, 127.5, 0.00001 );
	constexpr auto v1024 = sum_hanning<1024,float>(false);
	ASSERT_NEAR( v1024, 511.5, 0.00001 );
}
