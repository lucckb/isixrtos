/*
 * choice_menu.cpp
 *
 *  Created on: 11 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/choice_menu.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
//Constructor
choice_menu::choice_menu( rectangle const& rect,
	layout const& layout ,window &win, style style )
	: widget( rect, layout, win ), m_style(style)
{
}

/* ------------------------------------------------------------------ */
//Repaint virtual function
void choice_menu::repaint()
{
	if( m_items )
	{
		auto gdi = make_gdi( );
		const auto c = get_coord() + get_owner().get_coord();
		for( const item *i=m_items; i->second; ++i )
		{
			gdi.draw_text(c.x(),c.y(),"DUPA");
		}
	}
}
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
