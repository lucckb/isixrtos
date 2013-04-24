/*
 * fir_decimate.hpp
 *
 *  Created on: 22-04-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------------- */
#ifndef DSP_FIR_DECIMATE_HPP_
#define DSP_FIR_DECIMATE_HPP_
/* ------------------------------------------------------------------------- */
#include <array>

/* ------------------------------------------------------------------------- */
namespace dsp {
/* ------------------------------------------------------------------------- */
/**
 * DT - data type
 * CT - coefficient type
 * N  - decimate length
 */
template<typename DT, typename CT, size_t TAPS> 
class fir_decimate
{
public:
	explicit constexpr fir_decimate( const CT * const coefs )
        : m_coefs( coefs )
	{       
	}
    //Insert operator
	void operator()( DT value )
	{
	    m_state[ m_last_idx++ ] = value;
        if( m_last_idx == TAPS ) m_last_idx = 0;
    }
    //Calculate fir operator
    DT operator()()
    {
          DT acc = 0;
          size_t index = m_last_idx;
          for(size_t i = 0; i < TAPS; ++i) 
          {
            index = index != 0 ? index-1 : TAPS-1;
            acc += m_state[index] * m_coefs[i];
          }
          return acc;
    }
private:
    std::array<DT, TAPS> m_state {};
	const CT * const m_coefs {};
    size_t m_last_idx {};
};

/* ------------------------------------------------------------------------- */
}
/* ------------------------------------------------------------------------- */
#endif /* DSP_FIR_DECIMATE_HPP_ */

/* ------------------------------------------------------------------------- */
