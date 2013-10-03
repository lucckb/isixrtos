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
bool button::repaint()
{
	auto gdi = make_gdi( get_layout() );
	auto &c = get_coord();
	gdi.fill_area( c.x(), c.y(), c.cx(), c.cy(), true );
	gdi.draw_text( c.x(), c.y(), m_caption.c_str());
	dbprintf("Repaint2");
	return 0;
}
/* ------------------------------------------------------------------ */
//* Report input event
bool button::report_event( const input::event_info& ev )
{
	dbprintf("report_event %i", ev.keyb.key);
	return true;
}
/* ------------------------------------------------------------------ */

} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
