/*
 * =====================================================================================
 *
 *       Filename:  energy_meter.cpp
 *
 *    Description:  Complex energy meter samples library
 *
 *        Version:  1.0
 *        Created:  16.10.2016 17:38:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <emeter/energy_meter.hpp>

namespace emeter {


//Adjust input energy cuttof when energy is less than 0.5W
measure_t energy_meter::scale_energy_mul( measure_t e ) {
	if( e > 0 && e<config::energy_cnt_tresh  ) {
		return 0;
	} else if( e < 0 && e>-config::energy_cnt_tresh ) {
		return 0;
	} else {
		return std::round( e * measure_t(ecnt_scale) );
	}
}


//! Calculate energies based on the phase
void energy_meter::calculate_energies( pwr_cnt& ecnt, const energy_phase_n& ephn ) noexcept
{
	measure_t val;
	val = scale_energy_mul( ephn( emeter::tags::p_avg ) );
	if( val > 0 ) {
		ecnt.p_plus += measure_t(val);
	} else {
		ecnt.p_minus += measure_t(-val);
	}
	val = scale_energy_mul( ephn( emeter::tags::q_avg ) );
	if( val > 0 ) {
		ecnt.q_plus += measure_t(val);
	} else {
		ecnt.q_minus += measure_t(-val);
	}
}


// Wh consumed
typename tags::detail::watt_h_pos::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::watt_h_pos p ) const noexcept
{
	auto val = read_atomic_accum_t( m_ecnt[phase].p_plus );
	return scale_energy_div( val, p );
}

// Wh produced
typename tags::detail::watt_h_neg::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::watt_h_neg p) const noexcept
{
	auto val = read_atomic_accum_t( m_ecnt[phase].p_minus );
	return scale_energy_div( val, p );
}


// varh consumed
typename tags::detail::var_h_pos::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::var_h_pos p) const noexcept
{
	auto val = read_atomic_accum_t( m_ecnt[phase].q_plus );
	return scale_energy_div( val, p );
}


// varh produced
typename tags::detail::var_h_neg::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::var_h_neg p) const noexcept
{
	auto val = read_atomic_accum_t( m_ecnt[phase].q_minus );
	return scale_energy_div( val, p );
}



}

