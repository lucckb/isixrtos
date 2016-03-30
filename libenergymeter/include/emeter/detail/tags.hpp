/*
 * =====================================================================================
 *
 *       Filename:  tags.hpp
 *
 *    Description:  Tags for energy calculation
 *
 *        Version:  1.0
 *        Created:  30.03.2016 21:14:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
namespace emeter {
namespace tag {

	//! Non billing information voltage currrent
	struct u_rms {};
	struct i_rms {};
	struct thd {};
	struct pwr_fact {};
	struct freq {};

	//! Non biling information
	struct p_avg {}; 
	struct q_avg {};
	struct s_avg {};

	//! Billing information
	struct var_h_pos {};
	struct watt_h_pos {};
	struct var_h_neg {};
	struct watt_h_neg {};
}
}

