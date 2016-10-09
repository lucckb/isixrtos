/*
 * =====================================================================================
 *
 *       Filename:  common.hpp
 *
 *    Description:  DSP common internal
 *
 *        Version:  1.0
 *        Created:  09.10.2016 12:28:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <cstdint>

namespace dsp {
namespace common {
namespace {

		template< typename T > struct types {
		};

		template<> struct types <int16_t> {
			using acc_t = int32_t;
		};

		template<> struct types <int32_t> {
			using acc_t = int64_t;
		};
		template<> struct types <float> {
			using acc_t = float;
		};
		template<> struct types <double> {
			using acc_t = double;
		};

		//Sequences
		template<unsigned... Is> struct seq{};
		template<unsigned N, unsigned... Is>
		struct gen_seq : gen_seq<N-1, N-1, Is...>{};
		template<unsigned... Is>
		struct gen_seq<0, Is...> : seq<Is...>{};

}}}
