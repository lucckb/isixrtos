/*
 * frame.cpp
 *
 *  Created on: 1 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/frame.hpp>
#include <gfx/input/event_info.hpp>
#include <gfx/gui/window.hpp>
#include <isix.h>
/* ------------------------------------------------------------------ */

namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
/* Repaint the all windows */
int frame::repaint()
{
	for( const auto item : m_windows )
	{
		item->repaint();
	}
	return 0;
}
/* ------------------------------------------------------------------ */
/** Execute gui main loop */
void frame::execute()
{
	for(input::event_info ev;;)
	{
		if( m_events_queue.pop( ev ) == isix::ISIX_EOK )
		{
			//TODO Queue handle
		}
	}
}
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx
/* ------------------------------------------------------------------ */



