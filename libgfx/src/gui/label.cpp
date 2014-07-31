/*
 * label.cpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/label.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
//Constructor
label::label( rectangle const& rect,layout const& layout ,
		window &win, unsigned flags )
	: widget( rect, layout, win, false ), 
	  draw_text_wdt(0), m_flags(flags)
{
}
/* ------------------------------------------------------------------ */
//Repaint the lable
void label::repaint( bool /* focus */ )
{
	const auto c = get_coord() + get_owner().get_coord();
	auto gdi = make_gdi();
	if( m_sel_color ) {
		gdi.set_fg_color( get_layout().sel() );
	}
	const auto ty = c.y() + (c.cy() - gdi.get_text_height())/2;
	const auto tx = (m_flags&flags::center)?
			( c.x() + (c.cx() - gdi.get_text_width(m_caption.c_str()))/2 ):
			( c.x() );
	auto text_wdt = 0;
	// draw text
	if( !m_caption.empty() )
		text_wdt = gdi.draw_text( tx, ty, m_caption.c_str() );

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
