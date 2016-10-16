/*
 * =====================================================================================
 *
 *       Filename:  simulate_pwr_ui.cpp
 *
 *    Description:  Simulate power and UI
 *
 *        Version:  1.0
 *        Created:  12.10.2016 20:42:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <emeter/detail/config.hpp>
#include <vector>
#include <memory>
#include <limits>
#include <cmath>
#include <future>
#include "simulate_pwr_ui.hpp"
namespace sim {


namespace {


		constexpr meas_t d2m( double V, double k ) {
			auto tmp =  V/k;
			if( tmp > Vref/2 )
				tmp = Vref/2;
			if( tmp < -Vref/2 )
				tmp = Vref/2;
			tmp = tmp/(Vref/2) * std::numeric_limits<meas_t>::max();
			return std::round( tmp );
		}

		//Get single phase for async calc
		std::vector<meas_t> gen_1ph_sin( double ampl, double freq,
				double angle, double time, int phase, double ku )
		{
			std::vector<meas_t> l1;
			l1.reserve( time * (1.0/emeter::config::sample_rate) + 1 );
			for( double ct=0; ct<time; ct+= 1.0/emeter::config::sample_rate ) {
				const auto A = ampl*std::sqrt(2) *
				std::sin( 2.0*M_PI*freq*ct + (phase/360.0)*2.0*M_PI + (angle/360.0)*2.0*M_PI );
				l1.push_back( d2m( A,ku ) );
			}
			return l1;
		}
}


	/**  Generate 3 phase sine table using multicore
	 *	 @param[in] ampl Aplitude V
	 *	 @param[in] freq Input freq
	 *	 @param[in] angle Start angle
	 *	 @param[in] Time duration
	 */
	phases_buf_t generate_sinus( double ampl, double freq,
			double angle, double time, double ku )
	{
		auto al2 = std::async(std::launch::async, gen_1ph_sin, ampl, freq, angle, time, 2, ku );
		auto al3 = std::async(std::launch::async, gen_1ph_sin, ampl, freq, angle, time, 3, ku );
		auto l1 = gen_1ph_sin( ampl, freq, angle, time, 1, ku );
		phases_buf_t ret {{ std::move(l1), std::move(al2.get()), std::move(al3.get()) }};
		return ret;
	}

}


