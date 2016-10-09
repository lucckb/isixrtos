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

#if 0
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
		ASSERT_NEAR( res[i], array[i], 0.01 );
	}
}
#endif


