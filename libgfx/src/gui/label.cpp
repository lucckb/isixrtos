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
	auto gdi = make_gdi( );
	const auto c = get_coord() + get_owner().get_coord();
	gdi.draw_text(c.x(), c.y(), "DUPA");
}

/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
