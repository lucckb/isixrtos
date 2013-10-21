/*
 * uc1601_display.cpp
 *
 *  Created on: 20 paź 2013
 *      Author: lucck
 */

#include <foundation/uc1601_display.hpp>
#include <foundation/dbglog.h>

namespace fnd {
namespace lcd {
/* ------------------------------------------------------------------ */
namespace {

	constexpr uint8_t UC1601_WRITE_DATA      = 0x00;
	constexpr uint8_t UC1601_READ_DATA       = 0x00;
	constexpr uint8_t UC1601_GET_STATUS      = 0x00;
	constexpr uint8_t UC1601_SET_CA_LSB      = 0x00;
	constexpr uint8_t UC1601_SET_CA_MSB      = 0x10;
	constexpr uint8_t UC1601_SET_CA_MASK     = 0x0F;
	constexpr uint8_t UC1601_SET_TC          = 0x24;
	constexpr uint8_t UC1601_SET_TC_MASK     = 0x03;
	constexpr uint8_t UC1601_SET_PC          = 0x28;
	constexpr uint8_t UC1601_SET_PC_MASK     = 0x07;
	constexpr uint8_t UC1601_SET_SL          = 0x40;
	constexpr uint8_t UC1601_SET_SL_MASK     = 0x1F;
	constexpr uint8_t UC1601_SET_PA          = 0xB0;
	constexpr uint8_t UC1601_SET_PA_MASK     = 0x0F;
	constexpr uint8_t UC1601_SET_PM          = 0x81;
	constexpr uint8_t UC1601_SET_PM_MASK     = 0xFF;
	constexpr uint8_t UC1601_SET_PD_EN       = 0x85;
	constexpr uint8_t UC1601_SET_PD_DIS      = 0x84;
	constexpr uint8_t UC1601_SET_AC          = 0x88;
	constexpr uint8_t UC1601_SET_AC_MASK     = 0x07;
	constexpr uint8_t UC1601_SET_AC_0        = 0x01;
	constexpr uint8_t UC1601_SET_AC_1        = 0x02;
	constexpr uint8_t UC1601_SET_AC_2        = 0x04;
	constexpr uint8_t UC1601_SET_LC3         = 0xA0;
	constexpr uint8_t UC1601_SET_FR_80       = 0xA0;
	constexpr uint8_t UC1601_SET_FR_100      = 0xA1;
	constexpr uint8_t UC1601_SET_DC1         = 0xA4;
	constexpr uint8_t UC1601_SET_DC1_EN      = 0xA5;
	constexpr uint8_t UC1601_SET_DC0         = 0xA6;
	constexpr uint8_t UC1601_SET_DC0_EN      = 0xA7;
	constexpr uint8_t UC1601_SET_DC2         = 0xAE;
	constexpr uint8_t UC1601_SET_DC2_EN      = 0xAF;
	constexpr uint8_t UC1601_SET_LC21        = 0xC0;
	constexpr uint8_t UC1601_SET_LC21_MASK   = 0x06;
	constexpr uint8_t UC1601_SET_SC          = 0xE2;
	constexpr uint8_t UC1601_SET_NOP         = 0xE3;
	constexpr uint8_t UC1601_SET_BR          = 0xE8;
	constexpr uint8_t UC1601_SET_BR_6        = 0xE8;
	constexpr uint8_t UC1601_SET_BR_7        = 0xE9;
	constexpr uint8_t UC1601_SET_BR_8        = 0xEA;
	constexpr uint8_t UC1601_SET_BR_9        = 0xEB;
	constexpr uint8_t UC1601_SET_CEN         = 0xF1;
	constexpr uint8_t UC1601_SET_CEN_MASK    = 0x7F;
	constexpr uint8_t UC1601_SET_DST         = 0xF2;
	constexpr uint8_t UC1601_SET_DST_MASK    = 0x7F;
	constexpr uint8_t UC1601_SET_DEN         = 0xF3;
	constexpr uint8_t UC1601_SET_DEN_MASK    = 0x7F;
	constexpr uint8_t UC1601_READ_DATA_CMD   = 0xFF;
	constexpr uint8_t UC1601_READ_DATA_MASK  = 0xFF;

}
/* ------------------------------------------------------------------ */
//Set address
int uc1601_display::address_set( uint8_t pa, uint8_t ca )
{
	int ret = 0;
	do
	{
		//
		//Step 1 Set Page Address
		//
		ret = bus.command((pa & UC1601_SET_PA_MASK) | UC1601_SET_PA);
		if( ret != 0 ) break;
		//
		//Step 2 Set Column Address MSB
		//
		ret = bus.command(((ca>>4) & UC1601_SET_CA_MASK) | UC1601_SET_CA_MSB);
		if( ret != 0 ) break;
		//
		//Step 3 Set Column Address LSB
		//
		ret = bus.command(((ca) & UC1601_SET_CA_MASK) | UC1601_SET_CA_LSB);
		if( ret != 0 ) break;
	}
	while(0);
	return ret;
}
/* ------------------------------------------------------------------ */
//Constructor
uc1601_display::uc1601_display( uc1601_bus &bus_, int rows )
	: bus(bus_)
{
	do
	{
		bus.mdelay(30);
		//Step 1 Set BR
		m_error = bus.command(UC1601_SET_BR_8);
		if( m_error ) break;
		bus.mdelay(10);
		//Step 2 Set PM
		m_error = bus.command( UC1601_SET_PM, 0xB0 );
		if( m_error ) break;
		bus.mdelay(10);
		//Step 3 set LCD Mapping Control
		m_error = bus.command( UC1601_SET_LC21 + 4 );
		if( m_error ) break;
		bus.mdelay(10);
		//Step 4 set com en
		m_error = bus.command( UC1601_SET_CEN, rows - 1 );
		if( m_error ) break;
		bus.mdelay(10);
		//Step 5 Set Display Enable
		m_error = bus.command( UC1601_SET_DC2_EN );
		if( m_error ) break;
		bus.mdelay(10);
		//After init clear the display
		clear();
		if( m_error ) break;
	}
	while(0);
}
/* ------------------------------------------------------------------ */
//Clear the display
void uc1601_display::clear()
{
	do
	{
		m_error = address_set( 0, 0 );
		if( m_error ) break;
		uint8_t buf = 0;
		for (int i = 0; i < (132*32)/8; i++)
		{
			m_error = bus.data_wr(&buf, sizeof(buf) );
		    if( m_error ) break;
		}
	} while(0);
}
/* ------------------------------------------------------------------ */
//Put char
void uc1601_display::putchar( char c )
{
	dbprintf("putchar");
	m_error = address_set( 0, 0 );
	if(!m_error)
	{
		uint8_t buf[] = { 0xFF };
		for (int i = 0; i < (132*32)/8; i++)
		{
			m_error = bus.data_wr(buf, sizeof(buf) );
			if(m_error) return;
		}
	}
}
/* ------------------------------------------------------------------ */
// Set cursor position
void uc1601_display::setpos( int x, int y )
{

}
/* ------------------------------------------------------------------ */
} /* namespace lcd */
} /* namespace fnd */
/* ------------------------------------------------------------------ */
