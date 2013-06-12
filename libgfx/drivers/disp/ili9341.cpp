/*
 * ili9341.cpp
 *
 *  Created on: 10-06-2013
 *      Author: lucck
 */

/* ------------------------------------------------------------------ */
#include <gfx/drivers/disp/ili9341.hpp>

/* ------------------------------------------------------------------ */


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
int ili9341::line( coord_t x, coord_t y, coord_t cx, coord_t cy,color_t color )
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
	m_bus.fill( color, SCREEN_WIDTH*SCREEN_HEIGHT );
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
void ili9341::reset()
{
	m_bus.set_ctlbits( RST_BIT_CMD, true );
	m_bus.delay( 10 );
	m_bus.set_ctlbits( RST_BIT_CMD, false );
	m_bus.delay( 10 );
	m_bus.set_ctlbits( RST_BIT_CMD, true );
	m_bus.delay( 150 );

}
/* ------------------------------------------------------------------ */
//Initialize display
int ili9341::init_display()
{
	reset();
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
	/*
	command( dcmd::GAMMASET, 0x01 );
	command( dcmd::POSGAMMACORRECTION, 0x0f, 0x27, 0x23,
			 0x0b, 0x0f, 0x05, 0x54, 0x74, 0x45, 0x0A, 0x17, 0x0A,
			 0x1c, 0x0e, 0x00, 0x08 );
	command( dcmd::NEGGAMMACORRECTION, 0x08, 0x1A, 0x1E, 0x03,
			0x0f, 0x05, 0x2e, 0x25, 0x3b, 0x01,
			0x06, 0x05, 0x25, 0x33, 0x00, 0x0f );
	*/
	command( dcmd::PIXFORMATSET, 0x55 );
	command( dcmd::MEMACCESS, 0x00 );
	command( dcmd::INTERFCTL, 0x01, 0x30 );
	command( dcmd::SLEEPOUT ); command_();
	m_bus.delay( 150 );
	command( dcmd::DISPLAYON );	command_();
	m_bus.delay( 30 );
    command( dcmd::COLADDRSET,  0, 0,  SCREEN_WIDTH>>8, SCREEN_WIDTH );
    command( dcmd::PAGEADDRSET, 0, 0, SCREEN_HEIGHT>>8, SCREEN_HEIGHT );
    command( dcmd::INTERFCTL, 0x01, 0x30 );
    //TODO: FIXME this
    return 0;
}
/* ------------------------------------------------------------------ */
//Set cursor position
void ili9341::set_area( coord_t x, coord_t y, coord_t cx, coord_t cy )
{
    const auto x2 = x + cx - 1;
    const auto y2 = y + cy - 1;
	command( dcmd::COLADDRSET,  x>>8, x, x2>>8, x2 );
    command( dcmd::PAGEADDRSET, y>>8, y, y2>>8, y2 );
}
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
