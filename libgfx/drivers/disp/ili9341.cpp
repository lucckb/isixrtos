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
int ili9341::clear()
{

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
int ili9341::power_ctl( power_ctl_t /* mode */ )
{

}
/* ------------------------------------------------------------------ */
/* Rotate screen */
int ili9341::rotate( rotation_t /* rot */)
{
	command( 0x10, 0x10203040 );
}
/* ------------------------------------------------------------------ */
//Initialize display
int ili9341::init_display()
{
	
}
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
