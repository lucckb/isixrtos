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
#include <cmath>
#include <limits>
#include <type_traits>
#include "dsp_basic_ops.hpp"
/* ------------------------------------------------------------------------- */
namespace dsp {

namespace integer
{
	 template<typename T> inline constexpr int cbits()
	 {
	    return __builtin_log2(std::numeric_limits<T>::max()) + 0.5;
	 }
	 //Floating point type
	 template<typename R, typename T>
	    typename std::enable_if<std::is_floating_point<typename std::complex<T>::value_type >::value, R >::type
	    inline scale(std::complex<T> t)
	    {

	         return t;
	    }
	 template<typename R, typename T>
    	 typename std::enable_if<std::is_floating_point<T>::value, R >::type
	 	    inline scale(T t)
	        {
	 	         return t;
	        }
     //Integral type
     template<typename R, typename T>
	 typename std::enable_if<std::is_integral<typename std::complex<T>::value_type >::value, R >::type
	    inline scale(std::complex<T> t)
	    {
	        return R( cpu::saturated_cast<typename R::value_type>( t.real() >> cbits<typename R::value_type>()),
                      cpu::saturated_cast<typename R::value_type>( t.imag() >> cbits<typename R::value_type>()) 
                    );
	    }
    /*
	 template<typename R, typename T>
	    typename std::enable_if<std::is_integral<T>::value, R >::type
	 	    inline scale(T t)
	        {
	 	        return cpu::saturated_cast<R>(t);
	        }
    */
}
/* ------------------------------------------------------------------------- */
/**
 * DT - data type
 * CT - coefficient type
 * MACT - multiply and accumulate data type
 * N  - decimate length
 */
template<typename DT, typename CT, size_t TAPS, typename ACC = DT>
class fir_decimate
{

public:
	explicit constexpr fir_decimate( const CT * const coefs )
        : m_coefs( coefs ), m_state()
	{       
	}
    //Reset the filter
    void reset()
    {
        m_state.fill( DT() );
        m_last_idx = 0;
    }
    //Insert operator
	void operator()( DT value )
	{
	    m_state[ m_last_idx++ ] = value;
        if( m_last_idx == TAPS ) m_last_idx = 0;
    }
    //Calculate fir operator
    DT operator()() const
    {
          using namespace cpu;
    	  ACC acc {};
          size_t index = m_last_idx;
          for(size_t i = 0; i < TAPS; ++i) 
          {
            index = index != 0 ? index-1 : TAPS-1;
            //Multiply and accumulate
            //acc += m_state[index] * m_coefs[i];
            acc = mac( acc, m_state[index], m_coefs[i] );
          }
          return integer::scale<DT>(acc);
    }
private:
   	const CT * const m_coefs;
    std::array<DT, TAPS> m_state {};
    size_t m_last_idx {};
};

/* ------------------------------------------------------------------------- */
}
/* ------------------------------------------------------------------------- */
#endif /* DSP_FIR_DECIMATE_HPP_ */

/* ------------------------------------------------------------------------- */
