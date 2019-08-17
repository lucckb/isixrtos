/*
 * seekbar.cpp
 *
 *  Created on: 15 paź 2013
 *      Author: lucck
 */
 
#include <gfx/gui/seekbar.hpp>
#include <foundation/sys/dbglog.h>
 
namespace gfx {
namespace gui {
 
//Repaint virtual function
void seekbar::repaint( bool /* focus */ )
{
	auto gdi = make_gdi( );
	//gdi.set_fg_color( get_layout().sel() );
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

	if (m_style == style::classic)
	{
		// outer part clear
		gdi.set_fg_color(get_owner().get_layout().bg());
		gdi.fill_area( c.x(), c.y(), c.cx(), y - c.y(), false );
		gdi.fill_area( c.x(), y_bg, c.cx(), c.cy() - ys - (y - c.y()), false );
		gdi.fill_area( c.x(), y, r, ys + 1, false );
		gdi.fill_area( x + cx, y, r + 1, ys + 1, false );
		gdi.set_pixel(x, y);
		gdi.set_pixel(x, y + ys);
		gdi.set_pixel(x + cx, y);
		gdi.set_pixel(x + cx, y + ys);
		
		// left part
		gdi.set_fg_color( colorspace::brigh( get_layout().sel(), 192 ) );

		if (x + cx > 0)
			gdi.draw_line(x + 1, y, x + cx - 1, y);

		if (x + ys > 0)
			gdi.draw_line(x, y + 1, x, y + ys - 1);

		if ((sex > 1) && (ys > 1))
		{
			gdi.set_fg_color( colorspace::brigh( get_layout().sel(), 0 ) );
			gdi.fill_area( x + 1, y + 1 , sex - 1, ys - 1, false );
		}

		if (x + cx > 0)
		{
			gdi.set_fg_color( colorspace::brigh( get_layout().sel(), -16 ) );
			gdi.draw_line(x + 1, y + ys, x + cx - 1, y + ys);
		}

		// right part
		gdi.set_fg_color( colorspace::brigh( get_layout().fg(), -96 ) );

		if (y + ys > 0)
			gdi.draw_line(x + cx, y + 1, x + cx, y + ys - 1);

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
	} else if (m_style == style::simple_bar)
	{
		if ((c.cx() > 5) && (c.cy() > 5))
		{
			constexpr auto border = 1;
			
			auto x1 = c.x();
			auto y1 = c.y();
			auto x2 = c.x() + c.cx() - 1;
			auto y2 = c.y() + c.cy() - 1;
			auto sx = c.cx() - border * 2 - 2;
			auto sy = c.cy() - border * 2 - 2;
			auto vx = (int(sx) * int((m_value - m_min))) / (int(m_max - m_min));
			
			gdi.set_fg_color(get_layout().fg());
			gdi.draw_line(x1, y1, x2, y1);
			gdi.draw_line(x1, y1, x1, y2);
			gdi.draw_line(x2, y1, x2, y2);
			gdi.draw_line(x1, y2, x2, y2);

			gdi.set_fg_color(get_layout().sel());

			if (vx > 0)
			{
				gdi.set_fg_color(get_layout().sel());
				gdi.fill_area(x1 + border + 1, y1 + border + 1 , vx, sy, false);
			}

			gdi.set_fg_color(get_layout().bg());
			if (sx > vx)
			{
				gdi.set_fg_color(get_layout().fg());
				gdi.fill_area(x1 + border + 1 + vx, y1 + border + 1, sx - vx, sy, true );
			}
		}
	}
}

 
//* Report input event
void seekbar::report_event( const input::event_info& ev )
{
	if (!m_report) return;

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
			else if( ev.keyb.key == kbdcodes::enter &&
					 ev.keyb.stat != kstat::RPT )
			{
				event btn_event( this, event::evtype::EV_CLICK );
				ret |= emit( btn_event );
			}
		}
	} else if( ev.type == event_info::EV_KNOB ) {
		if( ev.knob.diff > 0 ) {
			m_value += m_step;
			if( m_value > m_max ) m_value = m_max;
			else ret = true;
		} else if( ev.knob.diff < 0 ) {
			m_value -= m_step;
			if( m_value < m_min ) m_value = m_min;
			else ret = true;
		}
	}
	if( ret ) {
		event btn_event( this, event::evtype::EV_CHANGE );
		emit( btn_event );
		modified();
	}
}

 
} /* namespace gui */
} /* namespace gfx */

 
