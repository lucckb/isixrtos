/*
 * =====================================================================================
 *
 *       Filename:  utility.hpp
 *
 *    Description:  Utility for gms
 *
 *        Version:  1.0
 *        Created:  01.03.2015 22:10:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <cstring>
#include <foundation/utils.h>
#include <gsm/errors.hpp>

namespace gsm_modem {
namespace detail {

	//Strncat utility
	inline void catcstr( char* str1, const char* str2, 
			const char* str3, const char* str4, size_t blen ) 
	{
		std::strncpy( str1, str2 , blen );
		std::strncat( str1, str3, blen-1 );
		std::strncat( str1 ,str4, blen-1 ); 
		str1[blen-1] = '\0';
	}
	//! Cat string and int
	inline int catcstrint( char*str1, const char* str2, int val, size_t blen ) 
	{
		std::strncpy( str1, str2 , blen );
		const auto pi = std::strlen(str1);
		if( blen - pi >= 14 ) {
			fnd::fnd_itoa( str1+pi, val, 1, '0' );
			return error::success;
		} 
		return error::invalid_argument;
	}
}
}
