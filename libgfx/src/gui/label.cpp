/*
 * label.cpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
 
#include <gfx/gui/label.hpp>
#include <foundation/sys/dbglog.h>
 
namespace gfx {
namespace gui {
 
//Constructor
label::label( rectangle const& rect,layout const& layout ,
		window &win, unsigned flags )
	: widget( rect, layout, win, flags & flags::select ),
	  m_flags(flags)
{
}
 
//Repaint the lable
void label::repaint( bool /* focus */ )
{
	const auto c = get_coord() + get_owner().get_coord();
	auto gdi = make_gdi();
	if( m_sel_color ) {
		gdi.set_fg_color( get_layout().sel() );
	}
	const auto fh = gdi.get_text_height();
	const auto ty = c.y() + (c.cy() - fh)/2;
	const auto tx = (m_flags&flags::center)?
			( c.x() + (c.cx() - gdi.get_text_width(m_caption.c_str()))/2 ):
			( c.x() );

	// draw text
	auto text_px = gdi.draw_text(tx, ty, m_caption.c_str());

	// clear background part
	if (!(m_flags & flags::center))
	{
		if (text_px < text_px_old)
		{
			gdi.fill_area(text_px, ty , text_px_old - text_px, fh, true);
		}
	}

	// save last text length
	text_px_old = text_px;
}
 
} /* namespace gui */
} /* namespace gfx */
 
