/*
 * =====================================================================================
 *
 *       Filename:  test_energy_phase_n.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03.04.2016 12:13:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <emeter/energy_meter.hpp>
#include <gtest/gtest.h>

/** Compare simple buffer management */
TEST( energy_phase_n, buffer_swap_simple )
{
	emeter::energy_phase_n<256> o;
	auto p1 = o.sample_voltage_begin();
	ASSERT_TRUE( p1 );
	auto p2 = o.sample_current_begin();
	ASSERT_TRUE( p2 );
	ASSERT_EQ( o.sample_current_end(), 0 );
	ASSERT_EQ( o.sample_voltage_end(), 0 );
	ASSERT_EQ( o.calculate(), 0 );
	ASSERT_NE( p1, p2 );
}

/** Test multi loop  */
TEST( energy_phase_n, buffer_swap_match )
{
	emeter::energy_phase_n<256> o;
	for( int i=0; i<64; ++i )
	{
		auto p1 = o.sample_voltage_begin();
		ASSERT_TRUE( p1 );
		auto p2 = o.sample_current_begin();
		ASSERT_TRUE( p2 );
		ASSERT_EQ( o.sample_current_end(), 0 );
		ASSERT_EQ( o.sample_voltage_end(), 0 );
		ASSERT_EQ( o.calculate(), 0 );
		ASSERT_NE( p1, p2 );
	}
}

/** Test multi loop  */
TEST( energy_phase_n, buffer_swap_match2 )
{
	emeter::energy_phase_n<256> o;
	auto v1 = o.sample_voltage_begin();
	auto i1 = o.sample_current_begin();
	o.sample_current_end();
	o.sample_voltage_end();
	auto v2 = o.sample_voltage_begin();
	auto i2 = o.sample_current_begin();
	ASSERT_TRUE( v1 );
	ASSERT_TRUE( v2 );
	ASSERT_TRUE( i1 );
	ASSERT_TRUE( i2 );
	ASSERT_NE( v1, v2 );
	ASSERT_NE( i1, i2 );
	ASSERT_EQ( o.sample_voltage_end(), 0 );
	ASSERT_EQ( o.sample_current_end(), 0 );
	auto v3 = o.sample_voltage_begin();
	auto i3 = o.sample_current_begin();
	ASSERT_FALSE( v3 );
	ASSERT_FALSE( i3 );
	ASSERT_EQ( o.calculate(), 0 );
	ASSERT_TRUE( o.sample_voltage_begin() );
	ASSERT_TRUE( o.sample_current_begin() );
	ASSERT_EQ( o.sample_voltage_end(), 0 );
	ASSERT_EQ( o.sample_current_end(), 0 );
	ASSERT_EQ( o.calculate(), 0 );
}



