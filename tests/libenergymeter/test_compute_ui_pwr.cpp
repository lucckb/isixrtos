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
#include <cmath>

namespace {
	constexpr auto fftsize = emeter::config::fft_size;
	constexpr auto u_scale =  333.333 * 1.65;
	constexpr auto i_scale = 1.65;
	struct ptest {
		double u;
		double i;
		double p;
		double q;
		double s;
	};
}

namespace {

		ptest normal_test( const sim::phases_buf_t& U ,const sim::phases_buf_t& I ) {
			emeter::energy_phase_n o(  new char[8192]  );
			o.set_scale_u( u_scale );
			o.set_scale_i( i_scale );
			auto vb = o.sample_voltage_begin();
			if( !vb ) throw std::bad_alloc();
			std::copy( &U[0][0], &U[0][fftsize+fftsize/2], vb );
			o.sample_voltage_end();
			auto vi = o.sample_current_begin();
			if( !vi ) throw std::bad_alloc();
			std::copy( &I[0][0], &I[0][fftsize+fftsize/2], vi );
			o.sample_current_end();
			o.calculate();
			return {
				o( emeter::tags::u_rms ),
				o( emeter::tags::i_rms ),
				o( emeter::tags::p_avg ),
				o( emeter::tags::q_avg ),
				o( emeter::tags::s_avg )
			};
		}

}




//! Base fft calculation options
TEST( energy_phase_n_compute, sinusoidal_energy_angle ) {
	constexpr auto sim_duration = 5;
	constexpr auto voltage = 230.0;
	constexpr auto current = 1.0;
	constexpr auto freq = 50;
	for( int angle=0;angle<=360;angle+=1 ) {
		auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
		auto i_vect = sim::generate_sinus( current, freq, 0, sim_duration, 1 );
		auto r = normal_test( v_vect, i_vect );
		//Sinusoidal wave power factors compare
		ASSERT_NEAR( r.p, voltage*current*std::cos( sim::deg2rad(angle)), 0.05 ) ;
		ASSERT_NEAR( r.q, voltage*current*std::sin( sim::deg2rad(angle)), 0.05 ) ;
		ASSERT_NEAR( r.s, voltage*current, 0.05 );
		ASSERT_NEAR( r.u, voltage, 0.05 );
		ASSERT_NEAR( r.i, current, 0.05 );
	}
}





