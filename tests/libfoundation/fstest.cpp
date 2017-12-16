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
#include <foundation/sys/dbglog.h>
#include <time.h>
#include <unistd.h>
#include <cstdio>



int _fstest_main_( int argc, const char** /*  argv*/) 
{
	
	try {
		// EEPROM type EMULATION
		//fnd::fs_eeprom test( 128, 16 , false );
		//FLASH LIKE EMULATION
		fnd::fs_eeprom test( 128, 4096 , true, "/tmp/boff_fstestenv.bin");
	 	fnd::filesystem::fs_env env { test, true, 8 };
		if( 1 ) {	//Very short data write test
			char buf[16] {};
			int ret = env.set(0, "luco", 5 );
			dbprintf("Ret set %i",  ret );
			ret = env.get( 0, buf, sizeof buf );
			dbprintf("Ret %i %s", ret, buf );
		}
		if( 1 ) 
		{
			auto &m_env = env;
			//static constexpr auto str1 = "Ala ma kota a kot ma ale no i co panie z tego wyniknie to nie wiadomo";
			static constexpr auto str2 = "Walentina to walientina podniebmna mis byly kwiaty dla gagarina"
				"a teraz jest valentina twist A teraz Pan odwiedzi nas przyjedzie do Polski bo wszyscy na niego czekaja";
			static constexpr auto strs2 = "to jest test krotki a teraz bedzie troche dluzszy ale nie za dlugi";
			dbprintf("String length for str2 %i", std::strlen(str2)+1);
			static char buf[1222] = { 0 };
			int envret2 = m_env.set( 5 , str2, std::strlen(str2)+1);
			dbprintf("Set env errno %i", envret2 );
			int envret3 = m_env.get(5, buf, sizeof buf );
			dbprintf("Get env errno %i str: %s", envret3, buf );
			//! Small shrink test
			envret2 = m_env.set( 5 , strs2, std::strlen(strs2) + 1);
			dbprintf("Set env errno %i", envret2 );
			envret3 = m_env.get(5, buf, sizeof buf );
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
#if 0
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

#define fatal(x) \
	do { if( !(x) ) { \
		fprintf(stderr,"Test error at %s:%i\n", __FILE__,__LINE__); \
		exit(-1); } \
	} while(0)


// Unit test for memtype
void fs_tunits( fnd::filesystem::fs_env& env ) 
{
	int err;
	char ibuf[256];
	using e =  fnd::filesystem::fs_env;
	//! Try to set non existing item first
	err = env.get( 1, ibuf, sizeof ibuf );
	fatal( err == e::err_invalid_id );
}



int fstest_main( int, const char** ) 
{
	static constexpr auto filename_eep = "/tmp/boff_fsenvtest_storage_eep.bin";
	static constexpr auto filename_fla = "/tmp/boff_fsenvtest_storage_fla.bin";
	try 
	{
		std::cout << "Trying to flash mode " << std::endl;
		//Flash emulation
		unlink( filename_fla );
		{
			fnd::fs_eeprom test( 128, 512 , true, filename_fla );
			fnd::filesystem::fs_env env { test, true, 8 };
			fs_tunits( env );
		}
		std::cout << "Trying to eeprom mode " << std::endl;
		//EEPROM emulation
		unlink( filename_eep );
		{
			fnd::fs_eeprom test( 128, 16 , false, filename_eep );
			fnd::filesystem::fs_env env { test, true, 8 };
			fs_tunits( env );
		}
	} 
	catch( const std::exception &e )  
	{
		std::cerr << "Exc " << e.what() << std::endl;
		return -1;
	}
	return 0;
}



