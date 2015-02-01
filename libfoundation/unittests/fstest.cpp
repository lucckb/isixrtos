/*
 * =====================================================================================
 *
 *       Filename:  fstest.cpp
 *
 *    Description:  FS test
 *
 *        Version:  1.0
 *        Created:  03/07/2014 09:35:35 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (lb)
 *   Organization: 
 *
 * =====================================================================================
 */
#include <foundation/fs_eeprom.hpp>
#include <foundation/fs_env.hpp>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <foundation/dbglog.h>
#include <time.h>
#include <cstdio>
int main( int argc, const char** /*  argv*/) {
	
	try {
		// EEPROM type EMULATION
		//fnd::fs_eeprom test( 128, 16 , false );
		//FLASH LIKE EMULATION
		fnd::fs_eeprom test( 8, 256 , true );
		fnd::filesystem::fs_env env { test, true };
		if( 1 ) {
			auto m_env = env;
			//static constexpr auto str1 = "Ala ma kota a kot ma ale no i co panie z tego wyniknie to nie wiadomo";
			static constexpr auto str2 = "Walentina to walientina podniebmna mis byly kwiaty dla gagarina"
				"a teraz jest valentina twist A teraz Pan odwiedzi nas przyjedzie do Polski bo wszyscy na niego czekaja";
			dbprintf("String length for str2 %i", std::strlen(str2) );
			static char buf[1222] = { 0 };
			int envret2 = m_env.set( 5 , str2, std::strlen(str2) );
			dbprintf("Set env errno %i", envret2 );
			int envret3 = m_env.get(5, buf, sizeof buf );
			dbprintf("Get env errno %i str: %s", envret3, buf );
		}
		static constexpr auto sx1 = "A teraz bedzie dlugi na kilka chainow przynajmniem moze nie tak calkiem %li";
		char s1[ 512 ];
		std::snprintf(s1, sizeof s1, sx1, time(NULL) );
		static constexpr auto s3 = "Ole olek na prezydenta tylko ty Ole ole wybierzmy przyszlosc oraz styl";
		char s2[ 128 ] = { 0 };
		if( argc == 2 ) {
			const auto ret = env.format();
			dbprintf("Format called!!! errno: %i", ret );
		}
#if 1
		auto r1 = env.set( 0, s1, std::strlen(s1)+1 );
		dbprintf("set R: %i", r1 );
#endif
#if 1
		r1 = env.set( 124, s3, std::strlen(s3)+1 );
		dbprintf("set R: %i", r1 );
#endif
#if 1
		s2[0] = '\0';
		auto r2 = env.get( 0, s2, sizeof s2 );
		dbprintf( "get R: %i [%s]", r2, s2 );
		if( std::strcmp( s2, s1) ) {
			dbprintf(">>>>>>>Test failed s2!=s1");
			return -1;
		} else {
			dbprintf(">>>>>>>Test ok");
		}
#endif
#if 1
		s2[0] = '\0';
		r2 = env.get( 124, s2, sizeof s2 );
		dbprintf( "get R: %i [%s]", r2, s2 );
		if( std::strcmp( s2, s3) ) {
			dbprintf(">>>>>>>Test failed s2!=s3");
			return -1;
		} else {
			dbprintf(">>>>>>>Test ok");
		}
#endif
#if 1
		const auto r3 = env.unset( 124 );
		dbprintf("unset R: %i", r3 );
		r2 = env.get( 124, s2, sizeof s2 );
		if( r2 != fnd::filesystem::fs_env::err_invalid_id )
		{
			dbprintf(">>>>>>>>>>>>>>> Error deleted id found!");
			return -1;
		}
		else
		{
			dbprintf(">>>>>>>>>>>> Delete test passed");
		}
#endif
	}
	catch ( std::exception &e ) {
		std::cerr << "Exc " << e.what() << std::endl;
	}
	return 0;
}
