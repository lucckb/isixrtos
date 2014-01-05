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
void seekbar::repaint()
{
	constexpr auto luma = 32;
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
	gdi.fill_area( x, y , sex, ys, false );
	gdi.fill_area( x+sex, y, cx-sex, ys, true );
	gdi.set_fg_color( colorspace::brigh(get_layout().bg(), luma));
	gdi.set_fill( true );
	gdi.draw_circle( x+sex,ycirc, r);
	m_psex = sex;
}
/* ------------------------------------------------------------------ */
//* Report input event
void seekbar::report_event( const input::event_info& ev )
{
	using namespace gfx::input;
	bool ret {};
	if( ev.keyb.stat == input::detail::keyboard_tag::status::DOWN )
	{
		if( ev.keyb.key == kbdcodes::os_arrow_down )
		{
			m_value += m_step;
			if( m_value > m_max ) m_value = m_max;
			else ret = true;
		}
		else if( ev.keyb.key == kbdcodes::os_arrow_up )
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
	if( ret )
	{
		event btn_event( this, event::evtype::EV_CHANGE );
		ret |= emit( btn_event );
	}
	dirty( ret );
}

/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */

/* ------------------------------------------------------------------ */
