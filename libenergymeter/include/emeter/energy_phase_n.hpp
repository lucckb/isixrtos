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
#include "types.hpp"

namespace emeter {

	//! Calculate energy in the Nth phase
	template <std::size_t FFTLEN>
	class energy_phase_n {
		static constexpr auto BUFLEN = FFTLEN+FFTLEN/2;
	public:
		energy_phase_n( energy_phase_n& ) = delete;
		energy_phase_n& operator=(energy_phase_n&) = delete;
	private:
		std::array<std::array<sample_t,BUFLEN>,2> m_buf {{{}}};
	};
};

