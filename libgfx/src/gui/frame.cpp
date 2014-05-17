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
	//TODO: Replace by semaphore guard (but now we are not using exception
	//so it should be save if don't use sem guards
	using evinfo = input::event_info;
	m_disp.clear( color::Black );
	repaint( true , nullptr , false );
	for( evinfo ev;; )
	{
		window* rpt_wnd = nullptr;
		if( m_events_queue.pop( ev ) == isix::ISIX_EOK ) {
			m_lock.wait( isix::ISIX_TIME_INFINITE );
			if( ev.type == evinfo::EV_PAINT ) {
				// EV_PAINT argument is not dispatched to the component like other events
				// After the execution event is dropped
				repaint( ev.paint.force, ev.window, ev.paint.clrbg );
				m_lock.signal();
				continue;
			}
			if( ev.type == evinfo::EV_WINDOW && ev.window != nullptr ) {
				rpt_wnd = ev.window;
			}
			if( rpt_wnd == nullptr && !m_windows.empty() ) {
				rpt_wnd = m_windows.front();
			}
			if( rpt_wnd ) {
				rpt_wnd->report_event( ev );
			}
		}
		{
			const auto tbeg = isix::isix_get_jiffies();
			repaint( false, rpt_wnd, false );
			dbprintf("Repaint time %i", isix::isix_get_jiffies()-tbeg);
		}
		m_lock.signal();
	}
}
/* ------------------------------------------------------------------ */
//Add widget to frame
void frame::add_window( window* window )
{
	m_lock.wait( isix::ISIX_TIME_INFINITE );
	m_windows.push_front( window );
	m_lock.signal( );
	queue_repaint( true, nullptr, true );
}
/* ------------------------------------------------------------------ */
//Delete the widget
void frame::delete_window( window* window )
{
	m_lock.wait( isix::ISIX_TIME_INFINITE );
	m_windows.remove( window );
	m_lock.signal( );
	queue_repaint( true, nullptr, true );
}
/* ------------------------------------------------------------------ */ 
/** Refresh frame manual requirement */
int frame::queue_repaint( bool force, window* wnd, bool force_clr )
{
	gfx::input::event_info ei  {
		isix::isix_get_jiffies(),
		gfx::input::event_info::evtype::EV_PAINT,
		wnd,
		{}
	};
	ei.paint = { force, force_clr };
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
void frame::repaint( bool force, window *wnd, bool force_clr )
{
	if( wnd == nullptr ) {
		for( const auto item : m_windows ) {
			item->repaint( force , force_clr );
		}
	} else {
		wnd->repaint( force, force_clr );
	}
}
/* ------------------------------------------------------------------ */
//Focus on the window
int frame::set_focus( window* win )
{
	m_lock.wait( isix::ISIX_TIME_INFINITE );
	auto elem = std::find_if( std::begin(m_windows), std::end(m_windows), 
			[&]( const window* w ) { return w == win; } );
	if( elem != m_windows.end() ) {
		m_windows.erase( elem );
		m_windows.push_front( *elem );
		queue_repaint( true, nullptr, true );
		m_lock.signal();
		return errno::success;
	} else {
		dbprintf("ERROR: Window %p not found", win );
		m_lock.signal();
		return errno::wnd_not_found;
	}
}
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx
/* ------------------------------------------------------------------ */



