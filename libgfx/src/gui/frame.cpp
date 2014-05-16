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
#include <algorithm>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
/** Execute gui main loop */
void frame::execute()
{
	m_disp.clear(color::Black);
	repaint( true , true );
	for( input::event_info ev;; )
	{
		bool force = false;
		if( m_events_queue.pop( ev ) == isix::ISIX_EOK )
		{
			if( ev.type == input::event_info::evtype::EV_CHANGE && ev.target != nullptr )  {
				auto tgtwin = reinterpret_cast<window*>( ev.target );
				tgtwin->report_event( ev );
				force = true;
			} else if( !m_windows.empty() ) {
				m_windows.front()->report_event( ev );
			}
		}
		{
			const auto tbeg = isix::isix_get_jiffies();
			repaint( force , false );
			dbprintf("Repaint time %i", isix::isix_get_jiffies()-tbeg);
		}
	}
}
/* ------------------------------------------------------------------ */
//Add widget to frame
void frame::add_window( window* window )
{
	m_windows.push_back( window );
	repaint( true, true, true );
}

/* ------------------------------------------------------------------ */
//Delete the widget
void frame::delete_window( window* window )
{
	m_windows.remove( window );
	repaint( true, true, true );
}
/* ------------------------------------------------------------------ */ 
/** Refresh frame manual requirement */
int frame::update( window* target_win )
{
	const gfx::input::event_info ei  {
		isix::isix_get_jiffies(),
		gfx::input::event_info::evtype::EV_CHANGE,
		{ .target = target_win }
	};
	return report_event( ei );
}
/* ------------------------------------------------------------------ */
/** Send gui event handler */
int frame::report_event( const input::event_info &event )
{
	return m_events_queue.push_isr( event );
}
/* ------------------------------------------------------------------ */
//Repaint first windows
void frame::repaint( bool force, bool all, bool force_clr )
{
	if( all ) {
		for( const auto item : m_windows ) {
			item->repaint( force , force_clr );
		}
	} else {
		if( ! m_windows.empty() )
			m_windows.front()->repaint( force, force_clr );
	}
}
/* ------------------------------------------------------------------ */
//Focus on the window
int frame::set_focus( window* win )
{
	auto elem = std::find_if( std::begin(m_windows), std::end(m_windows), 
			[&]( const window* w ) { return w == win; } );
	if( elem != m_windows.end() ) {
		m_windows.erase( elem );
		m_windows.push_front( *elem );
		repaint( true, true, true );
		return errno::success;
	} else {
		dbprintf("ERROR: Window %p not found", win );
		return errno::wnd_not_found;
	}
}
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx
/* ------------------------------------------------------------------ */



