/*
 * util_ftoa.cpp
 *
 *  Created on: 2 lis 2013
 *      Author: lucck
 */

#include "detail_dtoa.hpp"

extern "C" {

	void fnd_ftoa(float value, char* str, int prec)
	{
		fnd::detail::fptoa(value, str, prec );
	}

}
