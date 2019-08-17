 
/*
 * editbox.hpp
 *
 *  Created on: 14 paz 2013
 *      Author: lucck
 */
 
#pragma once
 
#include <gfx/gui/widget.hpp>
#include <gfx/gui/detail/defines.hpp>
 
namespace gfx {
namespace gui {
 
/* Edit box widget for editing values */
class editbox: public widget
{
	using keystat = gfx::input::detail::keyboard_tag::status;
public:
	//! Limit handler for 
	using limit_handler = std::function<void(detail::string&)>;
	//! Edit box type
	enum class type	: char {
		text,				//! Text edit
		float_pos,			//! Floating point type
		float_neg,			//! Negative floating point
		integer_pos,		//! Integer positive
		integer_neg,		//! Integer negative
	};
	//! Keyboard mode
	enum class kbd_mode : char {
		joy,		//! Joystick mode
		qwerty		//! Full kbd mode
	};
	//! Constructor
	editbox( rectangle const& rect, layout const& layout, window &win )
	 : widget( rect, layout, win ),
	   m_cursor_x( (get_coord() + get_owner().get_coord()).x()+text_margin )
	{}
	//! Destructor
	virtual ~editbox()
	{}
	//! Get raw key
	char raw_key() const {
		return m_raw_key;
	}
	//! Set mask character
	void mask( char mask_ch = '*') { 
		m_mask = mask_ch; 
	}
	//! Set edit mode
	void set_mode( type etype, short min_len = 0, short max_len = 0 )  {
		m_type =  etype;
		m_min_len = min_len;
		m_max_len = max_len;
	}
	//! Set max length
	void set_max_length( short max_len ) {
		m_max_len = max_len;
	}

	//! Set limit handler
	void set_limit_handler( limit_handler hwnd ) {
		m_limit_hwnd = hwnd;
	}
	//! Read only get
	bool readonly() const { 
		return m_readonly; 
	}
	//! Read only set
	void readonly( bool readonly ) { 
		m_readonly = readonly; 
	}
	//Set full kbdmode
	void kbdmode( kbd_mode kbdmode ) { 
		m_kbdmode = kbdmode; 
	}
	//! Get field value
	template< typename T>
	void value( const T value ) {
		m_value = value;
		if( m_value.size() < m_min_len ) {
			m_value.resize( m_min_len, ' ' );
		}
		to_begin();
	}
	//! Set text value
	const detail::string& value() const {
		return m_value;
	}
	//! Get int value
	int value_int() const;
	//! Set int value
	void value( int value );
	//! Clear the entry box
	void clear() {
		to_begin();
		m_value.clear();
	}
	//* Report input event
	virtual void report_event( const input::event_info& ev );
protected:
	//! On repaint the widget return true when changed
	virtual void repaint( bool focus );
private:
	//! Handle joy KBD
	bool handle_joy( const input::detail::keyboard_tag &key_tag );
	//! Handle querty KBD
	bool handle_qwerty( const input::detail::keyboard_tag &key_tag );
	//! Increment decrement character according criteria
	char ch_inc( char ch ) const;
	char ch_dec( char ch ) const;
	//! Insert character get
	char insert_ch(char prev_char);
	//! Move cursor forward
	void cursor_forward();
	//! Cursor move backward
	void cursor_backward();
	//! Cursor to end 
	void cursor_end();
	//! Move cursor to begin
	void to_begin();
	//! If key matches the selected format
	bool key_match( int ch ) const;
private:
	unsigned short m_min_len { 0 };			//Minimum len
	unsigned short m_max_len { 0 };			//Maximum len
	type m_type {type::text};				//Widget type
	detail::string m_value;					//Default value
	bool m_readonly {};						//Is readonly
	kbd_mode m_kbdmode {kbd_mode::joy};		//Full kbd fully supported
	size_t m_cursor_pos {};					//Current cursor position
	int m_cursor_x {};						//Cursor position on screen
	char m_mask {};							//Mask character enabled
	char m_raw_key {};						//!Raw key
	limit_handler m_limit_hwnd;				//! Limit handler
private:	//Private constants
	static constexpr coord_t text_margin = 2;
	bool long_flag { false };
};
 

} /* namespace gui */
} /* namespace gfx */

 
