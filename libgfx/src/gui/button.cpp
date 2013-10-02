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
	gdi().fill_area(get_x(), get_y(), get_cx(), get_y(), true );
	gdi().draw_text(get_x(), get_y(), m_caption.c_str());
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
