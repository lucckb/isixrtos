/*
 * widget.cpp
 *
 *  Created on: 1 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/widget.hpp>
#include <gfx/disp/gdi.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */

namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
// On repaint the widget return true when changed
bool widget::repaint()
{
	return false;
}
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx
/* ------------------------------------------------------------------ */
