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
#include <foundation/sys/dbglog.h>
  
namespace gfx {
namespace gui {
 
namespace {
	// remove backspaces from a std::string
	void remove_backspaces( std::string& s )
	{
	std::string::size_type i;
	while ((i = s.find ('\b')) != std::string::npos)
		if (i == 0)
		s.erase (0, 1); // backspace at start, just erase it
		else
		s.erase (i - 1, 2); // erase character before backspace (and backspace)
	} // end of removeBackspaces
}
  
//! On repaint the widget return true when changed
void multiview::repaint( bool focus )
{
	if( focus ) {
		m_last_x = INVAL;
	}
	if( m_clear_req ) {
		gui_clear_box();
	} else {
		if( focus ) {
			gui_all_lines();
		} else {
			gui_add_line();
		}
	}
	gui_draw_frame();
}
 
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
	for( auto ch : m_line ) {
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
 
// Clear gui BOX
void multiview::gui_clear_box()
{
	auto gdi = make_gdi( );
	const auto c = get_coord() + get_owner().get_coord();
	gdi.fill_area(c.x()+1, c.y()+1, c.cx()-2, c.cy()-2, true );
	m_clear_req = false;
	m_last_x = INVAL;
}
 
//! Called when whole window should be repaint
void multiview::gui_all_lines()
{
	remove_backspaces( m_line );
	auto gdi = make_gdi();
	const auto c = get_coord() + get_owner().get_coord();
	const auto max_x = c.x() + c.cx() - text_margin * 2;
	const auto min_x = c.x() + text_margin;
	const auto txth = gdi.get_text_height();
	const auto max_h = c.cy() - 2;
	//! Calculate numer required lines
	int xc { max_x };
	int yc { max_h - txth };
	int lc { 0 };
	auto it = m_line.size()-1;
	for( ; it>0; --it ) {
		const auto ch = m_line[it];
		if( ch == '\n' ) {
			xc =  max_x;
			yc -= txth;
			++lc;
		} else if( ch == '\r' ) {
			continue;
		} else if( std::isprint( ch ) ) {
			const auto cw = gdi.get_text_width( ch );
			xc -= cw;
			if( xc < min_x ) {
				yc -= txth;
				xc = max_x-cw;
				++lc;
			}
		}
		if( yc <= txth  ) {
			break;
		}
	}
	//! Draw begin from valid line 
	yc = c.y() + max_h - txth - lc*txth;
	xc = min_x;
	if( xc + gdi.get_text_width(m_line[it]) >= max_x ) {
		yc += txth;
		xc = min_x;
	}
	for( unsigned i=it; i<m_line.size(); ++i) {
		const auto ch = m_line[i];
		if( ch == '\n' ) {
			xc =  min_x;
			yc += txth;
		} else if( ch == '\r' ) {
			continue;
		} else if( std::isprint( ch ) ) {
			if( xc + gdi.get_text_width(ch) >= max_x ) {
				yc += txth;
				xc = min_x;
			}
			xc = gdi.draw_text( xc, yc, ch );
		}
	}
	m_last_x = xc;
}
  
} //ns gui
} //ns gfx
  

