/*
 * sqrt_int.hpp
 *
 *  Created on: 25-04-2013
 *      Author: lucck
 */

#pragma once


namespace dsp {
namespace integer {

	//Integer SQRT
	unsigned int inline sqrt(unsigned int n)
	{
		unsigned int c = 0x8000;
		unsigned int g = 0x8000;
		for(;;)
		{
			if(g*g > n) g ^= c;
			c >>= 1;
			if(c == 0) return g;
			g |= c;
		}
	}

}}

