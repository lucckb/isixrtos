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
	return false;
}
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx
/* ------------------------------------------------------------------ */
