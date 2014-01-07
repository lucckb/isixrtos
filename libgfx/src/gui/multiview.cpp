/*
 * =====================================================================================
 *
 *       Filename:  multiview.cpp
 *
 *    Description:  Multi Edit Box based only on last line message
 *
 *        Version:  1.0
 *        Created:  05.01.2014 19:49:23
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <gfx/gui/multiview.hpp>
#include <foundation/dbglog.h>

/* ------------------------------------------------------------------ */ 
namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */ 
//! On repaint the widget return true when changed
void multiview::repaint()
{
	constexpr auto luma = 128;
	constexpr auto luma2 = luma/2;
	auto gdi = make_wgdi( );
	const auto c = get_coord() + get_owner().get_coord();
	auto x = c.x() + text_margin;
	const auto yp = c.y() + c.cy() - 2 - gdi.get_text_height();
	//Draw test
	for( char ch : m_line ) {
		if( x + gdi.get_text_width(ch) >= c.x()+c.cx()-text_margin*2 ) {
		#if 0
			gdi.scroll( c.x()+text_margin, yp - gdi.get_text_height(), 
				c.cx()-2*text_margin, gdi.get_text_height(),
				gdi.get_text_height(), get_owner().get_layout().bg());
			x = c.x() + 1;
		#endif
		} else {
			x = gdi.draw_text( x , yp, ch );
		}
	}
	//FRM1
	gdi.set_fg_color( colorspace::brigh( get_layout().bg(), -luma ) );
	gdi.draw_line(c.x(), c.y()+c.cy(), c.x()+c.cx(), c.y()+c.cy() );
	gdi.draw_line(c.x()+c.cx(), c.y()+1, c.x()+c.cx(), c.y()+c.cy() );
	//FRM2
	gdi.set_fg_color( colorspace::brigh( get_layout().bg(), -luma2 ) );
	gdi.draw_line(c.x()+1, c.y()+c.cy()-1, c.x()+c.cx(), c.y()-1+c.cy() );
	gdi.draw_line(c.x()+c.cx()-1, c.y()+1, c.x()+c.cx()-1, c.y()+c.cy() );
	//FRM3
	gdi.set_fg_color( colorspace::brigh( get_layout().bg(), luma ) );
	gdi.draw_line(c.x()+1, c.y(), c.x()+c.cx()-1, c.y() );
	gdi.draw_line(c.x(), c.y(), c.x(), c.y()+c.cy()-2 );
}
/* ------------------------------------------------------------------ */ 
} //ns gui
} //ns gfx
/* ------------------------------------------------------------------ */ 

