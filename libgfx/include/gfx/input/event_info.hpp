/*
 * event_info.hpp
 *
 *  Created on: 29 wrz 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#pragma once 
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
	class window;
	class widget;
}
namespace input {

/* ------------------------------------------------------------------ */
/** Keybd codes */
struct kbdcodes {
	enum keyboard : char {
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
struct mousecodes {
	enum mouse : char {
		any_button, left_button, middle_button, right_button
	};
};
/* ------------------------------------------------------------------ */
namespace detail {

	/** Keyboard tag event */
    struct keyboard_tag {
		typedef unsigned char control_key_type;         //! Control keys
		typedef unsigned char key_type;		        //! Key defs
		enum class status : unsigned char               //! Key status
		{
			DOWN,                                       //! Key is down
			UP                                          //! Key is up
		} stat;         
		key_type key;                                   //! Current key
		key_type scan;									//! Scan code
		union {
			control_key_type ctrl;                      //! Control events
			struct {
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
	//! User message arguments
	union argument {
		unsigned uint;	//! Unsigned type
		int 	 sint;	//! Signed type
		void *   ptr;	//! Pointer type
		const void* cptr;	//! Const pointer to void
	};
	//! Data for hotplug event
	struct hotplug {
		bool connected() const {
			return htype == type::plug;
		}
		bool is_keyboard() const {
			return devtype == device::keyboard; 
		}
		bool is_joystick() const {
			return devtype == device::joystick;
		}
		bool is_mouse() const {
			return devtype == device::mouse;
		}
		enum class device : unsigned char {	//! Hotplug device identifier
			keyboard,
			mouse,
			joystick
		} devtype;
		enum class type : bool {	//! Event type
			unplug,
			plug,
		} htype;
		const void *devid;		//! Device private data
	};
} // ns detail
/* ------------------------------------------------------------------ */
struct event_info {
	/** Event type */
	enum evtype	: unsigned short {
		EV_PAINT,	/** Repaint all windows without propagate as report event */
		EV_WINDOW,	/** User window event */
		EV_KEY,		/** Keyboard event  */
		EV_MOUSE,	/** Relative event  */
		EV_HOTPLUG, /** Hotplug event for ex Insert remove kbd/mouse etc */
		/** Events raised by the component callbacks 
		 * on emit widget level*/
		EV_CLICK,	/** Click event  */
		EV_CHANGE	/** Component changed */
	};
	unsigned time;  //! Timestamp
	evtype type;    //! Event type
	gui::window *window; 		    	//! Optional window address EV_PAINT, EV_WINDOW
	union {
		detail::keyboard_tag keyb;      //! Keyboard tag
		struct {						//! For EV_WINDOW , EV_WIDGET 
			detail::argument param1;			//! User message 1
			detail::argument param2;			//! User message 2
			detail::argument param3;			//! User message 3
		} user;							//! User message part
		struct {						//! For EV_PAINT  message
			bool force;					//! Force component redraw
			bool clrbg;					//! Clear background
		}  paint;
		detail::hotplug hotplug;		//! Hotplug event
	};
};
/* ------------------------------------------------------------------ */
}	//ns gui
}	//ns gfx

