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
#include <emeter/types.hpp>

namespace emeter {
namespace tags {

	//! Non billing information voltage currrent
	struct u_rms {  using value_type = measure_t; };
	struct i_rms {  using value_type = measure_t; };
	struct thd { using value_type = measure_t;  };
	struct pwr_fact { using value_type = measure_t; };
	struct freq { using value_type = measure_t; };

	//! Non biling information
	struct p_avg { using value_type = measure_t; }; 
	struct q_avg { using value_type = measure_t; };
	struct s_avg { using value_type = measure_t; };

	//! Billing information
	struct var_h_pos { using value_type = accum_t; };
	struct watt_h_pos { using value_type = accum_t; };
	struct var_h_neg { using value_type = accum_t; };
	struct watt_h_neg { using value_type = accum_t; };

}
}

