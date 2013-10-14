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
#include <gfx/drivers/disp/disp_base.hpp>
#include <isix.h>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
/** Execute gui main loop */
void frame::execute()
{
	m_disp.clear(color::Black);
	repaint();
	for( input::event_info ev;; )
	{
		bool need_repaint {};
		if( m_events_queue.pop( ev ) == isix::ISIX_EOK )
		{
			need_repaint = m_windows.empty()?false:m_windows.front()->report_event( ev );
		}
		if( need_repaint )
		{
			const auto tbeg = isix::isix_get_jiffies();
			repaint();
			dbprintf("Repaint time %i", isix::isix_get_jiffies()-tbeg);
		}
	}
}
/* ------------------------------------------------------------------ */
//Add widget to frame
void frame::add_window( window* window )
{
	m_windows.push_back( window );
	repaint();
}

/* ------------------------------------------------------------------ */
//Delete the widget
void frame::delete_window( window* window )
{
	m_windows.remove( window );
	repaint();
}

/* ------------------------------------------------------------------ */
/** Send gui event handler */
int frame::report_event( const input::event_info &event )
{
	return m_events_queue.push_isr( event );
}
/* ------------------------------------------------------------------ */
//Repaint first windows
void frame::repaint()
{
	if( ! m_windows.empty() )
		m_windows.front()->repaint();
}
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx
/* ------------------------------------------------------------------ */



