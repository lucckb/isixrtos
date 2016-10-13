/*
 * =====================================================================================
 *
 *       Filename:  energy_phase_bufs.cpp
 *
 *    Description:  Energy pgase bufs calculator
 *
 *        Version:  1.0
 *        Created:  11.10.2016 20:19:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <emeter/detail/energy_phase_bufs.hpp>

namespace emeter {
namespace detail {


int energy_phase_bufs::calculate() noexcept
{
	const buf_stat bs = m_states.load();
	int err { e_not_yet };
	auto ui = bs.find( state::ready_u );
	auto ii = bs.find( state::ready_i );
	if( ui!=buf_stat::npos && ii!=buf_stat::npos ) {
		do_calculate( m_buf[ui].data(), m_buf[ii].data() );
		sample_change_state_idx( ui, state::free );
		sample_change_state_idx( ii, state::free );
		err = e_ok;
	}
	return err;
}


sample_t* energy_phase_bufs::sample_find_change_state( state find, state replace ) noexcept
{
	sample_t* ret {};
	unsigned newval;
	unsigned oldval;
	do {
		oldval = m_states.load();
		const auto f = buf_stat(oldval).find( find );
		if( f != buf_stat::npos ) {
			newval = buf_stat(oldval).set( f, replace );
			ret = m_buf[f].data();
		} else {
			return nullptr;
		}
	} while( !m_states.compare_exchange_weak(oldval,newval) );
	return ret;

}

}}
