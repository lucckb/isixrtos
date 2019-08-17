/*
 * frame.hpp
 *
 *  Created on: 30 wrz 2013
 *      Author: lucck
 */
 
#pragma once
 
#include <gfx/gui/object.hpp>
#include <gfx/input/event_info.hpp>
#include <gfx/gui/detail/defines.hpp>
#include <gfx/gui/primitives.hpp>
#include <gfx/drivers/disp/disp_base.hpp>
#include <stack>
#include <isix.h>
 
namespace gfx {
namespace gui {

 
class window;
 
/* Gui Manager master class */
class frame : public object {
	static constexpr auto system_events_qsize = 64;
public:
	struct error { enum {
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
	int update( window* target_win = nullptr ) {
		return queue_repaint( true, target_win, false );
	}
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
	//! Focus on the window
	int set_focus( window* win, window* back_win = nullptr );

	//!  Pop focus on prev window
	int pop_focus() {
		if( !m_win_stack.empty() ) {
			auto cf = m_win_stack.top();
			m_win_stack.pop();
			return set_focus( cf );
		} else {
			return error::wnd_not_found;
		}
	}
	//! If previous focus is empty
	bool stack_empty() const {
		return m_win_stack.empty();
	}
	//! Get active window
	window* get_active_window() const {
		return m_windows.back();
	}
	//! Get width
	coord_t width() const {
		return m_disp.get_width();
	}
	//! Get height
	coord_t height() const {
		return m_disp.get_height();
	}
private:
	/** Repaint visible area 
	 * @param[in] force Force repaint independent of component refresh
	 * @param[in] wnd Window to repaint if empty 
	 * @param[in] force_clr Force clear background
	 */
	void repaint( bool force, window* wnd, bool force_clr );
	/** Private update function safe from other threads */
	int queue_repaint( bool force, window* wnd, bool force_clr );
private:
	//Private events queue
	isix::fifo<input::event_info> m_events_queue;
	//! Isix lock container data
	isix::semaphore m_lock { 1, 1 };
	//Windows container
	detail::container<window*> m_windows;
	//Display
	drv::disp_base& m_disp;
	//Main frame color
	color_t m_color;
	//Default layout
	layout m_default_win_layout { color_t(~m_color), m_color, color::Red };					/* Window layout */
	layout m_default_layout { color::Black, color::LightGray, color::BlueViolet  } ;		/* Component layout */
	std::stack<window*> m_win_stack;
};

 
}}
 
