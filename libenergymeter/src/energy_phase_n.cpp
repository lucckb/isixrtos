/*
 * =====================================================================================
 *
 *       Filename:  energy_phase_n.cpp
 *
 *    Description:  Calculate energy for single phase
 *
 *        Version:  1.0
 *        Created:  11.10.2016 20:23:18
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <emeter/energy_phase_n.hpp>
#include <foundation/dsp/fft_window.hpp>
#include <foundation/dsp/fft.hpp>
#include <cmath>

namespace emeter {

namespace {

	constexpr auto wnd_s1 = dsp::window::sum_hanning<config::fft_size,measure_t>();
	constexpr auto wnd_s2 = wnd_s1 * wnd_s1;
	constexpr measure_t wscale =  std::sqrt( wnd_s2 * config::fft_size );
}

//!Calculate FFT voltage
int energy_phase_n::calculate_u( const sample_t* raw_u, std::size_t size ) noexcept
{
	auto fft_in = reinterpret_cast<measure_t*>(m_scratch);
	auto fft_out = reinterpret_cast<cplxmeas_t*>(m_scratch);
	dsp::window::apply_hanning<config::fft_size>( raw_u, fft_in );
	dsp::refft::fft_real( fft_out, fft_in, config::fft_bits );
	for( size_t i=0; i<config::fft_size/2+1; ++i ) {
		fft_out[i] = std::sqrt(2.0f) * fft_out[i] / wscale;
	}
	return 0;
}
//! Caculate FFT current
int energy_phase_n::calculate_i( const sample_t* raw_i, std::size_t size ) noexcept
{
	(void)raw_i; (void)size;
	return 0;
}

}

