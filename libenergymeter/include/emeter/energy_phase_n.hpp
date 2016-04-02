/*
 * =====================================================================================
 *
 *       Filename:  energy_phase_n.hpp
 *
 *    Description:  Calculate energy on the N phase
 *
 *        Version:  1.0
 *        Created:  30.03.2016 21:57:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <cstddef>
#include <array>
#include <atomic>
#include "detail/tags.hpp"

namespace emeter {

	//! Calculate energy in the Nth phase
	template <std::size_t FFTLEN>
	class energy_phase_n {
	public:
		energy_phase_n( energy_phase_n& ) = delete;
		energy_phase_n& operator=(energy_phase_n&) = delete;
		energy_phase_n() {
		}
		// Get URMS
		typename tags::detail::u_rms::value_type 
			operator()( const tags::detail::u_rms& ) const noexcept
		{
			return m_u.load();
		}
		// Get IRMS
		typename tags::detail::i_rms::value_type 
			operator()( const tags::detail::i_rms& ) const noexcept
		{
			return m_i.load();
		}
		//ADC sampling process current
		sample_t* process_current() noexcept {
			return nullptr;
		}
		//ADC sampling process voltage
		sample_t* process_voltage() noexcept {
			return nullptr;
		}
		
	protected:
		static constexpr auto buflen = FFTLEN+FFTLEN/2;
		static constexpr auto fftlen = FFTLEN+FFTLEN/2;
		std::atomic<typename tags::detail::u_rms::value_type> m_u;
		std::atomic<typename tags::detail::i_rms::value_type> m_i;
		std::atomic<typename tags::detail::thd_u::value_type> m_thdu;
		std::atomic<typename tags::detail::thd_i::value_type> m_thdi;
	private:
		std::array<std::array<sample_t,buflen>,4> m_buf {{{}}};
		std::atomic<sample_t*> m_su { m_buf[0].data() };
		std::atomic<sample_t*> m_pu { m_buf[1].data() };
		std::atomic<sample_t*> m_si { m_buf[2].data() };
		std::atomic<sample_t*> m_pi { m_buf[3].data() };
	};
};

