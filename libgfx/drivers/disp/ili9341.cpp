/*
 * ili9341.cpp
 *
 *  Created on: 10-06-2013
 *      Author: lucck
 */

/* ------------------------------------------------------------------ */
#include <gfx/drivers/disp/ili9341.hpp>

/* ------------------------------------------------------------------ */

extern "C" void isix_wait( int );

namespace gfx {
namespace drv {

/* ------------------------------------------------------------------ */
//Constructor
ili9341::ili9341( disp_bus &bus ) :
	disp_base( SCREEN_WIDTH, SCREEN_HEIGHT ), m_bus( bus )
{

}
/* ------------------------------------------------------------------ */
/* Set pixel */
int ili9341::set_pixel( coord_t x, coord_t y, color_t color )
{

}
/* ------------------------------------------------------------------ */
/* Get PIXEL */
color_t ili9341::get_pixel()
{

}
/* ------------------------------------------------------------------ */
/* Set PIXEL */
int ili9341::clear( color_t color )
{
	command( dcmd::MEMORYWRITE );
	for( size_t p=0; p<(SCREEN_WIDTH*SCREEN_HEIGHT); ++p )
		m_bus.write( color );
	command_();
}
/* ------------------------------------------------------------------ */
/* Fill area */
int ili9341::fill( coord_t x, coord_t y, coord_t cx, coord_t cy )
{

}
/* ------------------------------------------------------------------ */
/* Blit area */
int ili9341::blit( coord_t x, coord_t y, coord_t cx, coord_t cy,
			      coord_t src_x, coord_t src_y, coord_t src_cx, const color_t *buf )
{

}
/* ------------------------------------------------------------------ */
/* Vertical scroll */
int ili9341::vert_scroll( coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor )
{

}
/* ------------------------------------------------------------------ */
/* Power ctl */
int ili9341::power_ctl( power_ctl_t  mode  )
{
	switch( mode )
	{
	case power_ctl_t::on:
		return init_display();
		break;
	}
}
/* ------------------------------------------------------------------ */
/* Rotate screen */
int ili9341::rotate( rotation_t /* rot */)
{
	return 0;
}
/* ------------------------------------------------------------------ */
//Initialize display
int ili9341::init_display()
{
	m_bus.reset();
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
	command( dcmd::POSGAMMACORRECTION, 0x0f, 0x27, 0x23,
			 0x0b, 0x0f, 0x05, 0x54, 0x74, 0x45, 0x0A, 0x17, 0x0A,
			 0x1c, 0x0e, 0x00, 0x08 );
	command( dcmd::NEGGAMMACORRECTION, 0x08, 0x1A, 0x1E, 0x03,
			0x0f, 0x05, 0x2e, 0x25, 0x3b, 0x01,
			0x06, 0x05, 0x25, 0x33, 0x00, 0x0f );
	command( dcmd::PIXFORMATSET, 0x55 );
	command( dcmd::MEMACCESS, 0x00 );
	command( dcmd::INTERFCTL, 0x01, 0x30 );
	command( dcmd::DISPLAYON );	command_();
	command( dcmd::SLEEPOUT ); command_();
	isix_wait( 500 );
    command( dcmd::COLADDRSET,  0, 0,  SCREEN_WIDTH>>8, SCREEN_WIDTH );
    command( dcmd::PAGEADDRSET, 0, 0, SCREEN_HEIGHT>>8, SCREEN_HEIGHT );
    command( dcmd::INTERFCTL, 0x01, 0x30 );
    //TODO: FIXME this
    return 0;
}
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
