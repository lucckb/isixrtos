/*
 * seekbar.cpp
 *
 *  Created on: 15 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/seekbar.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
//Repaint virtual function
void seekbar::repaint( bool /* focus */ )
{
	auto gdi = make_gdi( );
	gdi.set_fg_color( get_layout().sel() );
	const auto c = get_coord() + get_owner().get_coord();
	const auto ys = c.cy()/2 + c.cy()/8;
	const auto r = c.cy()/2 - c.cy()/8;
	const auto y = c.y()+(c.cy()-ys)/2;
	const auto x = c.x() + r;
	const auto cx = c.cx() - 2*r;
	auto sex = (int(cx) * int(m_value - m_min))/int(m_max - m_min);

	const auto ycirc = c.y()+c.cy()/2;
	const auto y_bg = y + ys + 1;

	if (sex < 0) sex = 0;

	// outer part clear
	gdi.set_fg_color(get_owner().get_layout().bg());
	gdi.fill_area( c.x(), c.y(), c.cx(), y - c.y(), false );
	gdi.fill_area( c.x(), y_bg, c.cx(), c.cy() - ys - (y - c.y()), false );
	gdi.fill_area( c.x(), y, r, ys + 1, false );
	gdi.fill_area( x + cx, y, r, ys + 1, false );
	gdi.set_pixel(x, y);
	gdi.set_pixel(x, y + ys);
	gdi.set_pixel(x + cx, y);
	gdi.set_pixel(x + cx, y + ys);
	
	// left part
	gdi.set_fg_color( colorspace::brigh( get_layout().sel(), 192 ) );
	gdi.draw_line(x + 1, y, x + sex, y);
	if (x + ys > 0)
		gdi.draw_line(x, y + 1, x, y + ys - 1);

	if ((sex > 1) && (ys > 1))
	{
		gdi.set_fg_color( colorspace::brigh( get_layout().sel(), 0 ) );
		gdi.fill_area( x + 1, y + 1 , sex - 1, ys - 1, false );
	}

	gdi.set_fg_color( colorspace::brigh( get_layout().sel(), -16 ) );
	gdi.draw_line(x + 1, y + ys, x + sex, y + ys);

	// right part
	if (x + cx > 0)
	{
		gdi.set_fg_color( colorspace::brigh( get_layout().fg(), 192 ) );
		gdi.draw_line(x + sex, y, x + cx - 1, y);
	}

	gdi.set_fg_color( colorspace::brigh( get_layout().fg(), -96 ) );

	if (y + ys > 0)
		gdi.draw_line(x + cx, y + 1, x + cx, y + ys - 1);

	if (x + cx > 0)
		gdi.draw_line(x + sex, y + ys, x + cx - 1, y + ys);

	if ((cx > sex) && (ys > 1))
	{
		gdi.set_fg_color( colorspace::brigh( get_layout().fg(), 0 ) );
		gdi.fill_area( x + sex, y + 1, cx - sex, ys - 1, true );
	}

	// ball
	gdi.set_fg_color( colorspace::brigh(get_layout().bg(), 32));
	gdi.set_fill( true );
	gdi.draw_circle( x+sex,ycirc, r);

	gdi.set_fg_color( colorspace::brigh(get_layout().bg(), -32));
	gdi.set_fill( false );
	gdi.draw_circle( x+sex,ycirc, r);

	gdi.set_fg_color( colorspace::brigh(get_layout().bg(), 32));
	gdi.draw_circle( x+sex,ycirc, r - 1);
}
/* ------------------------------------------------------------------ */
//* Report input event
void seekbar::report_event( const input::event_info& ev )
{
	using namespace gfx::input;
	using kstat = input::detail::keyboard_tag::status;
	bool ret {};
	if( ev.type == event_info::EV_KEY ) {
		if( ev.keyb.stat==kstat::DOWN || ev.keyb.stat==kstat::RPT ) {
			if( ev.keyb.key == kbdcodes::os_arrow_up )
			{
				m_value += m_step;
				if( m_value > m_max ) m_value = m_max;
				else ret = true;
			}
			else if( ev.keyb.key == kbdcodes::os_arrow_down )
			{
				m_value -= m_step;
				if( m_value < m_min ) m_value = m_min;
				else ret = true;
			}
			else if(  ev.keyb.key == kbdcodes::enter )
			{
				event btn_event( this, event::evtype::EV_CLICK );
				ret |= emit( btn_event );
			}
		}
	}
	if( ret ) {
		event btn_event( this, event::evtype::EV_CHANGE );
		emit( btn_event );
		modified();
	}
}

/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */

/* ------------------------------------------------------------------ */
