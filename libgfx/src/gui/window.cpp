/*
 * window.cpp
 *
 *  Created on: 1 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/window.hpp>
#include <gfx/disp/gdi.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */

namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
// On repaint the window return true when changed
bool window::repaint()
{
	m_gdi.fill_area(m_x, m_y, m_cx, m_cy, true );
	dbprintf("Repaint");
	return true;
}
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx
/* ------------------------------------------------------------------ */
