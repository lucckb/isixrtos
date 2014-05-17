/*
 * =====================================================================================
 *
 *       Filename:  battery_icon.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14.05.2014 20:00:18
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gfx/gui/battery_icon.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */ 
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
//! Constructor
battery_icon::battery_icon( rectangle const& rect,layout const& layout ,window &win )
	: widget( rect, layout, win )
{
}
/* ------------------------------------------------------------------ */
//! On repaint widget event
void battery_icon::repaint() 
{
	dbprintf("battery_icon -> Repaint %u", m_percent );
	constexpr auto luma = 48;
	const auto bat_top_width = 3;
	const auto c = get_coord() + get_owner().get_coord();
	auto gdi = make_gdi();
	const auto h =  c.cy();
	const auto w = c.cx() - bat_top_width;
	const auto wi = w - 5;
	const auto w1 = (wi * m_percent) / 100;
	const auto w2 = wi - w1;
	const auto x0 = c.x();
	const auto y0 = c.y();

	// outer rect
	gdi.set_fg_color(colorspace::brigh(get_layout().bg(), 0));
	gdi.draw_line(x0, y0, x0 + w, y0);
	gdi.draw_line(x0, y0 + h, x0 + w, y0 + h);
	gdi.draw_line(x0, y0 + 1, x0, y0 + h - 1);
	gdi.draw_line(x0 + w, y0 + 1, x0 + w, y0 + h - 1);

	// positive connector
	gdi.draw_line(x0 + w + 1, y0 + 3, x0 + w + 1, y0 + h - 3);
	gdi.draw_line(x0 + w + 2, y0 + 3, x0 + w + 2, y0 + h - 3);
	gdi.draw_line(x0 + w + 3, y0 + 4, x0 + w + 3, y0 + h - 4);

	// inner rect
	gdi.set_fg_color(colorspace::brigh(get_layout().bg(), luma));
	gdi.draw_line(x0 + 1, y0 + 1, x0 + w - 1, y0 + 1);
	gdi.draw_line(x0 + 1, y0 + h - 1, x0 + w - 1, y0 + h - 1);
	gdi.draw_line(x0 + 1, y0 + 2, x0 + 1, y0 + h - 2);
	gdi.draw_line(x0 + w - 1, y0 + 2, x0 + w - 1, y0 + h - 2);

	// inner fill
	gdi.set_fg_color(gfx::color::LightGreen);
	gdi.fill_area(x0 + 3, y0 + 3, w1, h - 5);
	gdi.set_fg_color(colorspace::brigh(get_layout().bg(), -luma));
	gdi.fill_area(x0 + 3 + w1, y0 + 3, w2, h - 5);

//	dbprintf("percent value %u", m_percent );
//	dbprintf("gdi_fill_area(%u, %u, %u, %u)", x0, y0, w1, h );
//	gdi.fill_area( x0, y0, w1, h );
//	dbprintf("gdi_fill_area(%u, %u, %u, %u)", x0+w1, y0, w2, h );
//	gdi.fill_area( x0 + w1 , y0, w2, h , true );
	// Battery top
//	const auto bat_top_y = y0 + ( y0 + h )/4 - 1;
//	gdi.fill_area( x0 + w, bat_top_y , bat_top_width, h/2 , true );
}
/* ------------------------------------------------------------------ */
//! Report input event handling 
void battery_icon::report_event( const input::event_info& ev )
{
	using evinfo = input::event_info;
	if( ev.type == evinfo::EV_CHANGE ) {
		modified();
	}
}
/* ------------------------------------------------------------------ */ 
}
}
/* ------------------------------------------------------------------ */ 
