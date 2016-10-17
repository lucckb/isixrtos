/*
 * =====================================================================================
 *
 *       Filename:  test_energy_meter.cpp
 *
 *    Description:  Energy meter class test
 *
 *        Version:  1.0
 *        Created:  10.10.2016 21:13:04
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
#include "simulate_pwr_ui.hpp"
#include "gprint.hpp"



//Helper function for apply buffer to all UI data
namespace {
	constexpr auto bufsiz = emeter::config::fftbuf_size;
	namespace tg = emeter::tags;
	constexpr auto u_scale =  333.333 * 1.65;
	constexpr auto i_scale = 1.65 * 100;

	// Apply energy meter on time
	void process_time( emeter::energy_meter& em, const sim::phases_buf_t& U,
			const sim::phases_buf_t& I )
	{
		for( size_t s=0; s<I[0].size(); s+= bufsiz ) {
			for( size_t ph=0; ph<I.size(); ++ph ) {
				//Current processing
				auto oh = em.sample_current_begin(ph);
				if(!oh) {
					throw std::logic_error("SCB");
				}
				std::copy( &I[ph][s], &I[ph][s+bufsiz], oh );
				if( em.sample_current_end( ph ) ) {
					throw std::logic_error("SCE");
				}
				//Voltage processing
				auto vb = em.sample_voltage_begin(ph);
				if(!vb) {
					throw std::logic_error("SCB");
				}
				std::copy( &U[ph][s], &U[ph][s+bufsiz], vb );
				if( em.sample_voltage_end( ph ) ) {
					throw std::logic_error("SCE");
				}
			}
			if( em.calculate() ) {
				throw std::logic_error("CALC");
			}
		}
	}
}

// Base rective energy only caculate up to 1KW
TEST( energy_meter, _1KW_r_only ) {
	emeter::energy_meter em;
	em.set_scale_u( u_scale );
	em.set_scale_i( i_scale );
	constexpr auto sim_duration = 3600*1;
	constexpr auto voltage = 230.0;
	constexpr auto current = 1.0;
	constexpr auto freq = 50;
	constexpr auto angle = 0;
	auto v_vect = sim::generate_sinus( voltage, freq, angle, sim_duration );
	auto i_vect = sim::generate_sinus( current, freq, 0, sim_duration, 1 );
	process_time( em, v_vect, i_vect );
	PRINTF("P %f Q %f\n", em(1,tg::p_avg), em(1,tg::q_avg) );
	PRINTF("U %f I %f\n", em(1,tg::u_rms), em(1, tg::i_rms) );
	PRINTF("1:E+ %f Q+ %f E- %f Q- %f\n", em(0,tg::watt_h_pos), em(0,tg::var_h_pos),
			em(0,tg::watt_h_neg), em(0,tg::var_h_neg ) );
	PRINTF("2:E+ %f Q+ %f E- %f Q- %f\n", em(1,tg::watt_h_pos), em(1,tg::var_h_pos),
			em(1,tg::watt_h_neg), em(1,tg::var_h_neg ) );
	PRINTF("3:E+ %f Q+ %f E- %f Q- %f\n", em(2,tg::watt_h_pos), em(2,tg::var_h_pos),
			em(2,tg::watt_h_neg), em(2,tg::var_h_neg ) );
	PRINTF("3f: E+ %f Q+ %f E- %f Q- %f\n", em(tg::watt_h_pos), em(tg::var_h_pos),
			em(tg::watt_h_neg), em(tg::var_h_neg ) );
}
