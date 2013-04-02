/*
 * nco_mixer.hpp
 *
 *  Created on: 02-04-2013
 *      Author: lucck
 */

#ifndef DSP_NCO_MIXER_HPP_
#define DSP_NCO_MIXER_HPP_
/* ------------------------------------------------------------------------- */
#include <cstddef>
#include <complex>
#include "array_sinus.hpp"
/* ------------------------------------------------------------------------- */
namespace dsp {
/* ------------------------------------------------------------------------- */
template <typename R, typename P, int SHIFT, std::size_t SSIN_SIZE > class nco_mixer
{
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
		m_vco_phz += phz_inc;
		if( m_vco_phz > max_angle() )		//handle 2 Pi wrap around
			m_vco_phz -= max_angle();
		return ret;
	}
private:
	inline int isinus( short value )
	{
		return dsp::integer::trig::sin<R, SSIN_SIZE>( value );
	}
	inline int icosinus( short value )
	{
		return dsp::integer::trig::sin<R, SSIN_SIZE>( max_angle()/4 + value );
	}
private:
	P m_vco_phz {};
};


/* ------------------------------------------------------------------------- */
}

/* ------------------------------------------------------------------------- */
#endif /* NCO_MIXER_HPP_ */
