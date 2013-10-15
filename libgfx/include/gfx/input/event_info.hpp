/*
 * event_info.hpp
 *
 *  Created on: 29 wrz 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_INPUT_EVENT_INFO_HPP_
#define GFX_INPUT_EVENT_INFO_HPP_

/* ------------------------------------------------------------------ */
namespace gfx {
namespace input {

/* ------------------------------------------------------------------ */
struct kbdcodes
{
	enum keyboard : char
	{
		//Control Code for ASCII
		select_all	= 0x1,
		copy		= 0x3,		//Ctrl+C
		backspace	= 0x8,	tab		= 0x9,
		enter_n		= 0xA,	enter	= 0xD,	enter_r = 0xD,
		alt			= 0x12,
		paste		= 0x16,		//Ctrl+V
		cut			= 0x18,		//Ctrl+X
		escape		= 0x1B,
		//System Code for OS
		os_pageup		= 0x21,	os_pagedown,
		os_arrow_left	= 0x25, os_arrow_up, os_arrow_right, os_arrow_down,
		os_insert		= 0x2D, os_del
	};
};

/* ------------------------------------------------------------------ */
struct mousecodes
{
	enum mouse : char
	{
		any_button, left_button, middle_button, right_button
	};
};
/* ------------------------------------------------------------------ */
namespace detail {

	struct keyboard_tag
	{
		typedef unsigned char control_key_type;
		typedef unsigned char key_type;
		enum class status : unsigned char
		{
			DOWN,
			UP
		} stat;
		key_type key;
		union
		{
			control_key_type ctrl;
			struct
			{
				unsigned char lctrl:1;
				unsigned char lshift:1;
				unsigned char lalt : 1;
				unsigned char lgui : 1;
				unsigned char rctrl: 1;
				unsigned char rshift:1;
				unsigned char ralt:1;
				unsigned char rgui: 1;

			} ctrlbits;
		};

	};
} // ns detail
/* ------------------------------------------------------------------ */
struct event_info
{
	/* Event type */
	enum evtype	: unsigned short
	{
		EV_SW,		/* Plug unplug event */
		EV_KEY,		/* Keyboard event  */
		EV_MOUSE,	/* Relative event  */
		EV_CLICK,	/* Click event  (Inherited) */
		EV_CHANGE	/* Component changed */
	};
	unsigned time;
	evtype type;
	union
	{
		detail::keyboard_tag keyb;
	};
};
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx

#endif /* EVENT_INFO_HPP_ */
