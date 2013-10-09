/*
 * button.cpp
 *
 *  Created on: 2 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/button.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
// On repaint the window return true when changed
void button::repaint()
{
	constexpr auto luma = 128;
	constexpr auto luma2 = luma/2;
	auto gdi = make_gdi( );
	const auto c = get_coord() + get_owner().get_coord();
	gdi.fill_area( c.x()+1, c.y()+1, c.cx()-2, c.cy()-2, true );
	const auto tx = c.x() + (c.cx() - gdi.get_text_width(m_caption.c_str()))/2;
	const auto ty = c.y() + (c.cy() - gdi.get_text_height())/2;
	gdi.draw_text( tx, ty, m_caption.c_str());
	if( !m_pushed )
	{
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
	else
	{
		gdi.set_fg_color( get_layout().bg() );
		gdi.draw_line(c.x()-1, c.y()+c.cy()-1, c.x()+c.cx()-1, c.y()-1+c.cy() );
		gdi.draw_line(c.x()+c.cx()-1, c.y()+1, c.x()+c.cx()-1, c.y()+c.cy() );
		//gdi.set_fg_color( colorspace::brigh( get_layout().bg(), -luma ) );
		gdi.draw_line(c.x(), c.y(), c.x() + c.cx(), c.y() );
		gdi.draw_line(c.x(), c.y(), c.x(), c.y() + c.cy() );


	}
}
/* ------------------------------------------------------------------ */
//* Report input event
bool button::report_event( const input::event_info& ev )
{
	bool ret {};
	if( m_push_key > 0 && m_push_key == ev.keyb.key )
	{
		m_pushed = ( ev.keyb.stat == input::detail::keyboard_tag::status::DOWN );
		event btn_event( this, event::evtype::EV_CLICK );
		ret = emit( event( this, ev));
	}
	return ret;
}
/* ------------------------------------------------------------------ */

} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
