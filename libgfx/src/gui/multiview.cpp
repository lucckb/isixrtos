/*
 : =====================================================================================
 *
 *       Filename:  multiview.cpp
 *
 *    Description:  Multi Edit Box based only on last line message
 *
 *        Version:  1.0
 *        Created:  05.01.2014 19:49:23
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *	 Backspace is only operational on the last line of the code
 * =====================================================================================
 */

#include <gfx/gui/multiview.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */ 
namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */ 
//! On repaint the widget return true when changed
void multiview::repaint( bool focus )
{
	if( focus ) {
		m_last_x = INVAL;
	}
	if( m_clear_req ) {
		gui_clear_box();
	} else {
		gui_add_line();
	}
	gui_draw_frame();
}
/* ------------------------------------------------------------------ */
//! GUI dram frame
void multiview::gui_draw_frame()
{
	constexpr auto luma = 64;
	constexpr auto luma2 = luma/2;
	auto gdi = make_wgdi( );
	const auto c = get_coord() + get_owner().get_coord();
	//FRM1
	gdi.bright_fg_color( -luma );
	gdi.draw_line(c.x(), c.y()+c.cy(), c.x()+c.cx(), c.y()+c.cy() );
	gdi.draw_line(c.x()+c.cx(), c.y()+1, c.x()+c.cx(), c.y()+c.cy() );
	//FRM2
	gdi.bright_fg_color( -luma2 );
	gdi.draw_line(c.x()+1, c.y()+c.cy()-1, c.x()+c.cx(), c.y()-1+c.cy() );
	gdi.draw_line(c.x()+c.cx()-1, c.y()+1, c.x()+c.cx()-1, c.y()+c.cy() );
	//FRM3
	gdi.bright_fg_color( -luma );
	gdi.draw_line(c.x()+1, c.y(), c.x()+c.cx()-1, c.y() );
	gdi.draw_line(c.x(), c.y(), c.x(), c.y()+c.cy()-2 );
	//Clear the data line
	m_line.clear();
}
/* ------------------------------------------------------------------ */ 
// Add gui line of text
void multiview::gui_add_line()
{
	auto gdi = make_gdi();
	const auto c = get_coord() + get_owner().get_coord();
	if( m_last_x == INVAL ) {
		m_last_x = c.x() + text_margin;
	}
	const auto yp = c.y() + c.cy() - 2 - gdi.get_text_height();
	//Draw test
	for( char ch : m_line ) {
		if( ch == '\r' ) {
			//Continue if the character is <CR>
			continue;
		} else if( ch == gfx::input::kbdcodes::backspace ) {
			auto pos = m_last_x - m_char_width;
			if( pos >= 0 ) {
				gdi.fill_area( pos, yp, m_char_width, gdi.get_text_height(), true );
				m_last_x = pos;
			}
		}
		if( m_last_x + gdi.get_text_width(ch)>=c.x()+c.cx()-text_margin*2 || ch=='\n' ) {
			gdi.scroll( 
					c.x()+text_margin, c.y(), c.cx()-text_margin*2, 
					c.cy()-1, gdi.get_text_height(), get_owner().get_layout().bg() 
			);
			m_last_x = c.x() + text_margin;
		} 
		if( std::isprint( ch ) ) {
			if( m_alternate_color ) {
				gdi.set_fg_color( get_layout().sel() );
			}
			const auto npos = gdi.draw_text( m_last_x , yp, ch );
			m_char_width = (m_last_x!=INVAL)?(npos - m_last_x):(npos);
			m_last_x = npos;
		}
	}
}

/* ------------------------------------------------------------------ */
// Clear gui BOX
void multiview::gui_clear_box()
{
	auto gdi = make_gdi( );
	const auto c = get_coord() + get_owner().get_coord();
	gdi.fill_area(c.x()+1, c.y()+1, c.cx()-2, c.cy()-2, true );
	m_clear_req = false;
	m_last_x = INVAL;
}
/* ------------------------------------------------------------------ */ 
} //ns gui
} //ns gfx
/* ------------------------------------------------------------------ */ 

