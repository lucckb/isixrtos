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
// On repaint the widget return true when changed
bool window::repaint()
{
	dbprintf("window::repaint");
	{
		const auto& lay = m_layout.inherit()?get_owner().get_def_win_layout():m_layout;
		disp::gdi gdi( get_owner().get_display(), lay.sel(), lay.bg(), lay.font() );
		if( m_flags & flags::fill )
		{
			gdi.fill_area( m_coord.x(), m_coord.y(), m_coord.cx(), m_coord.cy(), true );
		}
		if( m_flags & flags::border )
		{
			gdi.draw_line(m_coord.x(),m_coord.y(),m_coord.x()+m_coord.cx(), m_coord.y() );
			gdi.draw_line(m_coord.x(),m_coord.y()+m_coord.cy(),m_coord.x()+m_coord.cx(), m_coord.y()+m_coord.cy() );
			gdi.draw_line(m_coord.x(),m_coord.y(),m_coord.x(), m_coord.y()+m_coord.cy() );
			gdi.draw_line(m_coord.x()+m_coord.cx(),m_coord.y(),m_coord.x()+m_coord.cx(), m_coord.y()+m_coord.cy() );
		}
	}
	bool res {};
	for( const auto item : m_widgets )
	{
		item->repaint();
	}
	return res;
}
/* ------------------------------------------------------------------ */
//Report event
bool window::report_event( const input::event_info& ev )
{
	//emit( event( this, ev ) );
	return (*m_current_widget)->report_event( ev );
}
/* ------------------------------------------------------------------ */
//Select next component
void window::select_next()
{
	if( ++m_current_widget == m_widgets.end() )
		m_current_widget = m_widgets.begin();

}
/* ------------------------------------------------------------------ */
//Select prev component
void window::select_prev()
{
	if( m_current_widget == m_widgets.begin() )
		m_current_widget = m_widgets.end();
	else
		--m_current_widget;
}
/* ------------------------------------------------------------------ */
}}

/* ------------------------------------------------------------------ */


