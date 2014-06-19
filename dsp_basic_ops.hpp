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
#ifndef __arm__
    /* Saturated cast between values */
    template <typename RetT, typename ValT >
        inline RetT saturated_cast( ValT val )
        {
            return (val>std::numeric_limits<RetT>::max() )?
            (std::numeric_limits<RetT>::max()):
            ( (std::numeric_limits<RetT>::min()>val)?(std::numeric_limits<RetT>::min()):(val) );
        }
#else
	namespace {
	namespace detail {
		//! Saturated cast signed
		inline __attribute__((always_inline)) 
			int32_t ssat( int32_t val, int bits ) 
		{
			int32_t out;
			asm volatile("ssat %0, %1, %2" : "=r" (out) : "I" (bits), "r" (val) ) ;
			return out;
		}
		//! Saturated cast signed
		inline __attribute__((always_inline)) 
			int32_t usat( int32_t val, int bits ) 
		{
			int32_t out;
			asm volatile("usat %0, %1, %2" : "=r" (out) : "I" (bits), "r" (val) ) ;
			return out;
		}
	}}
    /* Saturated cast between values */
    template <typename RetT, typename ValT >
        inline RetT saturated_cast( ValT val )
	{
		if( std::numeric_limits<RetT>::is_signed ) {
			return detail::ssat( val, std::numeric_limits<RetT>::digits+1 );
		} else {
			return detail::usat( val, std::numeric_limits<RetT>::digits );
		}
	}
#endif
/* ------------------------------------------------------------------------- */
}}

/* ------------------------------------------------------------------------- */

#endif /* DSP_BASIC_OPS_HPP_ */
/* ------------------------------------------------------------------------- */
