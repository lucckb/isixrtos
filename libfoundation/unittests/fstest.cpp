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
int main( int argc, const char** argv) {
	
	try {
		// EEPROM type EMULATION
		fnd::fs_eeprom test( 128, 16 , false );
		//FLASH LIKE EMULATION
		//fnd::fs_eeprom test( 8, 128 , true );
		fnd::filesystem::fs_env env { test };
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
#if 0
		r1 = env.set( 124, s3, std::strlen(s3)+1 );
		dbprintf("set R: %i", r1 );
#endif
#if 1
		s2[0] = '\0';
		auto r2 = env.get( 0, s2, sizeof s2 );
		dbprintf( "get R: %i [%s]", r2, s2 );
#endif
#if 1
		s2[0] = '\0';
		r2 = env.get( 124, s2, sizeof s2 );
		dbprintf( "get R: %i [%s]", r2, s2 );
#endif
#if 0
		const auto r3 = env.unset( 124 );
		dbprintf("unset R: %i", r3 );
#endif
	}
	catch ( std::exception &e ) {
		std::cerr << "Exc " << e.what() << std::endl;
	}
	return 0;
}
