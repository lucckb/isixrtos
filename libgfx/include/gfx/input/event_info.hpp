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
