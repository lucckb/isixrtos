/* ------------------------------------------------------------------ */
/*
 * editbox.hpp
 *
 *  Created on: 14 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#pragma once
/* ------------------------------------------------------------------ */
#include <gfx/gui/widget.hpp>
#include <gfx/gui/detail/defines.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
/* Edit box widget for editing values */
class editbox: public widget
{
	using keystat = gfx::input::detail::keyboard_tag::status;
public:
	enum class type	 : char {
		text,				//Text edit
		float_pos,		//Floating point type
		float_neg,		//Negative floating point
		integer_pos,		//Integer positive
		integer_neg,		//Integer negative
	};
	enum class kbd_mode : char {
		joy,		//Joystick mode
		qwerty		//Full kbd mode
	};
	//Constructor
	editbox( rectangle const& rect,layout const& layout ,window &win )
	 : widget( rect, layout, win ),
	   m_cursor_x((get_coord() + get_owner().get_coord()).x()+text_margin)
	{}
	//Destructor
	virtual ~editbox()
	{}
	//Get value
	template <typename T> T get() const;
	//Set mask character
	void mask( char mask_ch = '*') { m_mask = mask_ch; }
	//Set minimum length
	void min_len( short min_len_ ) { m_min_len = min_len_; }
	//Set maximum len
	void max_len( short max_len_ ) { m_max_len = max_len_; }
	//Read only
	bool readonly() const { return m_readonly; }
	void readonly( bool readonly ) { m_readonly  = readonly; }
	//Set full kbdmode
	void kbdmode( kbd_mode kbdmode ) { m_kbdmode = kbdmode; }
	//Get field value
	template< typename T>
	void value( const T value ) {
		m_value = value;
	}
	//Set text value
	const detail::string& value() const {
		return m_value;
	}
	//* Report input event
	virtual void report_event( const input::event_info& ev );
protected:
	// On repaint the widget return true when changed
	virtual void repaint();
private:
	//Handle joy KBD
	bool handle_joy( const input::detail::keyboard_tag &key_tag );
	//Handle querty KBD
	bool handle_qwerty( const input::detail::keyboard_tag &key_tag );
	//Increment decrement character according criteria
	char ch_inc( char ch ) const;
	char ch_dec( char ch ) const;
	//Insert character get
	char insert_ch();
	//! Move cursor forward
	void cursor_forward();
private:
	short m_min_len { 0 };					//Minimum len
	short m_max_len { 0 };					//Maximum len
	type m_type {type::text};				//Widget type
	detail::string m_value;					//Default value
	bool m_readonly {};						//Is readonly
	kbd_mode m_kbdmode {kbd_mode::joy};		//Full kbd fully supported
	size_t m_cursor_pos {};					//Current cursor position
	int m_cursor_x {};						//Cursor position on screen
	char m_mask {};							//Mask character enabled
private:	//Private constants
	static constexpr coord_t text_margin = 2;
};
/* ------------------------------------------------------------------ */

} /* namespace gui */
} /* namespace gfx */

/* ------------------------------------------------------------------ */
