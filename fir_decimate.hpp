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
/* ------------------------------------------------------------------------- */
namespace dsp {
/* ------------------------------------------------------------------------- */
namespace cpu
{
	 template<typename R, typename T> inline void mac( R &acc, T x, T y )
     {
    	 acc += x * y;
     }
     template< typename T> inline T sat( T, int pos )
     {

     }
     template<typename T> inline void mac( std::complex<T> &acc, std::complex<T> x , T y )
     {
    	 acc += x * y;
     }
}
namespace integer
{
	 template<typename T> inline constexpr int cbits()
	 {
	    return std::log2(std::numeric_limits<T>::max()) + 0.5;
	 }
	 //Floating point type
	 template<typename T>
	 typename std::enable_if<std::is_floating_point<typename std::complex<T>::value_type >::value, std::complex<T> >::type
	    inline scale(std::complex<T> t)
	 {

	     return t;
	 }
	 template<typename T>
	 typename std::enable_if<std::is_floating_point<T>::value, T >::type
	 	 inline scale(T t)
	 {

	 	 return t;
	 }
	 //Integral type
	 template<typename T>
	 typename std::enable_if<std::is_integral<typename std::complex<T>::value_type >::value, std::complex<T> >::type
	 	  inline scale(std::complex<T> t)
	 {
		 return  std::complex<T>(
			cpu::sat(t.real()>>cbits<T>(),  cbits<T>() + 1),
			cpu::sat(t.imag()>>cbits<T>(),  cbits<T>() + 1)
		 );
	 }
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
    DT operator()()
    {
          using namespace cpu;
    	  ACC acc {};
          size_t index = m_last_idx;
          for(size_t i = 0; i < TAPS; ++i) 
          {
            index = index != 0 ? index-1 : TAPS-1;
            //Multiply and accumulate
            //acc += m_state[index] * m_coefs[i];
            mac( acc, m_state[index], m_coefs[i] );
          }
          return integer::scale(acc);
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
