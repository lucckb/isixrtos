/*
 * dsp_basic_ops.hpp
 *
 *  Created on: 03-05-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------------- */
#ifndef DSP_BASIC_OPS_HPP_
#define DSP_BASIC_OPS_HPP_

/* ------------------------------------------------------------------------- */
namespace dsp {
namespace cpu {
/* ------------------------------------------------------------------------- */

     /* Do multply and accumulate real */
	 template<typename R, typename T> inline R mac( R acc, T x, T y )
     {
    	 acc += x * y;
    	 return acc;
     }
	 /* Do multiply and acumulate imag */
     template<typename R, typename T> inline std::complex<R> mac( std::complex<R> acc, std::complex<T> x , T y )
     {
    	 return std::complex<R>( mac(acc.real(), x.real(), y), mac(acc.imag(), x.imag(), y) );
     }
    /* Saturated cast between values */
    template <typename RetT, typename ValT >
        constexpr inline RetT saturated_cast( ValT val )
        {
            return (val>std::numeric_limits<RetT>::max() )?
            (std::numeric_limits<RetT>::max()):
            ( (std::numeric_limits<RetT>::min()>val)?(std::numeric_limits<RetT>::min()):(val) );
        }

/* ------------------------------------------------------------------------- */
}}

/* ------------------------------------------------------------------------- */

#endif /* DSP_BASIC_OPS_HPP_ */
/* ------------------------------------------------------------------------- */
