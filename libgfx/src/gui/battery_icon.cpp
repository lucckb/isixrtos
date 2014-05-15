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
	const auto bat_top_width = 2;
	const auto c = get_coord() + get_owner().get_coord();
	auto gdi = make_gdi();
	const auto h =  c.cy() / 2;
	const auto w = c.cx() - bat_top_width;
	const auto w1 = (w * m_percent) / 100;
	const auto w2 = w - w1;
	const auto x0 = 2 + c.x();
	const auto y0 = 1 + c.y() + c.cy() / 4;
	dbprintf("percent value %u", m_percent );
	dbprintf("gdi_fill_area(%u, %u, %u, %u)", x0, y0, w1, h );
	gdi.fill_area( x0, y0, w1, h );
	dbprintf("gdi_fill_area(%u, %u, %u, %u)", x0+w1, y0, w2, h );
	gdi.fill_area( x0 + w1 , y0, w2, h , true );
	// Battery top
	const auto bat_top_y = y0 + ( y0 + h )/4 - 1;
	gdi.fill_area( x0 + w, bat_top_y , bat_top_width, h/2 , true );
}
/* ------------------------------------------------------------------ */
//! Report input event handling 
void battery_icon::report_event( const input::event_info& ev )
{
	if( ev.type == input::event_info::EV_CHANGE ) {
		modified();
	}
}
/* ------------------------------------------------------------------ */ 
}
}
/* ------------------------------------------------------------------ */ 
