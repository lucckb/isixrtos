 
/*
 * editbox.cpp
 *
 *  Created on: 14 paź 2013
 *      Author: lucck
 *      TODO: Cursor change improvement
 *      TODO: Set min and max window limit
 */
 
#include <gfx/gui/editbox.hpp>
#include <foundation/sys/dbglog.h>
#include <foundation/algo/utils.h>
#include <cstdlib>
 
namespace gfx {
namespace gui {
 
//! On repaint the widget return true when changed
void editbox::repaint(  bool /* focus */  )
{
	constexpr auto luma = 128;
	constexpr auto luma2 = luma/2;
	auto gdi = make_wgdi( );
	const auto c = get_coord() + get_owner().get_coord();
	const auto ty = c.y() + (c.cy() - gdi.get_text_height())/2;
	gdi.fill_area( c.x()+1, c.y()+1, c.cx()-2, c.cy()-2, true );
	//Draw after the cursor
	auto x = m_cursor_x;
	for(auto it = m_value.begin()+m_cursor_pos; it!=m_value.end(); ++it ) {
		if( x + gdi.get_text_width(*it) >= c.x()+c.cx()-text_margin*2 )
			break;
		x = gdi.draw_text( x , ty, *it );
	}
	//Draw before the cursor
	x = m_cursor_x;
	for(auto it = m_value.begin()+m_cursor_pos-1; m_cursor_pos>0&&it>=m_value.begin(); --it ) {
		const auto tw = gdi.get_text_width(*it);
		if( x - tw < c.x() + text_margin )
			break;
		x -= tw;
		gdi.draw_text( x , ty, *it );
	}
	//Draw cursor
	gdi.draw_line( m_cursor_x, c.y()+text_margin, m_cursor_x, c.y()+c.cy()-4 );

	//FRM1
	gdi.bright_fg_color( -luma );
	gdi.draw_line( c.x(), c.y()+c.cy(), c.x()+c.cx(), c.y()+c.cy() );
	gdi.draw_line( c.x()+c.cx(), c.y()+1, c.x()+c.cx(), c.y()+c.cy() );
	//FRM2
	gdi.bright_fg_color( -luma2 );
	gdi.draw_line( c.x()+1, c.y()+c.cy()-1, c.x()+c.cx(), c.y()-1+c.cy() );
	gdi.draw_line( c.x()+c.cx()-1, c.y()+1, c.x()+c.cx()-1, c.y()+c.cy() );
	//FRM3
	gdi.bright_fg_color( luma );
	gdi.draw_line( c.x()+1, c.y(), c.x()+c.cx()-1, c.y() );
	gdi.draw_line( c.x(), c.y(), c.x(), c.y()+c.cy()-2 );
}
 
//* Report input event
void editbox::report_event( const input::event_info& ev )
{
	bool ret {};
	if(ev.type == event::evtype::EV_KEY ) {
		// dbprintf("Keycode %04x", ev.keyb.key );
		if( m_kbdmode == kbd_mode::joy ) {
			ret = handle_joy( ev.keyb );
		} else if( m_kbdmode == kbd_mode::qwerty ) {
			ret = handle_qwerty( ev.keyb );
		} else {
			dbprintf("Unknown kbd mode %i", m_kbdmode );
		}
		//! Limit handler callback
		if( m_limit_hwnd ) {
			m_limit_hwnd( m_value );
		}
	} else {
		//dbprintf("Unhandled event type %i", ev.type );
	}
	if( ret ) {
		modified();
	}
}
  
//! Move cursor forward
void editbox::cursor_forward()
{
	const auto c = get_coord() + get_owner().get_coord();
	if( m_cursor_pos<m_value.size() ) {
		++m_cursor_pos;
		auto gdi = make_wgdi( );
		const auto new_cur_x =  gdi.get_text_width( m_value[m_cursor_pos]) + m_cursor_x;
		if( new_cur_x  < c.x()+c.cx()-text_margin*2)
			m_cursor_x = new_cur_x;
	}
	else {
		m_cursor_pos = 0;
		m_cursor_x = c.x() + text_margin;
	}
}
 
//! Goto cursor end
void editbox::cursor_end() 
{
	m_cursor_pos = m_value.size();
	const auto c = get_coord() + get_owner().get_coord();
	auto gdi = make_wgdi( );
	const auto new_cur_x =  gdi.get_text_width( m_value[m_cursor_pos] ) + m_cursor_x;
	if( new_cur_x  < c.x()+c.cx()-text_margin*2 )
		m_cursor_x = new_cur_x;
}
 
//! Move cursor backward
void editbox::cursor_backward() 
{
	const auto c = get_coord() + get_owner().get_coord();
	if( m_cursor_pos > 0 ) {
		--m_cursor_pos;
	} else {
		m_cursor_pos = m_value.size();	
	}
	auto gdi = make_wgdi( );
	auto new_cur_x = c.x() + text_margin;

	for (unsigned n = 0; n < m_cursor_pos; n++)
		new_cur_x += gdi.get_text_width( m_value[n] );

	const auto max_x = c.x() + c.cx() - text_margin * 2;
	if (new_cur_x > max_x) new_cur_x = max_x;

	m_cursor_x = new_cur_x;
}
 
//Handle joy KBD
bool editbox::handle_joy( const input::detail::keyboard_tag& evk )
{
	using namespace gfx::input;

	if (evk.stat==keystat::UP)
	{
		if (!long_flag)
		{
			if (evk.key == kbdcodes::os_arrow_right) cursor_forward();
			if (evk.key == kbdcodes::os_arrow_left) cursor_backward();
		}
	}

	if (evk.stat==keystat::LNG)
	{
		if (evk.key == kbdcodes::os_arrow_right)
		{
			if ((m_max_len == 0) || (m_value.size() < m_max_len))
			{
				char prev_char;
				if (m_cursor_pos > 0)
					prev_char = m_value[m_cursor_pos - 1];
				else
					prev_char = 0;
				
				m_value.insert(m_cursor_pos, 1, insert_ch(prev_char));
			}
			
			long_flag = true;
		}
		
		if (evk.key == kbdcodes::os_arrow_left)
		{
			if (m_cursor_pos > 0)
			{
				m_value.erase(m_cursor_pos - 1, 1);
				cursor_backward();
				long_flag = true;
			}
		}
	}

	if (evk.stat==keystat::DOWN)
		long_flag = false;

	if (evk.stat==keystat::DOWN || evk.stat==keystat::RPT)
	{
		if( evk.key == kbdcodes::os_arrow_up )
		{
			m_value[m_cursor_pos] = ch_inc( m_value[m_cursor_pos] );
		} else if( evk.key == kbdcodes::os_arrow_down )
		{
			m_value[m_cursor_pos] = ch_dec( m_value[m_cursor_pos] );
		}
	}

	return true;
}
 
//! Handle querty KBD
bool editbox::handle_qwerty( const input::detail::keyboard_tag& evk )
{
	using namespace gfx::input;
	bool ret {};
	if( (evk.stat==keystat::DOWN || evk.stat==keystat::RPT ) && 
		!m_readonly && (!evk.ctrl || evk.ctrlbits.lshift || evk.ctrlbits.rshift )
	) {
		m_raw_key = 0;
		if( evk.key == kbdcodes::os_arrow_right ) {
			cursor_forward();
			ret = true;
		} else if( evk.key == kbdcodes::os_arrow_left ) {
			cursor_backward();
			ret = true;
		} else if( evk.key == kbdcodes::enter ) {
			event btn_event( this, event::evtype::EV_CLICK );
			ret |= emit( btn_event );
			m_raw_key = evk.key;
			ret = true;
		} else if( evk.key == kbdcodes::backspace ) {
			// Backspace handle
			if( m_cursor_pos-1<=m_value.size() && m_value.size()>m_min_len ) {
				m_value.erase( m_cursor_pos-1, 1 );
				cursor_backward();
				m_raw_key = evk.key;
				ret = true; 
			}
		} else if( !evk.key && evk.scan==scancodes::end ) {
			cursor_end();
			ret = true;

		} else { 
			m_raw_key = evk.key;
			if( key_match( evk.key ) ) {
				if( m_value.size() <= m_cursor_pos ) {
					if( !m_max_len || m_cursor_pos < m_max_len ) {
						m_value += evk.key;
					}
				} else {
					m_value[m_cursor_pos] = evk.key;
				}
				cursor_forward();
				ret = true;
			}
		}
	}
	if( ret && m_raw_key ) {
		event btn_event( this, event::evtype::EV_CHANGE );
		ret |= emit( btn_event );
	}
	return ret;
}
 
//! Clear the box
void editbox::to_begin() 
{
	const auto c = get_coord() + get_owner().get_coord();
	m_cursor_x = c.x() + text_margin;
	m_cursor_pos = 0;
}
 
//Get insert char
char editbox::insert_ch(char prev_char)
{
	switch( m_type ) {
		default:
			if ((prev_char >= 'a') && (prev_char <= 'z'))
				return 'a';
			else
				return 'A';
		break;
		case type::float_pos:
		case type::float_neg:
			return '.';
		case type::integer_pos:
		case type::integer_neg:
			return '0';
	}
}
 
//Validate is character contains the criteria
char editbox::ch_inc( char ch ) const
{
	switch( m_type )
	{
	case type::text:
		if (ch < 'z') ch++;
		else ch = ' ';
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
 
//Validate is character contains the criteria
char editbox::ch_dec( char ch ) const
{
	switch( m_type )
	{
	case type::text:
		if (ch > ' ') ch--;
		else ch = 'z';
		break;
	case type::integer_pos:
		if(ch>'0' && ch<='9') ch--;
		break;
	case type::integer_neg:
		if(ch>'0' && ch<='9') ch--;
		else if( ch=='0') ch='-';
		else if( ch=='-') ch='9';
		break;
	case type::float_pos:
		if(ch>'0' && ch<='9') ch--;
		else if( ch=='0') ch = 'E';
		else if( ch=='E') ch = '.';
		else if( ch=='.') ch = '0';
		break;
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
  
//! If key matches the selected format
bool editbox::key_match( int ch ) const
{
	switch( m_type ) {
	case type::text:
		return std::isprint( ch );
	case type::integer_neg:
		return std::isdigit( ch ) || ch == '-';
	case type::integer_pos:
		return std::isdigit( ch );
	case type::float_pos:
		return isdigit( ch ) || ch=='E' || ch=='e' || ch=='.';
	case type::float_neg:
		return isdigit( ch ) || ch=='E' || ch=='e' || ch=='.' || ch=='-';
	}
	return false;
}
 
//! Get int value
int editbox::value_int() const
{
	return std::atoi( m_value.c_str() );
}
  
//! Set int value
void editbox::value( int value ) 
{
	char buf[32];
	fnd::fnd_itoa( buf, value, m_min_len, '0');
	m_value = buf;
	to_begin();
}
 
} /* namespace gui */
} /* namespace gfx */
 
