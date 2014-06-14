/*
 * window.cpp
 *
 *  Created on: 7 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/window.hpp>
#include <gfx/gui/widget.hpp>
#include <gfx/input/event_info.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
namespace {
	//Draw the single line box
	void draw_line_box( const rectangle &r, disp::gdi &gdi )
	{
		gdi.draw_line(r.x(),r.y(),r.x()+r.cx(), r.y() );
		gdi.draw_line(r.x(),r.y()+r.cy(),r.x()+r.cx(), r.y()+r.cy() );
		gdi.draw_line(r.x(),r.y(),r.x(), r.y()+r.cy() );
		gdi.draw_line(r.x()+r.cx(),r.y(),r.x()+r.cx(), r.y()+r.cy() );
	}
}
/* ------------------------------------------------------------------ */
// On repaint the widget return true when changed
void window::repaint( bool force, bool force_clr )
{
	force |= m_changed;
	const auto& lay = m_layout.inherit()?get_owner().get_def_win_layout():m_layout;
	disp::gdi gdi( get_owner().get_display(), lay.sel(), lay.bg(), lay.font() );
	{
		//NOTE: Force clear is reqired when one windows
		// is replaced with another for example on focus 
		// set and changing
		if( (m_flags & flags::fill) && force_clr ) {
			gdi.fill_area( m_coord.x(), m_coord.y(), m_coord.cx(), m_coord.cy(), true );
		}
		if( m_flags & flags::border ) {
			draw_line_box( m_coord, gdi );
		}
	}
	for( const auto item : m_widgets ) {
		item->redraw( force );
	}
	//If border outside component is required
	if( m_flags & flags::selectborder ) {
		{
			const auto s = (*m_current_widget)->get_coord() + get_coord() ;
			//DBG END
			draw_line_box( s, gdi );
		}
		if( m_redraw_widget != m_widgets.end() ) {
			const auto s = (*m_redraw_widget)->get_coord() + get_coord() + 1;
			disp::gdi gdic( get_owner().get_display(), (m_flags&flags::fill)?(lay.bg()):(color_t(color::Black)) );
			draw_line_box( s, gdic );
		}
	}
}

/* ------------------------------------------------------------------ */
//Report event
void window::report_event( const input::event_info& ev )
{
	using evinfo = input::event_info;
	//! Emit signal to the to the window callbacks
	m_changed = emit( event( this, ev ) );
	//  Don't dispatch hotplug event to to the
	//  widgets only only windows got this event
	//  it should be dispatched
	if( ev.type == evinfo::EV_HOTPLUG ) {
		return;
	}
	if( ev.type != evinfo::EV_WINDOW ) {
		auto widget = current_widget();
		if( widget )
			widget->report_event( ev );
	} else {
		for( const auto& widget : m_widgets ) {
			widget->report_event( ev );
		}
	}
	//! FIXME: Debug only
	if( ev.type == evinfo::EV_KEY ) {
		dbprintf( "Key %02x Scan %02x %s", ev.keyb.key, ev.keyb.scan, (bool(ev.keyb.stat)?"UP":"DOWN") );
	}
}
/* ------------------------------------------------------------------ */
//Select next component
void window::select_next()
{
	if( m_current_widget != m_widgets.end() ) {
		m_redraw_widget = m_current_widget;
		for( size_t s=m_widgets.size(),i=0; i<s; ++i ) {
			if( ++m_current_widget == m_widgets.end() )
				m_current_widget = m_widgets.begin();
			if( (*m_current_widget)->selectable() )
				break;
		}
	}
}
/* ------------------------------------------------------------------ */
//Select prev component
void window::select_prev()
{
	if( m_current_widget != m_widgets.end() )
	{
		m_redraw_widget = m_current_widget;
		for( size_t s=m_widgets.size(),i=0; i<s; ++i )
		{
			if( m_current_widget == m_widgets.begin() )
				m_current_widget = --m_widgets.end();
			else
				--m_current_widget;
			if( (*m_current_widget)->selectable() )
				break;
		}
	}
}
/* ------------------------------------------------------------------ */
void window::add_widget( widget * const w )
{
	m_widgets.push_front( w );
	if( w->selectable() ) {
		m_current_widget = m_widgets.begin();
	}
}
/* ------------------------------------------------------------------ */
void window::delete_widget( widget * const w )
{
	m_widgets.remove( w );
	m_current_widget = m_widgets.empty()||!w->selectable()?m_widgets.end():m_widgets.begin();
}
/* ------------------------------------------------------------------ */
}}

/* ------------------------------------------------------------------ */


