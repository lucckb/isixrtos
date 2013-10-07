/*
 * window.cpp
 *
 *  Created on: 7 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/window.hpp>
#include <gfx/gui/widget.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
// On repaint the widget return true when changed
bool window::repaint()
{
	dbprintf("window::repaint");
	bool res {};
	for( const auto item : m_widgets )
	{
		item->repaint();
	}
	return res;
}
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
}}

/* ------------------------------------------------------------------ */


