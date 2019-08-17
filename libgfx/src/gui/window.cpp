/*
 * window.cpp
 *
 *  Created on: 7 paź 2013
 *      Author: lucck
 */
 
#include <gfx/gui/window.hpp>
#include <gfx/gui/widget.hpp>
#include <gfx/input/event_info.hpp>
#include <foundation/sys/dbglog.h>
 
namespace gfx {
namespace gui {
 
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
 
// On repaint the widget return true when changed
void window::repaint( bool force, bool force_clr )
{
	if( force && has_focus() ) {
		event focus_event( this, event::evtype::EV_FOCUS );
		emit( focus_event );
	}
	const auto& lay = m_layout.inherit()?get_owner().get_def_win_layout():m_layout;
	disp::gdi gdi( get_owner().get_display(), lay.sel(), lay.bg(), lay.font() );
	if( force_clr ) {
		//NOTE: Force clear is reqired when one windows
		// is replaced with another for example on focus 
		// set and changing
		if( (m_flags & flags::fill) ) {
			gdi.fill_area( m_coord.x(), m_coord.y(), m_coord.cx(), m_coord.cy(), true );
		}
		//! Window border
		if( m_flags & flags::border ) {
			draw_line_box( m_coord, gdi );
		} else {
			if( m_flags & flags::fill ) {
				disp::gdi gdic( get_owner().get_display(), 
					(m_flags&flags::fill)?(lay.bg()):(color_t(color::Black)) 
				);
				draw_line_box( m_coord, gdic );
			}
		} 
	}
	for( const auto item : m_widgets ) {
		item->redraw( force, m_changed );
	}
	//If border outside component is required
	if( (m_flags & flags::selectborder) && has_focus() ) {
		{
			const auto s = (*m_current_widget)->get_coord() + get_coord() + 1;

			draw_line_box( s, gdi );

			if (m_border_style == style::double_border)
			{
				if ((s.x() > 0) && (s.y() > 0))
				{
					const auto s2 = rectangle(s.x() - 1, s.y() - 1, s.cx() + 2, s.cy() + 2);
					draw_line_box( s2, gdi );
				}
			}
		}
		if( m_redraw_widget != m_widgets.end() ) {
			const auto s = (*m_redraw_widget)->get_coord() + get_coord() + 1;
			disp::gdi gdic( get_owner().get_display(), (m_flags&flags::fill)?(lay.bg()):(color_t(color::Black)) );
			draw_line_box( s, gdic );

			if (m_border_style == style::double_border)
			{
				if ((s.x() > 0) && (s.y() > 0))
				{
					const auto s2 = rectangle(s.x() - 1, s.y() - 1, s.cx() + 2, s.cy() + 2);
					draw_line_box( s2, gdic );
				}
			}
		}
	}
}

 
//Report event
void window::report_event( const input::event_info& ev )
{
	using evinfo = input::event_info;
	//! Emit signal to the to the window callbacks
	m_changed = emit( event( this, ev ) );
	//  Don't dispatch hotplug event to to the
	//  widgets only only windows got this event
	//  it should be dispatched
	if( ev.type==evinfo::EV_HOTPLUG || ev.type==evinfo::EV_TIMER ) {
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
}
 
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
 
//Select prev component
void window::select_prev()
{
	if( m_current_widget != m_widgets.end() ) {
		m_redraw_widget = m_current_widget;
		for( size_t s=m_widgets.size(),i=0; i<s; ++i ) {
			if( m_current_widget == m_widgets.begin() )
				m_current_widget = --m_widgets.end();
			else
				--m_current_widget;
			if( (*m_current_widget)->selectable() )
				break;
		}
	}
}
 
//! Select wiget directly
void window::select( widget * const w ) 
{
	auto elem = std::find_if( std::begin(m_widgets), std::end(m_widgets), 
			[&]( const widget* wdg ) { return w == wdg; } );
	if( elem != m_widgets.end() && w->selectable() ) {
		if (m_current_widget != elem)
			m_redraw_widget = m_current_widget;
		else
			m_redraw_widget = m_widgets.end();

		m_current_widget = elem;
	}
}
 
void window::add_widget( widget* const w )
{
	m_widgets.push_back( w );
	if( m_current_widget==m_widgets.end() && w->selectable() ) {
		select( w );
	}
}
 
void window::delete_widget( widget* const w )
{
	m_widgets.remove( w );
	m_current_widget = m_widgets.empty()||!w->selectable()?m_widgets.end():m_widgets.begin();
}
 
}}

 


