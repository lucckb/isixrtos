/*
 * nco_mixer.hpp
 *
 *  Created on: 02-04-2013
 *      Author: lucck
 */

#ifndef NCO_MIXER_HPP_
#define NCO_MIXER_HPP_
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
	std::complex<R> operator()(R signal, P phz_inc )
	{
		const std::complex<R> ret(
				(signal*icosinus(m_vco_phz)) >> SHIFT ,
				(signal*isinus(m_vco_phz))   >> SHIFT
			);
		m_vco_phz += phz_inc;
		if( m_vco_phz > isinmax() )		//handle 2 Pi wrap around
			m_vco_phz -= isinmax();
		return ret;
	}
	constexpr R max_angle( )
	{
		return dsp::integer::trig::sin_arg_max<R, SSIN_SIZE>();
	}
private:
	inline int isinus( short value )
	{
		return dsp::integer::trig::sin<R, SSIN_SIZE>( value );
	}
	constexpr short isinmax( )
	{
		return dsp::integer::trig::sin_arg_max<R, SSIN_SIZE>();
	}
	inline int icosinus( short value )
	{
		return dsp::integer::trig::sin<R, SSIN_SIZE>( isinmax()/4 + value );
	}
private:
	P m_vco_phz {};
};


/* ------------------------------------------------------------------------- */
}

/* ------------------------------------------------------------------------- */
#endif /* NCO_MIXER_HPP_ */
