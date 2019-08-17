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
#include <foundation/sys/dbglog.h>
  
namespace gfx {
namespace gui {
 
//! Constructor
battery_icon::battery_icon( rectangle const& rect,layout const& layout ,window &win )
	: widget( rect, layout, win )
{
}
 
//! On repaint widget event
void battery_icon::repaint( bool /* focus */ ) 
{
	constexpr auto luma = 32;
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
	gdi.set_fg_color(get_layout().bg());

	gdi.draw_line(x0, y0, x0 + w, y0);
	gdi.draw_line(x0, y0 + h, x0 + w, y0 + h);
	gdi.draw_line(x0, y0 + 1, x0, y0 + h - 1);
	gdi.draw_line(x0 + w, y0 + 1, x0 + w, y0 + h - 1);

	// positive connector
	gdi.draw_line(x0 + w + 1, y0 + 3, x0 + w + 1, y0 + h - 3);
	gdi.draw_line(x0 + w + 2, y0 + 3, x0 + w + 2, y0 + h - 3);
	gdi.draw_line(x0 + w + 3, y0 + 4, x0 + w + 3, y0 + h - 4);

	// inner rect
	gdi.bright_fg_color( luma );
	gdi.draw_line(x0 + 1, y0 + 1, x0 + w - 1, y0 + 1);
	gdi.draw_line(x0 + 1, y0 + h - 1, x0 + w - 1, y0 + h - 1);
	gdi.draw_line(x0 + 1, y0 + 2, x0 + 1, y0 + h - 2);
	gdi.draw_line(x0 + w - 1, y0 + 2, x0 + w - 1, y0 + h - 2);

	// inner fill
	gdi.set_fg_color(get_layout().fg());
	gdi.fill_area(x0 + 3, y0 + 3, w1, h - 5);

	gdi.set_fg_color(get_layout().bg());
	gdi.fill_area(x0 + 3 + w1, y0 + 3, w2, h - 5);

	if (m_charging)
	{
		// drawing charging icon
		auto bx0 = x0 + 4;
		auto by0 = y0 + 4;
		auto bx1 = x0 + w - 4;
		auto by1 = y0 + h - 4;
		auto bxup = bx0 + ((bx1 - bx0) * 5) / 9;	
		auto bxdn = bx0 + ((bx1 - bx0) * 4) / 9;
		auto bym = (by0 + by1) / 2;

		if ((bx1 > bx0) && (by1 > by0))
		{
			gdi.set_fg_color(color::White);

			gdi.draw_line(bx0, bym, bxup, by0);
			gdi.draw_line(bxup, by0, bxup, bym);
			gdi.draw_line(bxup, bym, bx1, bym);
			gdi.draw_line(bx0, bym, bxdn, bym);
			gdi.draw_line(bxdn, bym, bxdn, by1);
			gdi.draw_line(bxdn, by1, bx1, bym);
		}
	}else if (m_full)
	{
		if (h > 12)
		{
			// drawing full (F) icon
			auto by0 = y0 + 4;
			auto by1 = y0 + h - 4;
			auto fh = h - 8;
			auto d = fh / 2;
			auto x = x0 + w / 2 ;
			auto x0 = x - d;
			auto x1 = x + d;
			
			gdi.set_fg_color(color::White);

			gdi.draw_line(x0, by0, x0, by1);
			gdi.draw_line(x0 + 1, by0, x0 + 1, by1);

			gdi.draw_line(x0, by0, x1, by0);
			gdi.draw_line(x0, by0 + 1, x1, by0 + 1);

			gdi.draw_line(x0, by0 + d, x + d / 4, by0 + d);
			gdi.draw_line(x0, by0 + d + 1, x + d / 4, by0 + d + 1);
		}
	}
}
 
}
}
  
