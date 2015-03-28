/*
 * =====================================================================================
 *
 *       Filename:  errno_test.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  25.03.2015 16:44:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck
 *   Organization:  
 *
 * =====================================================================================
 */
#include <isix.h>
#include <cerrno>
#include <qunit.hpp>
#include "errno_test.hpp"
#include <foundation/dbglog.h>

namespace tests 
{
	
	namespace 
	{
		constexpr auto stack_size = 256;
		constexpr auto task_prio = 3;
		void errno_thread( void* thrinfo ) 
		{
			int& err = *static_cast<int*>(thrinfo);
			for( int i=0;i<16;++i )
			{
				errno = err;
			}
			isix_wait_ms(1);
			err = errno;
		}
	}

	//!Errno test run 
	void errno_threadsafe::run()
	{
		int err[4] { 5, 10, 15, 20 };
		static constexpr int except[4] { 5, 10, 15, 20 };
		errno = 50;
		for( int i=0; i<4; ++i ) 
		{
			auto thr = isix_task_create( errno_thread, &err[i],
					stack_size, task_prio, isix_task_flag_newlib );
			QUNIT_IS_NOT_EQUAL( thr, nullptr );
		}
		isix_wait_ms(10);
		for( int i=0; i<4; ++i ) {
			QUNIT_IS_EQUAL( err[i], except[i] );
		}
		QUNIT_IS_EQUAL( errno, 50 );
	}
}
