/* ------------------------------------------------------------------ */
/*
 * editbox.cpp
 *
 *  Created on: 14 paź 2013
 *      Author: lucck
 *      TODO: Cursor change improvement
 *      TODO: Set min and max window limit
 */
/* ------------------------------------------------------------------ */
#include <gfx/gui/editbox.hpp>
#include <foundation/dbglog.h>

/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
// On repaint the widget return true when changed
void editbox::repaint()
{
	constexpr auto luma = 128;
	constexpr auto luma2 = luma/2;
	auto gdi = make_wgdi( );
	const auto c = get_coord() + get_owner().get_coord();
	const auto ty = c.y() + (c.cy() - gdi.get_text_height())/2;
	gdi.fill_area( c.x()+1, c.y()+1, c.cx()-2, c.cy()-2, true );
	//Draw after the cursor
	auto x = m_cursor_x;
	for(auto it = m_value.begin()+m_cursor_pos; it!=m_value.end(); ++it )
	{
		if( x + gdi.get_text_width(*it) >= c.x()+c.cx()-text_margin*2 )
			break;
		x = gdi.draw_text( x , ty, *it );
	}
	//Draw before the cursor
	x = m_cursor_x;
	for(auto it = m_value.begin()+m_cursor_pos-1; m_cursor_pos>0&&it>=m_value.begin(); --it )
	{
		const auto tw = gdi.get_text_width(*it);
		if( x - tw < c.x()+text_margin )
			break;
		x -= tw;
		gdi.draw_text( x , ty, *it );
	}
	//Draw cursor
	gdi.draw_line( m_cursor_x, c.y()+text_margin, m_cursor_x, c.y()+c.cy()-4 );

	//Draw cursor
	//FRM1
	gdi.set_fg_color( colorspace::brigh( get_layout().bg(), -luma ) );
	gdi.draw_line(c.x(), c.y()+c.cy(), c.x()+c.cx(), c.y()+c.cy() );
	gdi.draw_line(c.x()+c.cx(), c.y()+1, c.x()+c.cx(), c.y()+c.cy() );
	//FRM2
	gdi.set_fg_color( colorspace::brigh( get_layout().bg(), -luma2 ) );
	gdi.draw_line(c.x()+1, c.y()+c.cy()-1, c.x()+c.cx(), c.y()-1+c.cy() );
	gdi.draw_line(c.x()+c.cx()-1, c.y()+1, c.x()+c.cx()-1, c.y()+c.cy() );
	//FRM3
	gdi.set_fg_color( colorspace::brigh( get_layout().bg(), luma ) );
	gdi.draw_line(c.x()+1, c.y(), c.x()+c.cx()-1, c.y() );
	gdi.draw_line(c.x(), c.y(), c.x(), c.y()+c.cy()-2 );
}
/* ------------------------------------------------------------------ */
//* Report input event
bool editbox::report_event( const input::event_info& ev )
{
	bool ret {};
	if(ev.type == event::evtype::EV_KEY )
	{
		if( m_kbdmode == kbd_mode::joy )
			ret = handle_joy( ev.keyb );
		else if( m_kbdmode == kbd_mode::joy )
			ret = handle_qwerty( ev.keyb );
		else
		{
			dbprintf("Unknown kbd mode %i", m_kbdmode );
		}
	}
	else
	{
		dbprintf("Unhandled event type %i", ev.type );
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//Handle joy KBD
bool editbox::handle_joy( const input::detail::keyboard_tag& evk )
{
	using namespace gfx::input;
	bool ret {};
	if( evk.stat == input::detail::keyboard_tag::status::DOWN && !m_readonly)
	{
		const auto c = get_coord() + get_owner().get_coord();
		if( evk.key == kbdcodes::os_arrow_right )
		{
			if( m_cursor_pos<m_value.size() )
			{
				++m_cursor_pos;
				auto gdi = make_wgdi( );
				const auto new_cur_x =  gdi.get_text_width( m_value[m_cursor_pos]) + m_cursor_x;
				if( new_cur_x  < c.x()+c.cx()-text_margin*2)
					m_cursor_x = new_cur_x;
			}
			else
			{
				m_cursor_pos = 0;
				m_cursor_x = c.x()+text_margin;
			}
			ret = true;
		}
		else if( evk.key == kbdcodes::os_arrow_left )
		{
			m_value.insert(m_cursor_pos, 1, insert_ch());
		}
		if( evk.key == kbdcodes::os_arrow_up )
		{
			m_value[m_cursor_pos] = ch_inc( m_value[m_cursor_pos] );
			ret = true;
		}
		else if( evk.key == kbdcodes::os_arrow_down )
		{
			m_value[m_cursor_pos] = ch_dec( m_value[m_cursor_pos] );
			ret = true;
		}
		else if( evk.key == kbdcodes::enter)
		{
			event btn_event( this, event::evtype::EV_CLICK );
			ret |= emit( btn_event );
		}
		if( ret )
		{
			event btn_event( this, event::evtype::EV_CHANGE );
			ret |= emit( btn_event );
		}
	}
	return ret;
}

/* ------------------------------------------------------------------ */
//Get insert char
char editbox::insert_ch()
{
	switch( m_type )
	{
		default:
			return '!';
		case type::float_pos:
		case type::float_neg:
			return '.';
		case type::integer_pos:
		case type::integer_neg:
			return '0';
	}
}
/* ------------------------------------------------------------------ */
//Validate is character contains the criteria
char editbox::ch_inc( char ch ) const
{
	switch( m_type )
	{
	case type::text:
		if( ++ch == 0 ) ch = ' ';
		break;
	case type::integer_pos:
		if(ch>='0' && ch<'9') ch++;
		break;
	case type::integer_neg:
		if(ch>='0' && ch<'9') ch++;
		else if( ch=='9') ch = '-';
		else if( ch=='-') ch = '0';
		break;
	case type::float_pos:
		if(ch>='0' && ch<'9') ch++;
		else if( ch=='9') ch = '.';
		else if( ch=='.') ch = 'E';
		else if( ch=='E') ch = '0';
		break;
	case type::float_neg:
		if(ch>='0' && ch<'9') ch++;
		else if( ch=='9') ch = '-';
		else if( ch=='-') ch = '.';
		else if( ch=='.') ch = 'E';
		else if( ch=='E') ch = '0';
		break;
	}
	return ch;
}
/* ------------------------------------------------------------------ */
//Validate is character contains the criteria
char editbox::ch_dec( char ch ) const
{
	switch( m_type )
	{
	case type::text:
		if( --ch == 0 ) ch = ' ';
		break;
	case type::integer_pos:
		if(ch>'0' && ch<='9') ch--;
		break;
	case type::integer_neg:
		if(ch>'0' && ch<='9') ch--;
		else if( ch=='0') ch='-';
		else if( ch=='-') ch='9';
	case type::float_pos:
		if(ch>'0' && ch<='9') ch--;
		else if( ch=='0') ch = 'E';
		else if( ch=='E') ch = '.';
		else if( ch=='.') ch = '0';
	case type::float_neg:
		if(ch>'0' && ch<='9') ch--;
		else if( ch=='0') ch = 'E';
		else if( ch=='E') ch = '.';
		else if( ch=='.') ch = '-';
		else if( ch=='-') ch = '0';
		break;
	}
	return ch;
}
/* ------------------------------------------------------------------ */
//Handle querty KBD
bool editbox::handle_qwerty( const input::detail::keyboard_tag& /*evk*/ )
{
	dbprintf("Querty mode not suported yet");
	return false;
}
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
