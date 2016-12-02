/*
 * =====================================================================================
 *
 *       Filename:  simulate_pwr_ui.hpp
 *
 *    Description:  Helper for simulate pwr ui
 *
 *        Version:  1.0
 *        Created:  12.10.2016 21:46:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <cstdint>
#include <vector>
#include <memory>
#include <cmath>

#pragma once
namespace sim {

	using meas_t = int16_t;
	using phases_buf_t = std::array<std::vector<meas_t>,3>;
namespace {
	constexpr auto Vref = 3.3;
	constexpr auto R2 = 1E6;
	constexpr auto R1 = 3E3;
	constexpr auto KU =  R2/R1;
}
	/**  Generate 3 phase sine table using multicore
	 *	 @param[in] ampl Aplitude V
	 *	 @param[in] freq Input freq
	 *	 @param[in] angle Start angle
	 *	 @param[in] Time duration
	 */
	phases_buf_t generate_sinus( double ampl, double freq, double angle, 
			double time, double ku=KU);



	constexpr auto deg2rad( double deg ) {
		return (deg/360.0)*2.0*M_PI;
	}


}

inline sim::phases_buf_t operator+( sim::phases_buf_t&& o1, const sim::phases_buf_t &o2 )
{
	for( size_t x=0;x<o1.size();++x )
	if( o1[x].size() != o2[x].size() ) {
		throw std::length_error("Array mismatch");
	}
	for( size_t x=0;x<o1.size();++x )
	for( size_t y=0;y<o1[x].size(); ++y ) {
		o1[x][y] += o2[x][y];
	}
	return std::move(o1);
}

inline sim::phases_buf_t operator+( sim::phases_buf_t&& o1, const double dcv )
{
	for( size_t x=0;x<o1.size();++x )
	for( size_t y=0;y<o1[x].size(); ++y ) {
		o1[x][y] += dcv;
	}
	return std::move(o1);
}

