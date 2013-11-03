/*
 * uc1601_display.cpp
 *
 *  Created on: 20 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <foundation/uc1601_display.hpp>
#include <foundation/lcd_font.hpp>
#include <foundation/lcd_bitmap.hpp>
/* ------------------------------------------------------------------ */
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
	constexpr uint8_t UC1601_SET_DC2_DEN     = 0xAE;
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
	//For rects defs
	constexpr auto max_pattern = 32;
	constexpr uint8_t empty_line[max_pattern] {};
	constexpr uint8_t top_line[max_pattern]
	                           	   { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
									 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
									 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
									 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
	constexpr uint8_t bottom_line[max_pattern]
	                                  { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
									 	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
									 	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
									 	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
	constexpr uint8_t full_line[max_pattern]
	                                { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
									  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
									  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
									  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
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
	m_ca = ca;
	m_pa = pa;
	return ret;
}
/* ------------------------------------------------------------------ */
//Constructor
uc1601_display::uc1601_display( uc1601_bus &bus_, uint8_t cols, uint8_t rows  )
	: bus(bus_), m_cols(cols), m_rows(rows)
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
/** Destructor */
uc1601_display::~uc1601_display()
{
	//Display reset
	 bus.command(UC1601_SET_DC2_DEN);
}
/* ------------------------------------------------------------------ */
//Clear the display
int uc1601_display::clear()
{
	do
	{
		m_error = address_set( 0, 0 );
		if( m_error ) break;
		uint8_t buf = 0;
		for (int i = 0; i < (uc1601_cols*m_rows)/8; i++)
		{
			m_error = bus.data_wr(&buf, sizeof(buf) );
		    if( m_error ) break;
		}
	} while(0);
	return m_error;
}
/* ------------------------------------------------------------------ */
//Put char
int uc1601_display::putc( char ch )
{
	do {
		if( m_font == nullptr )
		{
			m_error = ERR_MISSING_FONT;
			break;
		}
		if( (ch < m_font->first_char || ch > m_font->last_char) && ch!=' ' )
		{
			m_error = ERR_NO_CHAR;
			break;
		}
		const auto cho = ch - m_font->first_char;
		const auto width = (ch!=' ')?(m_font->chr_desc[int(cho)].width):(m_font->spc_width);
		const auto offs = m_font->chr_desc[int(cho)].offset;
		auto bmp_ptr = (ch!=' ')?(&m_font->bmp[offs]):(empty_line);
		const auto saved_pa = m_pa;
		if( m_ca + width + 1 > m_cols )
		{
			m_error = ERR_OUT_RANGE;
			break;
		}
		for(size_t w=0; w<m_font->height;w+=8)
		{
			if( m_error ) break;
			m_error = bus.data_wr(bmp_ptr, width );
			if( m_error ) break;
			m_error = bus.data_wr(empty_line, 1 );
			if( m_error ) break;
			bmp_ptr += width;
			m_error = address_set( m_pa + 1, m_ca );
			if( m_error ) break;
		}
		if( m_error ) break;
		m_error = address_set( saved_pa, m_ca + width + 1 );
	} while(0);
	return m_error;
}
/* ------------------------------------------------------------------ */
// Set cursor position
int uc1601_display::setpos( int x, int y )
{
	do
	{
		if( x > m_cols || y > m_rows )
		{
			m_error = ERR_OUT_RANGE;
			break;
		}
		if( y % 8 )
		{
			m_error = ERR_ALIGN;
			break;
		}
		m_error = address_set( y/8, x );
	}
	while(0);
	return m_error;
}
/* ------------------------------------------------------------------ */
//draw box arround the area
int uc1601_display::box( int x1, int y1, int cx, int cy, box_t type )
{
	const auto save_pa = m_pa;
	const auto save_ca = m_ca;
	do
	{
		if( x1 > m_cols || y1 > m_rows || (x1+cx)>m_cols || (y1+cy)>m_rows )
		{
			m_error = ERR_OUT_RANGE;
			break;
		}
		if( y1 % 8 || cy %8 )
		{
			m_error = ERR_ALIGN;
			break;
		}
		for( int w=0; w<cy; w+=8 )
		{
			m_error = address_set( (y1+w)/8, x1 );
			if( m_error ) break;
			const uint8_t *pattern;
			if( type == box_t::frame )
			{
				if( w == 0 ) pattern = top_line;
				else if( w == cy-8 ) pattern = bottom_line;
				else pattern = empty_line;
			}
			else if( type == box_t::fill )
			{
				pattern = full_line;
			}
			else
			{
				pattern = empty_line;
			}
			for(int rx=cx; rx>0; rx-=cx>max_pattern?max_pattern:cx )
			{
				if( m_error ) break;
				m_error = bus.data_wr(pattern, rx>max_pattern?max_pattern:rx );
				if( m_error ) break;
			}
		}
		if( type == box_t::frame )
		for( int w=0; w<cy; w+=8 )
		{
			if( m_error ) break;
			//Left frame
			m_error = address_set( (y1+w)/8, x1 );
			if( m_error ) break;
			m_error = bus.data_wr(full_line, 1 );
			if( m_error ) break;
			//Right frame
			m_error = address_set( (y1+w)/8, x1+cx );
			if( m_error ) break;
			m_error = bus.data_wr(full_line, 1 );
			if( m_error ) break;
		}
	}
	while(0);
	m_pa = save_pa;
	m_ca = save_ca;
	return m_error;
}
/* ------------------------------------------------------------------ */
//Display a progress bar
int uc1601_display::progress_bar(int x1, int y1, int cx, int cy, int value, int max )
{
	do
	{
		const int cx1 = ( cx * value ) / max;
		const int cx2 = cx - cx1;
		box( x1, y1, cx1, cy, box_t::fill );
		if( m_error ) break;
		box( x1+cx1, y1, cx2, cy, box_t::frame );
		if( m_error ) break;
	}
	while(0);
	return m_error;
}

/* ------------------------------------------------------------------ */
//Show the icon on screen
int uc1601_display::show_icon( int x1, int y1, const icon_t *icon )
{
	const auto save_pa = m_pa;
	const auto save_ca = m_ca;
	do
	{
		if( icon == nullptr )
		{
			m_error = ERR_INVALID_ARG;
			break;
		}
		if( y1 % 8 )
		{
			m_error = ERR_ALIGN;
			break;
		}
		if( y1/8 + icon->pg_width > m_rows/8  ||  x1+icon->height > m_cols )
		{
			m_error = ERR_OUT_RANGE;
			break;
		}
		for(unsigned p=0; p<icon->pg_width; ++p )
		{
			m_error = address_set( y1/8+p, x1 );
			if( m_error ) break;
			m_error = bus.data_wr( &icon->data[p*icon->height], icon->height );
			if( m_error ) break;
		}
	}
	while(0);
	m_pa = save_pa;
	m_ca = save_ca;
	return m_error;
}
/* ------------------------------------------------------------------ */
//Clear to end of line
int uc1601_display::endl()
{
	do {

		if( m_font == nullptr )
		{
			m_error = ERR_MISSING_FONT;
			break;
		}
		box( m_ca, m_pa*8, m_cols-m_ca, m_font->height+8-m_font->height%8 - m_pa*8 );
	} while(0);
	return m_error;
}
/* ------------------------------------------------------------------ */
} /* namespace lcd */
} /* namespace fnd */
/* ------------------------------------------------------------------ */
