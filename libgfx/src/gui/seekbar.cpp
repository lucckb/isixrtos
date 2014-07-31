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
	// do not repaint if value was not changed
	//if (m_value == m_pvalue) return;
	//m_pvalue = m_value;

	auto gdi = make_gdi( );
	gdi.set_fg_color( get_layout().sel() );
	const auto c = get_coord() + get_owner().get_coord();
	const auto ys = c.cy()/2 + c.cy()/8;
	const auto r = c.cy()/2 - c.cy()/8;
	const auto y = c.y()+(c.cy()-ys)/2;
	const auto x = c.x() + r;
	const auto cx = c.cx() - 2*r;
	const auto sex = (int(cx) * int(m_value))/int(m_max);
	const auto ycirc = c.y()+c.cy()/2;

	if( m_psex != sex )
	{
		auto gdiw = make_wgdi();
		gdiw.set_fg_color( get_owner().get_layout().bg() );
		gdiw.set_fill(true);
		gdiw.draw_circle( x+m_psex ,ycirc, r );
	}

	// left part
	gdi.set_fg_color( colorspace::brigh( get_layout().sel(), 192 ) );
	gdi.draw_line(x + 1, y, x + sex, y);
	gdi.draw_line(x, y + 1, x, y + ys - 1);

	gdi.set_fg_color( colorspace::brigh( get_layout().sel(), 0 ) );
	gdi.fill_area( x + 1, y + 1 , sex - 1, ys - 1, false );

	gdi.set_fg_color( colorspace::brigh( get_layout().sel(), -16 ) );
	gdi.draw_line(x + 1, y + ys, x + sex, y + ys);

	// right part
	gdi.set_fg_color( colorspace::brigh( get_layout().fg(), 192 ) );
	gdi.draw_line(x + sex, y, x + cx - 1, y);

	gdi.set_fg_color( colorspace::brigh( get_layout().fg(), -96 ) );
	gdi.draw_line(x + cx, y + 1, x + cx, y + ys - 1);
	gdi.draw_line(x + sex, y + ys, x + cx -1, y + ys);

	gdi.set_fg_color( colorspace::brigh( get_layout().fg(), 0 ) );
	gdi.fill_area( x + sex, y + 1, cx - sex, ys - 1, true );

	// ball
	gdi.set_fg_color( colorspace::brigh(get_layout().bg(), 32));
	gdi.set_fill( true );
	gdi.draw_circle( x+sex,ycirc, r);

	gdi.set_fg_color( colorspace::brigh(get_layout().bg(), -32));
	gdi.set_fill( false );
	gdi.draw_circle( x+sex,ycirc, r);

	gdi.set_fg_color( colorspace::brigh(get_layout().bg(), 32));
	gdi.draw_circle( x+sex,ycirc, r - 1);


	m_psex = sex;
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
