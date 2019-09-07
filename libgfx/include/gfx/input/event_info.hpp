/*
 * event_info.hpp
 *
 *  Created on: 29 wrz 2013
 *      Author: lucck
 */
 
#pragma once 
 
namespace gfx {
namespace gui {
	class window;
	class widget;
	class timer;
}
namespace input {

 
/** Keybd codes */
struct kbdcodes {
	enum keyboard : unsigned char {
		//Control Code for ASCII
		extend_code = 0x0,
		select_all	= 0x1,
		copy		= 0x3,		//Ctrl+C
		backspace	= 0x8,	tab		= 0x9,
		enter_n		= 0xA,	enter	= 0xA,	enter_r = 0xA,
		escape		= 0x10,
		//System Code for OS
		os_pageup, os_pagedown,
		os_arrow_left, os_arrow_up, os_arrow_right, os_arrow_down,
		os_insert, os_del
	};
};
struct scancodes {
	//! Scan codes
	enum kbdscan : unsigned char {
		arrow_up = 0x52,
		arrow_down = 0x51,
		arrow_right = 0x4f,
		arrow_left = 0x50,
		escape = 0x29,
		end = 0x4d,
		del = 0x4c,
		F1 = 0x3a,
		F2 = 0x3b,
		F3 = 0x3c,
		F4 = 0x3d,
		F5 = 0x3e,
		F6 = 0x3f,
		F7 = 0x40,
		F8 = 0x41,
		F9 = 0x42,
		F10 = 0x43,
		F11 = 0x44,
		F12 = 0x45,
		T = 0x17,
		R = 0x15,
		A = 0x04,
		L = 0x0f,
		C = 0x06,
		D = 0x07,
		S = 0x16,
	};
};

 
/* Mouse codes */
struct mousecodes {
	enum mouse : char {
		any_button, left_button, middle_button, right_button
	};
};

/* Touch screen gestures */
struct touchgestures {
	enum touchgests : char {
		undefined,		//! Unknown
		move_up,		//! Gesture move up
		move_right,		//! Gesture move right
		move_down,		//! Gesture Move down
		move_left,		//! Gesture Move left
		zoom_in,		//! Gesture zoom in
		zoom_out,		//! Gesture zom out
		_end
	};
}; 

/* Touch screen events */
struct touchevents {
	enum touchevt : char {
		undefined,	//! Event unknown
		press_down,	//! Touch event press down
		lift_up,	//! Touch event lift up
		contact,	//! Touch event contact
		_end
	};
};
 
namespace detail {

	/** Keyboard tag event */
    struct keyboard_tag {
		typedef unsigned char control_key_type;         //! Control keys
		typedef unsigned char key_type;					//! Key defs
		enum class status : unsigned char               //! Key status
		{
			DOWN,                                       //! Key is down
			UP,                                         //! Key is up
			RPT,										//! Rpt key event
			LNG,										//! Long press event
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

	/** Knob tag event */
	struct knob_tag {
		int value;
		int diff;
		int maxval;
	};
	/** Touch screen tag  */
	struct touch_tag {
		static constexpr auto max_touch = 2;
		unsigned char num_touches;				//! Number of touches
		unsigned short x[max_touch];			//! Position x
		unsigned short y[max_touch];			//! Position y
		unsigned char weight[max_touch];		//! Touch weight
		touchevents::touchevt eventid[max_touch];			//! Touch event id
		unsigned char area[max_touch];						//! Touch area
		touchgestures::touchgests gestureid;				//! Gesture id
	};
	//! User message arguments
	union argument {
		unsigned uint;	//! Unsigned type
		int		 sint;	//! Signed type
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
	//! Structure for the GUI timer
	struct timer {
		int id;
		gfx::gui::timer* owner;
	};
} // ns detail
 
struct event_info {
	/** Event type */
	enum evtype	: unsigned short {
		EV_PAINT,	/** Repaint all windows without propagate as report event */
		EV_WINDOW,	/** User window event */
		EV_KEY,		/** Keyboard event  */
		EV_MOUSE,	/** Relative event  */
		EV_KNOB,	/** Knob UI event */
		EV_TOUCH,	/** Touch screen callback */
		EV_HOTPLUG, /** Hotplug event for ex Insert remove kbd/mouse etc */
		/** Events raised by the component callbacks
		 * on emit widget level*/
		EV_CLICK,	/** Click event  */
		EV_CHANGE,	/** Component changed */
		EV_FOCUS,	/** Focus get or focus lost */
		EV_TIMER,	/** EV timer event */
		EV_JOY_REPORT,	/** Joystick report */
	};
	unsigned time;  //! Timestamp
	evtype type;    //! Event type
	gui::window *window;				//! Optional window address EV_PAINT, EV_WINDOW, EV_TIMER
	union {
		detail::keyboard_tag keyb;      //! Keyboard tag
		detail::knob_tag knob;			//! Knob change value tag
		detail::touch_tag touch;		//! Touch screen event
		struct {						//! For EV_WINDOW , EV_WIDGET
			detail::argument param1;			//! User message 1
			detail::argument param2;			//! User message 2
			detail::argument param3;			//! User message 3
			detail::argument param4;			//! User message 3
		} user;							//! User message part
		struct {						//! For EV_PAINT  message
			bool force;					//! Force component redraw
			bool clrbg;					//! Clear background
		}  paint;
		detail::hotplug hotplug;		//! Hotplug event
		detail::timer timer;			//! Timer event
	};
};
 
}	//ns gui
}	//ns gfx

