/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:  Open EFM types
 *
 *        Version:  1.0
 *        Created:  30.03.2016 19:54:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

namespace emeter {
	//! Type used for energy calculation
	using energy_t = unsigned long long;
	//! Type used for voltage calculation
	using voltage_t = float;
	//! Type used for current calculation
	using current_t = float;
	//! Type used for sampling 
	using sample_t =  unsigned short;
	//! Thd type 
	using thd_t = float;
}
