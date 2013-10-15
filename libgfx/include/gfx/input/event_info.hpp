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
/** Keybd codes */
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
/* Mouse codes */
struct mousecodes
{
	enum mouse : char
	{
		any_button, left_button, middle_button, right_button
	};
};
/* ------------------------------------------------------------------ */
namespace detail {

	/** Keyboard tag event */
    struct keyboard_tag
	{
		typedef unsigned char control_key_type;         //! Control keys
		typedef unsigned char key_type;                 //! Key defs
		enum class status : unsigned char               //! Key status
		{
			DOWN,                                       //! Key is down
			UP                                          //! Key is up
		} stat;         
		key_type key;                                   //! Current key
		union
		{
			control_key_type ctrl;                      //! Control events
			struct
			{
				unsigned char lctrl:1;                  //! Left control press
				unsigned char lshift:1;                 //! Left shift press
				unsigned char lalt : 1;                 //! Left alt
				unsigned char lgui : 1;                 //! Left gui
				unsigned char rctrl: 1;                 //! Right control
				unsigned char rshift:1;                 //! Right shift
				unsigned char ralt:1;                   //! Right alt
				unsigned char rgui: 1;                  //! Right gui

			} ctrlbits;
		};

	};
} // ns detail
/* ------------------------------------------------------------------ */
struct event_info
{
	/** Event type */
	enum evtype	: unsigned short
	{
		EV_SW,		/** Plug unplug event */
		EV_KEY,		/** Keyboard event  */
		EV_MOUSE,	/** Relative event  */
		EV_CLICK,	/** Click event  (Inherited) */
		EV_CHANGE	/* Component changed */
	};
	unsigned time;  //! Timestamp
	evtype type;    //! Event type
	union
	{
		detail::keyboard_tag keyb;      //! Keyboard tag
	};
};
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx

#endif /* EVENT_INFO_HPP_ */
