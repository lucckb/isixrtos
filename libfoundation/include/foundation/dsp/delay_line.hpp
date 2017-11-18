/*
 * =====================================================================================
 *
 *       Filename:  delay_line.hpp
 *
 *    Description:  Delay line
 *
 *        Version:  1.0
 *        Created:  18.11.2017 16:38:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <cstddef>
#include <array>

namespace dsp {

	template <typename T, size_t LEN>
	class delay_line {
	public:
		T operator()( T value )
		{
			m_buf[ wri++ ] = value;
			wri %= LEN;
			const auto ret = m_buf[ rdi++ ];
			rdi %= LEN;
			return ret;
		}
	private:
		std::array<T, LEN>  m_buf {{}};
		size_t rdi {}, wri { LEN -1 };
	};

}



