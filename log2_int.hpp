/*
 * log2_int.hpp
 *
 *  Created on: 16-05-2013
 *      Author: lucck
 */

#ifndef DSP_LOG2_INT_HPP_
#define DSP_LOG2_INT_HPP_

/* ------------------------------------------------------------------------- */
namespace dsp {
namespace integer {
/* ------------------------------------------------------------------------- */
/* Taylor series log in range 1-2 */
    template <typename N, unsigned SC>
    inline unsigned log_1_2( N x )
    {
		if( x == SC ) return 0;
		const  N x1 = ( (x-N(SC)) * N(SC) ) / ( x+N(SC) );
		const  N x2 = N( N(x1)  * N(x1)) / N(SC);
		const  N x3 = N( N(x2)  * N(x1)) / N(SC);
		const  N x5 = N( N(x3)  * N(x2)) / N(SC);
		const  N x7 = N( N(x5)  * N(x2)) / N(SC);
		return N(2) * N( x1 + x3/N(3) + x5/N(5) + x7/N(7) );
    }
	/* Log2 without partials */
	inline unsigned log2c( unsigned x )
	{
	  return (8*sizeof (unsigned ) - __builtin_clz(x))-1;
	}

	inline unsigned log2c(unsigned long long x)
	{
	  return (8*sizeof (unsigned long long) - __builtin_clzll(x))-1;
	}

	/* Log2 partial version */
	template <typename N, unsigned SC>
	inline unsigned log2_1_n( N x )
	{
		constexpr N LE2L2 = (std::log(10)/std::log2(10)) * double(SC);
		const auto l2c = log2c( N(x / SC) );
		const N lr = x / ( N(1)<< l2c );
		const N ler = log_1_2<N,SC>( lr );
		const N l2r = N(ler*SC) / LE2L2;
		return l2r + l2c*N(SC);
	}
	 template <typename N, unsigned SC>
	 inline int log2_0_1( N x )
	 {
	   constexpr N LN2SC = std::log2( SC ) * double(SC);
	   return log2_1_n<N,SC>( x * N(SC) )  - LN2SC;
	 }

/* ------------------------------------------------------------------------- */
}}
/* ------------------------------------------------------------------------- */

#endif /* LOG2_INT_HPP_ */
