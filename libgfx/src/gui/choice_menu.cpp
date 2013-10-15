/*
 * choice_menu.cpp
 *
 *  Created on: 11 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/choice_menu.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
//Constructor
choice_menu::choice_menu( rectangle const& rect,
	layout const& layout ,window &win, style style )
	: widget( rect, layout, win ), m_style(style),
	  m_max_box_items(calc_max_items())
{
}

/* ------------------------------------------------------------------ */
//Calculate max number of items using current font
int choice_menu::calc_max_items() const
{
	const auto lay = get_layout();
	const auto coo = get_coord();
	return (coo.cy()-y_margin) / lay.font()->height;
}
/* ------------------------------------------------------------------ */
//Repaint virtual function
void choice_menu::repaint()
{
	if( m_items )
	{
		auto gdi_sel = make_wgdi( );
		auto gdi_nsel = make_wgdi( );
		gdi_nsel.set_bg_color( get_owner().get_layout().bg() );
		gdi_sel.set_bg_color( get_layout().sel() );
		const auto coo = get_coord() + get_owner().get_coord();
		const auto ymul = gdi_sel.get_text_height();
		const auto ystart = coo.y() + y_margin;
		const auto boxpos = m_curr_item>m_max_box_items-1?m_max_box_items-1:m_curr_item;
		for( int c=0,s=m_curr_item-boxpos>0?m_curr_item-boxpos:0;
			  c<m_max_box_items; ++c,++s )
		{
			auto &gdi = m_curr_item==s?gdi_sel:gdi_nsel;
			const auto y = ystart + c * ymul;
			auto x = coo.x()+x_margin_left;
			if( m_style == style::select )
			{
				gdi.fill_area( x, y, ymul, ymul, true );
				x+= gdi_sel.get_text_height();
			}
			const auto xf = gdi.draw_text( x , y , m_items[s].second );
			gdi.fill_area( xf, y, coo.x()+coo.cx()-xf-x_margin_right, ymul, true );
			if( m_style == style::select )
			{
				gdi.set_fill(true);
				const auto r = gdi_sel.get_text_height() /2 ;
				const auto xc = coo.x()+x_margin_left + r;
				const auto yc = y + r;
				gdi.draw_circle( xc, yc, r - slider_space - 1 );
				if( m_sel_item == s )
				{
					auto lgdi = make_gdi( );
					lgdi.set_fill(true);
					lgdi.draw_circle(xc, yc, (r - slider_space - 1)/2 );
				}
			}
		}
	}
	//Draw frame corners
	{
		auto gdi = make_gdi( );
		auto gdiw = make_wgdi( );
		const auto c = get_coord() + get_owner().get_coord();
		//Left lines
		gdi.set_fg_color( get_layout().bg() );
		gdi.draw_line(c.x(), c.y()+1, c.x(), c.y()+c.cy()-2 );
		gdiw.draw_line(c.x()+1, c.y()+1,c.x()+1, c.y()+c.cy()-2 );
		//Bottom lines
		gdi.draw_line(c.x()+2, c.y()+c.cy()-2, c.x()+c.cx()-2, c.y()+c.cy()-2 );
		gdiw.draw_line(c.x()+1, c.y()+c.cy()-1, c.x()+c.cx()-1, c.y()+c.cy()-1 );
		//Top Line
		gdi.draw_line(c.x()+2, c.y()+1, c.x()+c.cx()-2, c.y()+1 );
		gdiw.draw_line(c.x()+1, c.y(), c.x()+c.cx()-2, c.y() );
		//Right line
		gdi.draw_line(c.x()+c.cx()-2, c.y()+1, c.x()+c.cx()-2, c.y()+c.cy()-2 );
		gdiw.draw_line(c.x()-1+c.cx(), c.y()+1,c.x()+c.cx()-1, c.y()+c.cy()-2 );
	}
	//Draw the slider
	{
		static constexpr coord_t min_slider = 10;
		const auto c = get_coord() + get_owner().get_coord();
		const auto twidth = c.cy() - y_margin*2;
		const auto sel_width_ =  m_max_box_items *( twidth ) / m_num_items;
		const auto sel_width = sel_width_<min_slider?min_slider:sel_width_;
		const auto ypos = (sel_width * m_curr_item ) / (m_num_items+1);
		auto gdi = make_gdi();
		constexpr auto luma = -128;
		gdi.set_fg_color( colorspace::brigh( get_layout().bg(), luma ) );
		gdi.fill_area(c.x()+c.cx()-x_margin_right+slider_space, c.y()+y_margin ,x_margin_right-slider_space-2 ,  twidth );
		//Selected part
		gdi.fill_area(c.x()+c.cx()-x_margin_right+slider_space, c.y()+y_margin+ypos ,x_margin_right-slider_space-2 , sel_width, true );
	}
}

/* ------------------------------------------------------------------ */
//Set menu items
void choice_menu::items( const item *items )
{
	m_items = items;
	m_num_items = 0;
	if( m_items )
	{
		for( size_t i=0; m_items[i].first; ++i, ++m_num_items );
	}
}
/* ------------------------------------------------------------------ */
//* Report input event
bool choice_menu::report_event( const input::event_info& ev )
{
	using namespace gfx::input;
	bool ret {};
	if( ev.keyb.stat == input::detail::keyboard_tag::status::DOWN )
	{
		if( ev.keyb.key == kbdcodes::os_arrow_down )
		{
			if(++m_curr_item == m_num_items) --m_curr_item;
			else ret = true;
		}
		else if( ev.keyb.key == kbdcodes::os_arrow_up )
		{
			if( --m_curr_item < 0 ) m_curr_item = 0;
			else ret = true;
		}
		else if(  ev.keyb.key == kbdcodes::enter )
		{
			if( m_style == style::select )
			{
				m_sel_item = m_curr_item;
			}
			event btn_event( this, event::evtype::EV_CLICK );
			ret = emit( event( this, ev));
		}
	}
	return ret;
}
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
