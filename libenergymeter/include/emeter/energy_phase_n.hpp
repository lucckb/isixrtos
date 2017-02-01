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
			return m_U.load();
		}
		// Get IRMS
		typename tags::detail::i_rms::value_type
			operator()( const tags::detail::i_rms& ) const noexcept {
			return m_I.load();
		}
		// Get Active power
		typename tags::detail::p_avg::value_type
			operator()( const tags::detail::p_avg& ) const noexcept {
			return m_P.load();
		}
		// Get Reactive power
		typename tags::detail::q_avg::value_type
			operator()( const tags::detail::q_avg& ) const noexcept {
			return m_Q.load();
		}
		// Get Reactive power
		typename tags::detail::s_avg::value_type
			operator()( const tags::detail::s_avg& ) const noexcept {
			return m_S.load();
		}
		// Set voltage scale
		void set_scale_u( measure_t scale ) noexcept {
			m_scale_u = scale;
		}
		// Set currrent scale
		void set_scale_i( measure_t scale ) noexcept {
			m_scale_i = scale;
		}
		// Get voltage scale
		measure_t get_scale_u() const noexcept {
			return m_scale_u;
		}
		// Get voltage scale
		measure_t get_scale_i() const noexcept {
			return m_scale_i;
		}
	protected:
		//!Calculate FFT voltage
		void do_calculate( const sample_t* raw_u, const sample_t* raw_i ) noexcept override;
	private:
		// Calculate single stage u or i
		static const cplxmeas_t* fft_calc( void* result, const sample_t* raw );
		// RMS calculate
		static measure_t rms( const cplxmeas_t input[] );
		// Measure power
		static std::pair<measure_t,measure_t>
			power( const cplxmeas_t ub[], const cplxmeas_t ib[] );
	private:
		std::atomic<typename tags::detail::u_rms::value_type> m_U;
		std::atomic<typename tags::detail::i_rms::value_type> m_I;
		std::atomic<typename tags::detail::p_avg::value_type> m_P;
		std::atomic<typename tags::detail::q_avg::value_type> m_Q;
		std::atomic<typename tags::detail::q_avg::value_type> m_S;
		void* const m_scratch; //! Scratch memory temporary_fft
		measure_t m_scale_u { 1.0 };
		measure_t m_scale_i { 1.0 };
	};
}
