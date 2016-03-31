/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  Main emtest
 *
 *        Version:  1.0
 *        Created:  31.03.2016 21:37:44
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <iostream>
#include <emeter/energy_meter.hpp>
#include <gtest/gtest.h>

TEST( is_test, test )
{
	EXPECT_FALSE( false );
	EXPECT_FALSE( true );
}

int main( int argc, char** argv ) {
	::testing::InitGoogleTest( &argc, argv );
	return RUN_ALL_TESTS();
}


