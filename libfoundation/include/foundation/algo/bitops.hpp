/*
 * bitops.hpp
 *
 *  Created on: 14-02-2012
 *      Author: lucck
 */
 
#ifndef BITOPS_HPP_
#define BITOPS_HPP_
 
#ifdef __cplusplus
namespace fnd
{
#endif
 
static inline unsigned ones32(register unsigned int x)
{
        /* 32-bit recursive reduction using SWAR...
	   but first step is mapping 2-bit values
	   into sum of 2 1-bit values in sneaky way
	*/
        x -= ((x >> 1) & 0x55555555);
        x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
        x = (((x >> 4) + x) & 0x0f0f0f0f);
        x += (x >> 8);
        x += (x >> 16);
        return(x & 0x0000003f);
}
 
#ifdef __cplusplus
}
#endif
 
#endif /* BITOPS_HPP_ */
 
