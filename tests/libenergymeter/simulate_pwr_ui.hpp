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

#pragma once
namespace sim {

	using meas_t = int16_t;
	using phases_buf_t = std::array<std::vector<meas_t>,3>;

	/**  Generate 3 phase sine table using multicore
	 *	 @param[in] ampl Aplitude V
	 *	 @param[in] freq Input freq
	 *	 @param[in] angle Start angle
	 *	 @param[in] Time duration
	 */
	phases_buf_t generate_sinus( double ampl, double freq, double angle, double time );

}

