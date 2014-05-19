/*
 * =====================================================================================
 *
 *       Filename:  waterfall.cpp
 *
 *    Description:  Waterfall widget
 *
 *        Version:  1.0
 *        Created:  19.05.2014 21:53:46
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gfx/gui/waterfall.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */ 
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */ 
//! GUI dram frame
void waterfall::draw_frame()
{
	constexpr auto luma = 64;
	constexpr auto luma2 = luma/2;
	auto gdi = make_wgdi( );
	const auto c = get_coord() + get_owner().get_coord();
	//FRM1
	gdi.bright_fg_color( -luma );
	gdi.draw_line(c.x(), c.y()+c.cy(), c.x()+c.cx(), c.y()+c.cy() );
	gdi.draw_line(c.x()+c.cx(), c.y()+1, c.x()+c.cx(), c.y()+c.cy() );
	//FRM2
	gdi.bright_fg_color( -luma2 );
	gdi.draw_line(c.x()+1, c.y()+c.cy()-1, c.x()+c.cx(), c.y()-1+c.cy() );
	gdi.draw_line(c.x()+c.cx()-1, c.y()+1, c.x()+c.cx()-1, c.y()+c.cy() );
	//FRM3
	gdi.bright_fg_color( -luma );
	gdi.draw_line(c.x()+1, c.y(), c.x()+c.cx()-1, c.y() );
	gdi.draw_line(c.x(), c.y(), c.x(), c.y()+c.cy()-2 );
}
/* ------------------------------------------------------------------ */ 
//! Handle waterfall event info
void waterfall::report_event( const input::event_info& ev )
{
	dbprintf( "WATERFALL UNHANDLED event %i", ev.type );
	/*  Howto select data from FFT 
	 *  (FFT_SIZE * i / SCREEN_SIZE 
	 *  Color?? X=V/256 RGB=( X, 0, 255 - X )
	 */
}
/* ------------------------------------------------------------------ */ 
//! Handle repaint 
void waterfall::repaint()
{
	//Draw bottom gui frame
	draw_frame();
}
/* ------------------------------------------------------------------ */ 
}
}
/* ------------------------------------------------------------------ */ 
