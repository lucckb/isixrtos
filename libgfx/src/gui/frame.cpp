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
int frame::repaint_all()
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
	m_disp.clear(color::White);
	repaint_all();
	for( input::event_info ev;; )
	{
		if( m_events_queue.pop( ev ) == isix::ISIX_EOK )
		{
			for( const auto item : m_windows )
			{
				item->report_event( ev );
			}
		}
	}
}
/* ------------------------------------------------------------------ */
//Add window to frame
void frame::add_window( window* window )
{
	m_windows.push_back( window );
	repaint();
}

/* ------------------------------------------------------------------ */
//Delete the window
void frame::delete_window( window* window )
{
	m_windows.remove( window );
	repaint_all();
}

/* ------------------------------------------------------------------ */
//Repaint first windows
int frame::repaint()
{
	return m_windows.empty()?false:m_windows.front()->repaint();
}
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx
/* ------------------------------------------------------------------ */



