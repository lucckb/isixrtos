/*
 * =====================================================================================
 *
 *       Filename:  emeter_phase_impl.hpp
 *
 *    Description:   Emeter phase implementation class
 *
 *        Version:  1.0
 *        Created:  02.04.2016 23:00:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <emeter/detail/tags.hpp>
#include <emeter/detail/energy_phase_bufs.hpp>
#include <atomic>

namespace emeter {
	class energy_phase_n : public detail::energy_phase_bufs
	{
	public:
		energy_phase_n( energy_phase_n& ) = delete;
		energy_phase_n& operator=(energy_phase_n&) = delete;
		energy_phase_n( void *scratch ) 
			: m_scratch( scratch ) {
		}
		virtual ~energy_phase_n() {
		}
		// Get URMS
		typename tags::detail::u_rms::value_type
			operator()( const tags::detail::u_rms& ) const noexcept {
			return m_u.load();
		}
		// Get IRMS
		typename tags::detail::i_rms::value_type
			operator()( const tags::detail::i_rms& ) const noexcept {
			return m_i.load();
		}
	protected:
		//!Calculate FFT voltage
		int calculate_u( const sample_t* raw_u, std::size_t size ) noexcept override;
		//! Caculate FFT current
		int calculate_i( const sample_t* raw_i, std::size_t size ) noexcept override;
	private:
		std::atomic<typename tags::detail::u_rms::value_type> m_u;
		std::atomic<typename tags::detail::i_rms::value_type> m_i;
		std::atomic<typename tags::detail::thd_u::value_type> m_thdu;
		std::atomic<typename tags::detail::thd_i::value_type> m_thdi;
		void* const m_scratch; //! Scratch memory temporary_fft
	};
}
