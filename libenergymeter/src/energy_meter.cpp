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


//! Adjust input energy cuttof when energy is less than 0.5W
int energy_meter::scale_energy_mul( measure_t e ) const noexcept
{
	if( std::abs(e) < m_power_tresh ) {
		return 0;
	} else {
		return std::lround( e * measure_t(ecnt_scale) );
	}
}

//! Calculate energies based on the phase
void energy_meter::calculate_energies( pwr_cnt& ecnt, const energy_phase_n& ephn ) noexcept
{
	if( ephn(emeter::tags::i_rms) >= m_current_tresh )
	{
		auto val = scale_energy_mul( ephn( emeter::tags::p_avg ) );
		if( val > 0 ) {
			ecnt.p_plus += val;
		} else {
			ecnt.p_minus += -val;
		}
		val = scale_energy_mul( ephn( emeter::tags::q_avg ) );
		if( val > 0 ) {
			ecnt.q_plus += val;
		} else {
			ecnt.q_minus += -val;
		}
	}
}


//! Wh consumed
typename tags::detail::watt_h_pos::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::watt_h_pos p ) const noexcept
{
	auto val = read_atomic_accum_t( m_ecnt[phase].p_plus );
	return acc_to_energy( val, p );
}

//! Wh produced
typename tags::detail::watt_h_neg::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::watt_h_neg p) const noexcept
{
	auto val = read_atomic_accum_t( m_ecnt[phase].p_minus );
	return acc_to_energy( val, p );
}


//! varh consumed
typename tags::detail::var_h_pos::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::var_h_pos p) const noexcept
{
	auto val = read_atomic_accum_t( m_ecnt[phase].q_plus );
	return acc_to_energy( val, p );
}


//! varh produced
typename tags::detail::var_h_neg::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::var_h_neg p) const noexcept
{
	auto val = read_atomic_accum_t( m_ecnt[phase].q_minus );
	return acc_to_energy( val, p );
}

// Wh consumed
void energy_meter::operator()
	( const std::size_t phase, tags::detail::raw_watt_h_pos,
	  typename tags::detail::raw_watt_h_pos::value_type val ) noexcept
{
	m_ecnt[phase].p_plus = val;
}

// Wh produced
void energy_meter::operator()
	( const std::size_t phase, tags::detail::raw_watt_h_neg,
	  typename tags::detail::raw_watt_h_neg::value_type val ) noexcept
{
	m_ecnt[phase].p_minus = val;
}

// varh consumed
void energy_meter::operator()
	( const std::size_t phase, tags::detail::raw_var_h_pos,
	  typename tags::detail::raw_var_h_pos::value_type val ) noexcept
{
	m_ecnt[phase].q_plus = val;
}

// varh produced
void energy_meter::operator()
	( const std::size_t phase, tags::detail::raw_var_h_neg,
	  typename tags::detail::raw_var_h_neg::value_type val ) noexcept
{
	m_ecnt[phase].q_minus = val;
}

//raw Wh consumed
typename tags::detail::raw_watt_h_pos::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::raw_watt_h_pos ) const noexcept
{
	return read_atomic_accum_t( m_ecnt[phase].p_plus );
}

// raw Wh produced
typename tags::detail::raw_watt_h_neg::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::raw_watt_h_neg ) const noexcept
{
	return read_atomic_accum_t( m_ecnt[phase].p_minus );
}

// raw vear consumed
typename tags::detail::raw_var_h_pos::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::raw_var_h_pos ) const noexcept
{
	return read_atomic_accum_t( m_ecnt[phase].q_plus );
}

// raw varh produced
typename tags::detail::raw_var_h_neg::value_type energy_meter::operator()
	( const std::size_t phase, tags::detail::raw_var_h_neg ) const noexcept
{
	return read_atomic_accum_t( m_ecnt[phase].q_minus );
}


}

