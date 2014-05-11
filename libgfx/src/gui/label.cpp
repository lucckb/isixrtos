/*
 * label.cpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/label.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
//Constructor
label::label( rectangle const& rect,layout const& layout ,window &win )
	: widget( rect, layout, win, false ), draw_text_wdt(0)
{
}
/* ------------------------------------------------------------------ */
//Repaint the lable
void label::repaint()
{
	const auto c = get_coord() + get_owner().get_coord();
	auto gdi = make_wgdi();
	const auto ty = c.y() + (c.cy() - gdi.get_text_height())/2;
	auto text_wdt = 0;

	// draw text
	if( !m_caption.empty() )
		text_wdt = gdi.draw_text( c.x(), ty, m_caption.c_str() );

	// clear background part
	if (text_wdt < draw_text_wdt)
		gdi.fill_area(text_wdt, ty , draw_text_wdt - text_wdt, get_coord().cy(), true);

	// save last text length
	draw_text_wdt = text_wdt;
}
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
