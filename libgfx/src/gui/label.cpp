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
	: widget( rect, layout, win, false )
{
}
/* ------------------------------------------------------------------ */
//Repaint the lable
void label::repaint()
{
	if( !m_caption.empty() )
	{
		const auto c = get_coord() + get_owner().get_coord();
		auto gdi = make_wgdi();
		const auto ty = c.y() + (c.cy() - gdi.get_text_height())/2;
		gdi.draw_text( c.x(), ty, m_caption.c_str() );
	}
}
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
