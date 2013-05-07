/*
 * nco_mixer.hpp
 *
 *  Created on: 02-04-2013
 *      Author: lucck
 */

#ifndef DSP_NCO_MIXER_HPP_
#define DSP_NCO_MIXER_HPP_
/* ------------------------------------------------------------------------- */
#include <cstdlib>
#include <cmath>
#include <cstddef>
#include <complex>
#include <limits>
#include "array_sinus.hpp"
/* ------------------------------------------------------------------------- */
namespace dsp {
/* ------------------------------------------------------------------------- */
template <typename R, typename P, std::size_t SSIN_SIZE, P PI2 > class nco_mixer
{
private:
	 template<typename T> static constexpr int cbits()
	 {
		 return __builtin_log2(std::numeric_limits<T>::max()) + 0.5;
	 }
	 static const int SHIFT = cbits<R>();
public:
	constexpr R max_angle( )
	{
		return dsp::integer::trig::sin_arg_max<R, SSIN_SIZE>();
	}
	std::complex<R> operator()(R signal, P phz_inc )
	{
		const std::complex<R> ret(
				(signal*icosinus(m_vco_phz)) >> SHIFT ,
				(signal*isinus(m_vco_phz))   >> SHIFT
			);
		const auto ddiv = std::div( phz_inc * max_angle(), PI2 );
		m_vco_phz_rem  += ddiv.rem;
		const auto fdiv = m_vco_phz_rem/PI2;
		m_vco_phz += ddiv.quot + fdiv;
		if( fdiv ) m_vco_phz_rem %= PI2;

		if( m_vco_phz > max_angle() )		//handle 2 Pi wrap around
			m_vco_phz -= max_angle();
		return ret;
	}
private:
	inline int isinus( P value )
	{
		return dsp::integer::trig::sin<R, SSIN_SIZE>( value );
	}
	inline int icosinus( P value )
	{
		return dsp::integer::trig::sin<R, SSIN_SIZE>( max_angle()/4 + value );
	}
private:
	P m_vco_phz {};
	P m_vco_phz_rem {};
};


/* ------------------------------------------------------------------------- */
}

/* ------------------------------------------------------------------------- */
#endif /* NCO_MIXER_HPP_ */
