/*
 * ili9341.cpp
 *
 *  Created on: 10-06-2013
 *      Author: lucck
 */

 
#include <gfx/drivers/disp/ili9341.hpp>
#include <cstdlib>
 
//TODO: Setup better configuration for init Refresh/RADIO gamma correction etc

namespace gfx {
namespace drv {

 
//Constructor
ili9341::ili9341( disp_bus &bus ) :
	disp_base( SCREEN_WIDTH, SCREEN_HEIGHT ), m_bus( bus )
{
}
 
/* Get PIXEL */
color_t ili9341::get_pixel( coord_t x, coord_t y )
{
	bus_lock lock(m_bus);
	uint8_t color[4];
	set_viewport( x, y, 1, 1 );
	command( dcmd::MEMORYREAD);
	m_bus.read( color, sizeof color );
	return rgb( color[3], color[2], color[1] );
}
 
/* Set PIXEL */
void ili9341::clear( color_t color )
{
	bus_lock lock(m_bus);
	if( m_orient==rotation_t::rot_0 || m_orient==rotation_t::rot_180 )
		set_viewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );

	else
		set_viewport( 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH );
	command( dcmd::MEMORYWRITE );
	m_bus.fill( color, SCREEN_WIDTH*SCREEN_HEIGHT );
}
 
/* Fill area */
void ili9341::set_pixel( coord_t x, coord_t y, color_t color )
{
	bus_lock lock(m_bus);
	set_viewport( x, y, 1, 1 );
	command( dcmd::MEMORYWRITE );
	m_bus.write( &color, sizeof color );
}
 
/* Blit area */
void ili9341::blit( coord_t x, coord_t y, coord_t cx, coord_t cy,
			        coord_t src_y, const color_t *buf )
{
	bus_lock lock(m_bus);
	buf +=  src_y * cx;
	set_viewport( x, y, cx, cy );
	command( dcmd::MEMORYWRITE );
	m_bus.write( buf, sizeof(color_t)*cx*cy );
}
 
/* Vertical scroll */
/** Rotate screen
 * @param[in]  x,y 		Source region
 * @param[in]  cx,cy  	Region length
 * @param[in]  lines  	Lines to scrol pos or neg
 * @param[in]  bgcolor 	Background color
 */
void ili9341::vert_scroll( coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor )
{
	{
		bus_lock lock(m_bus);
		command( dcmd::PIXFORMATSET, 0x66 );	//Set pixel format like read format
		if( lines > 0 )
		{
			for( unsigned l=0,le=(cy-lines); l<le; ++l )
			{
				move_line( x, cx, y+lines+l, y+l  );
			}
		}
		else
		{
			lines = -lines;
			for( int l=cy-lines; l>=0; --l )
			{
				move_line( x, cx, y + l, y+lines+l );
			}
			lines = -lines;
		}
		command( dcmd::PIXFORMATSET, 0x55 ); 	// Restore previous pixel format
	}
	if( lines > 0 )
		ili9341::fill( x, y+cy-lines, cx, lines, bgcolor );
	else
		ili9341::fill( x, y, cx, -lines, bgcolor );
}
 
//Move line
void ili9341::move_line( coord_t x, coord_t cx, coord_t row_from, coord_t row_to )
{
	/* read row0 into the buffer and then write at row1*/
	set_viewport(x, row_from, cx, 1);
	command( dcmd::MEMORYREAD );
	m_bus.read( m_scr_buf, (cx*3)+1);
	set_viewport(x, row_to, cx, 1);
	command( dcmd::MEMORYWRITE );
	m_bus.write( m_scr_buf+1, cx*3 );
}

 
bool ili9341::power_ctl( power_ctl_t  mode )
{
	switch( mode )
	{
	case power_ctl_t::on:
	{
		if( m_pwrstate == power_ctl_t::unknown )
		{
			if( !init_display() )
			{
				m_pwrstate = mode;
			}
		}
		else if( m_pwrstate == power_ctl_t::sleep )
		{
			bus_lock lock(m_bus);
			command( dcmd::SLEEPOUT );
			m_bus.delay( 150 );
			command( dcmd::DISPLAYON );
			m_bus.delay( 30 );
			m_pwrstate = mode;
		}
		else if( m_pwrstate == power_ctl_t::off )
		{
			bus_lock lock(m_bus);
			command( dcmd::DISPLAYON );
			m_bus.delay( 30 );
			m_pwrstate = mode;
		}
		break;
	}
	case power_ctl_t::off:
	{
		if( m_pwrstate == power_ctl_t::on )
		{
			bus_lock lock(m_bus);
			command( dcmd::DISPLAYOFF );
			m_pwrstate = mode;
		}
		break;
	}
	case power_ctl_t::sleep:
	{
		if( m_pwrstate != power_ctl_t::sleep )
		{
			bus_lock lock(m_bus);
			command( dcmd::SLEEPENTER );
			m_bus.delay( 10 );
			m_pwrstate = mode;
		}
		break;
	}
	default:
		break;
	//switchout
	}
	return !(m_pwrstate==mode);
}
 
/* Rotate screen */
void ili9341::rotate( rotation_t rot )
{
	static constexpr uint8_t MY = 1<<7;
	static constexpr uint8_t MX = 1<<6;
	static constexpr uint8_t MV = 1<<5;
	if( m_orient != rot )
	{
		uint8_t mad_reg = MADREG_RGB;
		switch( rot )
		{
		case rotation_t::rot_0:
			set_screen_size( SCREEN_WIDTH, SCREEN_HEIGHT );
			break;
		case rotation_t::rot_90:
			mad_reg |= (MV|MX);
			set_screen_size( SCREEN_HEIGHT, SCREEN_WIDTH );
			break;
		case rotation_t::rot_180:
			mad_reg |= MY;
			set_screen_size( SCREEN_WIDTH, SCREEN_HEIGHT );
			break;
		case rotation_t::rot_270:
			set_screen_size( SCREEN_HEIGHT, SCREEN_WIDTH );
			mad_reg |= (MV|MY) ;
			break;
			break;
		}
		bus_lock lock(m_bus);
		command( dcmd::MADCTL, mad_reg );
		m_orient = rot;
	}
}
 
void ili9341::reset()
{
	m_bus.set_ctlbits( RST_BIT_CMD, false );
	m_bus.delay( 10 );
	m_bus.set_ctlbits( RST_BIT_CMD, true );
	m_bus.delay( 150 );
}
 
//Initialize display
bool ili9341::init_display()
{
	reset();
	bus_lock lock(m_bus);
	command( dcmd::POWERCTLB, 0x00, 0x81, 0x00, 0x30 );
	command( dcmd::POWERONSEQCTL, 0x64, 0x03, 0x12, 0x81  );
	command( dcmd::DIVTIMCTLA, 0x85, 0x00, 0x00, 0x79 );
	command( dcmd::POWERCTLA,  0x39, 0x2c, 0x00, 0x34, 0x00, 0x02 );
	command( dcmd::PUMPRATIOCTL, 0x20 );
	command( dcmd::DIVTIMCTLB, 0x00, 0x00 );
	command( dcmd::FRAMECTLNOR, 0x00, 0x1b );
	command( dcmd::POWERCTL1, 0x2e );
	command( dcmd::POWERCTL2, 0x12 );
	command( dcmd::VCOMCTL1, 0x50, 0x19 );
	command( dcmd::VCOMCTL2, 0x90 );
	command( dcmd::MEMACCESS, 0xA8 );
	command( dcmd::FUNCTIONCTL, 0x0A, 0xA2 );
	command( dcmd::ENABLE3GREG, 0x00 );
	command( dcmd::GAMMASET, 0x01 );
	if(0) {
	command( dcmd::POSGAMMACORRECTION, 0x0f, 0x27, 0x23,
			 0x0b, 0x0f, 0x05, 0x54, 0x74, 0x45, 0x0A, 0x17, 0x0A,
			 0x1c, 0x0e, 0x00, 0x08 );
	command( dcmd::NEGGAMMACORRECTION, 0x08, 0x1A, 0x1E, 0x03,
			0x0f, 0x05, 0x2e, 0x25, 0x3b, 0x01,
			0x06, 0x05, 0x25, 0x33, 0x00, 0x0f );
	}
	command( dcmd::PIXFORMATSET, 0x55 );
	command( dcmd::MEMACCESS, 0x00 );
	command( dcmd::INTERFCTL, 0x01, 0x30, 1<<5 );

	command( dcmd::MADCTL, MADREG_RGB );

	//Enable the Display
	command( dcmd::SLEEPOUT );
	m_bus.delay( 150 );
	command( dcmd::DISPLAYON );
	m_bus.delay( 30 );
	//Check if the display is connected
	{
		uint8_t res[2];
		command( dcmd::READPWRMODE );
		m_bus.read( res, sizeof res );
		if( !(res[1]&0x80) ) return true;
	}
	return false;
}

 
//Set viewport
void ili9341::set_viewport( coord_t x, coord_t y, coord_t cx, coord_t cy )
{
	const auto x2 = x + cx - 1;
	const auto y2 = y + cy - 1;
	command( dcmd::COLADDRSET,  x>>8, x, x2>>8, x2);
	command( dcmd::PAGEADDRSET, y>>8, y, y2>>8, y2);
}
 
/* Fill area */
void ili9341::fill( coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color )
{
	bus_lock lock(m_bus);
	set_viewport( x, y, cx, cy );
	command( dcmd::MEMORYWRITE );
	m_bus.fill( color, cx * cy );
}

 
/* Set blit area (viewport) */
void ili9341::ll_blit( coord_t x, coord_t y, coord_t cx, coord_t cy )
{
	bus_lock lock(m_bus);
	const auto x2 = x + cx - 1;
	const auto y2 = y + cy - 1;
	command( dcmd::COLADDRSET,  x>>8, x, x2>>8, x2 );
	command( dcmd::PAGEADDRSET, y>>8, y, y2>>8, y2 );
	command( dcmd::MEMORYWRITE );
}

 
/* Push into the memory */
void ili9341::ll_blit( const color_t *buf, size_t len )
{
	bus_lock lock(m_bus);
	m_bus.write( buf, sizeof(color_t)*len );
}

 
}}
 
