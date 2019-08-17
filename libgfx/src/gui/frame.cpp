/*
 * frame.cpp
 *
 *  Created on: 1 paź 2013
 *      Author: lucck
 */
 
#include <gfx/gui/frame.hpp>
#include <gfx/input/event_info.hpp>
#include <gfx/gui/window.hpp>
#include <gfx/drivers/disp/disp_base.hpp>
#include <isix.h>
#include <foundation/sys/dbglog.h>
#include <algorithm>
 
namespace gfx {
namespace gui {
 
/** Execute gui main loop */
void frame::execute()
{
	using evinfo = input::event_info;
	m_disp.clear( color::Black );

//	repaint( true , nullptr , false );

	for( evinfo ev;; ) {
		window* rpt_wnd = nullptr;
		if( m_events_queue.pop( ev ) == ISIX_EOK )
		{
			if( ev.type == evinfo::EV_PAINT ) {
				// EV_PAINT argument is not dispatched to the component like other events
				// After the execution event is dropped
				repaint( ev.paint.force, ev.window, ev.paint.clrbg );
				continue;
			}
			//! Emit the info on the frame level
			emit( event( this, ev ) );
			//! Window handling event
			if( ev.type == evinfo::EV_WINDOW && ev.window != nullptr ) {
				rpt_wnd = ev.window;
			}
			{
				isix::sem_lock _lck( m_lock );
				if( rpt_wnd == nullptr && !m_windows.empty() ) {
					rpt_wnd = m_windows.back();
				}
			}
			//! Dispatch hotplug event to all widows
			if( ev.type==evinfo::EV_HOTPLUG ) {
				rpt_wnd = nullptr;
				isix::sem_lock _lck( m_lock );
				for( const auto item : m_windows ) {
					item->report_event( ev );
				}
			}
			if( ev.type==evinfo::EV_TIMER && !ev.window ) {
				rpt_wnd = nullptr;
				isix::sem_lock _lck( m_lock );
				for( const auto item : m_windows ) {
					item->report_event( ev );
				}
			}
			if( rpt_wnd ) {
				rpt_wnd->report_event( ev );
			}
		}
		{
			repaint( false, rpt_wnd, false );
		}
	}
}
 
//Add widget to frame
void frame::add_window( window* window )
{
	isix::sem_lock _lck( m_lock );
	m_windows.push_back( window );
	//queue_repaint( true, nullptr, true );
}
 
//Delete the widget
void frame::delete_window( window* window )
{
	isix::sem_lock _lck( m_lock );
	m_windows.remove( window );
	//queue_repaint( true, nullptr, true );
}
 
/** Refresh frame manual requirement */
int frame::queue_repaint( bool force, window* wnd, bool force_clr )
{
	gfx::input::event_info ei  {
		isix_get_jiffies(),
		gfx::input::event_info::evtype::EV_PAINT,
		wnd,
		{}
	};
	ei.paint = { force, force_clr };
	return report_event( ei );
}
 
/** Send gui event handler */
int frame::report_event( const input::event_info &event )
{
	return m_events_queue.push_isr( event );
}
 
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
 
//Focus on the window
int frame::set_focus( window* win, window* back_win )
{
	isix::sem_lock _lck( m_lock );
	auto elem = std::find_if( std::begin(m_windows), std::end(m_windows),
			[&]( const window* w ) { return w == win; } );
	if( elem != m_windows.end() ) {
		m_windows.erase( elem );
		if( back_win ) {
			m_win_stack.push( back_win );
		}
		m_windows.push_back( *elem );
		queue_repaint( true, win, true );
		return error::success;
	} else {
		dbprintf("ERROR: Window %p not found", win );
		return error::wnd_not_found;
	}
}
 
}	//ns gui
}	//ns gfx
 



