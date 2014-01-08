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
namespace
{

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
void window::repaint()
{
	const auto& lay = m_layout.inherit()?get_owner().get_def_win_layout():m_layout;
	disp::gdi gdi( get_owner().get_display(), lay.sel(), lay.bg(), lay.font() );
	{
		if( m_flags & flags::fill )
		{
			gdi.fill_area( m_coord.x(), m_coord.y(), m_coord.cx(), m_coord.cy(), true );
		}
		if( m_flags & flags::border )
		{
			draw_line_box( m_coord, gdi );
		}
	}
	for( const auto item : m_widgets ) {
		item->repaint();
	}
	//If border outside component is required
	if( m_flags & flags::selectborder )
	{
		{
			const auto s = (*m_current_widget)->get_coord() + get_coord() + 1;
			draw_line_box( s, gdi );
		}
		if( m_redraw_widget != m_widgets.end() )
		{
			const auto s = (*m_redraw_widget)->get_coord() + get_coord() + 1;
			disp::gdi gdic( get_owner().get_display(), (m_flags&flags::fill)?(lay.bg()):(color_t(color::Black)) );
			draw_line_box( s, gdic );
		}
	}
}

/* ------------------------------------------------------------------ */
//Report event
bool window::report_event( const input::event_info& ev )
{
	//Emit signal to others
	bool ret = emit( event( this, ev ) );
	ret |= (*m_current_widget)->report_event( ev );
	return  ret; 
}
/* ------------------------------------------------------------------ */
//Select next component
void window::select_next()
{
	if( m_current_widget != m_widgets.end() )
	{
		m_redraw_widget = m_current_widget;
		for( size_t s=m_widgets.size(),i=0; i<s; ++s )
		{
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
		for( size_t s=m_widgets.size(),i=0; i<s; ++s )
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
	if( w->selectable() )
		m_current_widget = m_widgets.begin();
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


