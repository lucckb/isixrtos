/*
 * =====================================================================================
 *
 *       Filename:  test_compute_ui_pwr.cpp
 *
 *    Description:  Test compute for UI and pwr
 *
 *        Version:  1.0
 *        Created:  12.10.2016 20:40:40
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <emeter/energy_meter.hpp>
#include <gtest/gtest.h>
#include "gprint.hpp"
#include "simulate_pwr_ui.hpp"


namespace {
	constexpr auto fftsize = emeter::config::fft_size;
}

//! Base fft calculation options
TEST( energy_phase_n_compute, base_test ) {
	constexpr auto sim_duration = 60;
	const auto v_vect = sim::generate_sinus( 230, 50, 0, sim_duration );
	const auto i_vect = sim::generate_sinus( 1, 50, 90, sim_duration, 1 );
	emeter::energy_phase_n o(  new char[8192]  );
	o.set_scale_u( 333.333 * 1.65 );
	o.set_scale_i( 1.65 );
	auto vb = o.sample_voltage_begin();
	ASSERT_TRUE( vb );
	std::copy( &v_vect[0][0], &v_vect[0][fftsize+fftsize/2], vb );
	ASSERT_EQ( o.sample_voltage_end(), 0 );
	auto vi = o.sample_current_begin();
	ASSERT_TRUE( vi );
	std::copy( &i_vect[0][0], &i_vect[0][fftsize+fftsize/2], vi );
	ASSERT_EQ( o.sample_current_end(), 0 );
	ASSERT_EQ( o.calculate(), 0 );
	PRINTF("URMS: %f\n", o( emeter::tags::u_rms ) );
	PRINTF("IRMS: %f\n", o( emeter::tags::i_rms ) );
	PRINTF("P: %f\n", o( emeter::tags::p_avg ) );
	PRINTF("Q: %f\n", o( emeter::tags::q_avg ) );
	PRINTF("S: %f\n", o( emeter::tags::s_avg ) );
}





