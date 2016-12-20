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
#include <tuple>


namespace emeter {

namespace {
	constexpr auto fftsiz = config::fft_size;
}

namespace {
	template <typename T,typename U>
	std::pair<T,U> operator+(const std::pair<T,U> & l,const std::pair<T,U> & r) {
		return {l.first+r.first,l.second+r.second};
}
}

// Calculate single stage u or i
const cplxmeas_t* energy_phase_n::fft_calc( void* result, const sample_t* raw )
{
	constexpr auto wnd_s = dsp::window::sum_hanning<fftsiz,measure_t>(true);
	constexpr measure_t wscale =  std::sqrt( wnd_s * fftsiz );
	auto fft_in = reinterpret_cast<measure_t*>(result);
	auto fft_out = reinterpret_cast<cplxmeas_t*>(result);
	dsp::window::apply_hanning<fftsiz>( raw, fft_in );
	dsp::refft::fft_real( fft_out, fft_in, config::fft_bits );
	for( size_t i=0; i<=fftsiz/2; ++i ) {
		fft_out[i] = std::sqrt(measure_t(2.0)) * fft_out[i] / wscale;
	}
	return reinterpret_cast<const cplxmeas_t*>( result );
}

// RMS calculate
measure_t energy_phase_n::rms( const cplxmeas_t input[] ) {
	measure_t sum { };
	//from 1 to N/2-1 - Remove DC offset
	for( size_t i=2; i<fftsiz/2; ++i ) {
		const auto r = input[i];
		sum += r.real()*r.real()+r.imag()*r.imag();
	}
	return std::sqrt( sum );
}
// Measure power
std::pair<measure_t,measure_t>
	energy_phase_n::power( const cplxmeas_t ub[], const cplxmeas_t ib[] )
{
	measure_t sum_p {};
	measure_t sum_q {};
	//from 1 to N/2-1 - Remove DC offset
	for( size_t i=2; i<fftsiz/2; ++i ) {
		const auto P = ub[i] * std::conj(ib[i]);
		sum_p += P.real();
		sum_q += P.imag();
	}
	return std::make_pair( sum_p, sum_q );
}


//! TODO: maybe thd as extras
//!Calculate FFT voltage
void energy_phase_n::do_calculate( const sample_t* raw_u, const sample_t* raw_i ) noexcept
{
	measure_t U {};
	measure_t I {};
	std::pair<measure_t,measure_t> P {};
	//Do loop twice due to lack of data (windowing)
	for( size_t h1=0; h1<2; ++h1,raw_u+=fftsiz/2,raw_i+=fftsiz/2 ) {
		const auto fft_u = fft_calc( m_scratch,  raw_u );
		const auto fft_i = fft_calc( &reinterpret_cast<cplxmeas_t*>(m_scratch)[fftsiz/2], raw_i );
		U = U + rms( fft_u );
		I = I + rms( fft_i );
		P = P + power( fft_u, fft_i );
	}
	U = (U/2)*m_scale_u;
	I = (I/2)*m_scale_i;
	P.first = (P.first/2)*m_scale_u*m_scale_i;
	P.second = (P.second/2)*m_scale_u*m_scale_i;
	m_U.store( U );
	m_I.store( I );
	m_P.store( P.first  );
	m_Q.store( P.second );
	m_S.store( U * I );
}

}

