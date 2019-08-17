/*
 * icon.cpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
 
#include <gfx/gui/icon.hpp>
 
namespace gfx {
namespace gui {

 
//Constructor
icon::icon( rectangle const& rect,layout const& layout ,window &win )
	: widget( rect, layout, win  )
{
}
 
//Repaint the window
void icon::repaint( bool /* focus */ )
{
	auto gdi = make_wgdi();
	const auto c = get_coord() + get_owner().get_coord();
	if( m_bitmap ) {
		const auto tx = c.x() + (c.cx() - m_bitmap->width)/2;
		const auto ty = c.y() + (c.cy() - m_bitmap->height)/2;
		gdi.draw_image(tx,ty, *m_bitmap );
	} else {
		gdi.fill_area( c.x(), c.y(), c.cx(), c.cy(), true );
	}
}

 
} /* namespace gui */
} /* namespace gfx */

 
