/*
 * =====================================================================================
 *
 *       Filename:  lexical_cast.hpp
 *
 *    Description:  To string conversion routines only for C++
 *
 *        Version:  1.0
 *        Created:  12.06.2017 20:23:28
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#ifdef __cplusplus


#include <string>
#include <foundation/utils.h>

namespace fnd {

	template<typename Target, typename Source>
		Target lexical_cast( const  Source& arg );

	template <> std::string
		lexical_cast<std::string, int>( const int& arg )
	{
		char buf[12] {};
		fnd::fnd_itoa( buf, arg, 1, ' ' );
		return buf;
	}
	template <> std::string
		lexical_cast<std::string, unsigned>( const unsigned& arg )
	{
		char buf[12] {};
		fnd::fnd_uitoa( buf, arg, 1, ' ', 10 );
		return buf;
	}
	template <> std::string
		lexical_cast<std::string, unsigned long long>( const unsigned long long& arg )
	{
		char buf[22] {};
		fnd::fnd_ulltoa( buf, arg, 1, ' ', 10 );
		return buf;
	}
	template <> std::string
		lexical_cast<std::string, float>( const float& arg )
	{
		char buf[33] {};
		fnd::fnd_ftoa( arg, buf, 9 );
		return buf;
	}
	template <> std::string
		lexical_cast<std::string, double>( const double& arg )
	{
		char buf[33] {};
		fnd::fnd_dtoa( arg, buf, 9 );
		return buf;
	}
}
#endif

