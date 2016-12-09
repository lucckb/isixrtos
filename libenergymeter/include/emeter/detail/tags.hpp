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
#pragma once

#include <emeter/types.hpp>

namespace emeter {
namespace tags {
namespace detail {
	//! Non billing information voltage currrent
	struct u_rms {  using value_type = measure_t; };
	struct i_rms {  using value_type = measure_t; };
	struct thd_u { using value_type = measure_t;  };
	struct thd_i { using value_type = measure_t;  };
	struct pwr_fact { using value_type = measure_t; };
	struct freq { using value_type = measure_t; };

	//! Non biling information
	struct p_avg { using value_type = measure_t; };
	struct q_avg { using value_type = measure_t; };
	struct s_avg { using value_type = measure_t; };

	//! Billing information
	struct var_h_pos { using value_type = energymeas_t; };
	struct watt_h_pos { using value_type = energymeas_t; };
	struct var_h_neg { using value_type = energymeas_t; };
	struct watt_h_neg { using value_type = energymeas_t; };

	//! Non billing pure raw data for storage
	struct raw_var_h_pos { using value_type = accum_t; };
	struct raw_watt_h_pos { using value_type = accum_t; };
	struct raw_var_h_neg { using value_type = accum_t; };
	struct raw_watt_h_neg { using value_type = accum_t; };
}
	namespace {
		constexpr detail::var_h_pos  var_h_pos{};
		constexpr detail::watt_h_pos watt_h_pos{};
		constexpr detail::var_h_neg var_h_neg{};
		constexpr detail::watt_h_neg watt_h_neg{};
		constexpr detail::p_avg p_avg{};
		constexpr detail::q_avg q_avg{};
		constexpr detail::s_avg s_avg{};
		constexpr detail::u_rms u_rms{};
		constexpr detail::i_rms i_rms{};
		constexpr detail::thd_u thd_u{};
		constexpr detail::thd_i thd_i{};
		constexpr detail::pwr_fact pwr_fact{};
		constexpr detail::freq freq{};
		constexpr detail::raw_var_h_pos raw_var_h_pos{};
		constexpr detail::raw_var_h_neg raw_var_h_neg{};
		constexpr detail::raw_watt_h_pos raw_watt_h_pos{};
		constexpr detail::raw_watt_h_neg raw_watt_h_neg{};
	}
}
}
