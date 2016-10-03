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

#include "detail/tags.hpp"

namespace emeter {
namespace detail {
	class emeter_phase_impl
	{
	public:
		emeter_phase_impl( emeter_phase_impl& ) = delete;
		emeter_phase_impl& operator=(emeter_phase_impl&) = delete;
		emeter_phase_impl() {
		}
	protected:
		int do_calculate( sample_t* , sample_t* , std::size_t ) noexcept
		{
			return 0;
		}
	};
};
};
