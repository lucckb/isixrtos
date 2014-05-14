/*
 * frame.hpp
 *
 *  Created on: 30 wrz 2013
 *      Author: lucck
 */

#pragma once
/* ------------------------------------------------------------------ */
#include <foundation/noncopyable.hpp>
#include <gfx/input/event_info.hpp>
#include <gfx/gui/detail/defines.hpp>
#include <gfx/gui/primitives.hpp>
#include <isix.h>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace drv {
	class disp_base;
}}
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
class window;
/* ------------------------------------------------------------------ */
/* Gui Manager master class */
class frame : private fnd::noncopyable
{
	static constexpr auto system_events_qsize = 64;
public:
	struct errno { enum {
		success = 0,
		wnd_not_found = -4096
	};};
	//Standard constructor
	frame( drv::disp_base &display, color_t color = color::Black )
		: m_events_queue( system_events_qsize ),
		  m_disp( display ), m_color( color )
	{
	}
	/** Refresh frame manual requirement */
	int update();
	/** Execute gui main loop */
	void execute();
	/** Send gui event handler */
	int report_event( const input::event_info &event );
	//Get display
	drv::disp_base& get_display() const { 
		return m_disp; 
	}
	//Add widget to frame
	void add_window( window* window );
	//Delete the widget
	void delete_window( window* window );
	//Get default layout
	layout const& get_def_layout() const { 
		return m_default_layout; 
	}
	//Get default window layout
	layout const& get_def_win_layout() const { 
		return m_default_win_layout; 
	}
	//Set layout
	void set_def_layout( const layout& lay ) { 
		m_default_layout = lay; 
	}
	//Set default window layout
	void set_def_win_layout( const layout& lay ) { 
		m_default_win_layout = lay; 
	}
	//Set basic text color
	void set_text_color( color_t color ) {
		m_default_win_layout.fg( color );
	}
	//Focus on the window
	int set_focus( window* win );
protected:
	//Repaint visible
	void repaint( bool force );
private:
	//Private events queue
	isix::fifo<input::event_info> m_events_queue;
	//Windows container
	detail::container<window*> m_windows;
	//Display
	drv::disp_base& m_disp;
	//Main frame color
	color_t m_color;
	//Default layout
	layout m_default_win_layout { color_t(~m_color), m_color, color::Red };					/* Window layout */
	layout m_default_layout { color::Black, color::LightGray, color::BlueViolet  } ;		/* Component layout */
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
