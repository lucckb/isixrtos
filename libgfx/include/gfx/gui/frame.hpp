/*
 * frame.hpp
 *
 *  Created on: 30 wrz 2013
 *      Author: lucck
 */

#ifndef GFX_GUI_FRAME_HPP_
#define GFX_GUI_FRAME_HPP_
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
	//Standard constructor
	frame( drv::disp_base &display ):
		m_events_queue( system_events_qsize ), m_disp( display )
	{

	}
	/** Execute gui main loop */
	void execute();
	/** Send gui event handler */
	int report_event( const input::event_info &event );
	//Get display
	drv::disp_base& get_display() const { return m_disp; }
	//Add widget to frame
	void add_window( window* window );
	//Delete the widget
	void delete_window( window* window );
	//Get default layout
	layout const& get_def_layout() const { return m_default_layout; }
	layout const& get_def_win_layout() const { return m_default_win_layout; }
	//Focus on the window
	void set_focus( window* window );
protected:
	//Repaint visible
	bool repaint();
private:
	//Private events queue
	isix::fifo<input::event_info> m_events_queue;
	//Windows container
	detail::windows_container<window*> m_windows;
	//Display
	drv::disp_base& m_disp;
	//Default layout
	layout m_default_win_layout { color::Black, color::White, color::Red };					/* Window layout */
	layout m_default_layout { color::Black, color::LightGray, color::BlueViolet  } ;		/* Component layout */
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* GUI_MANAGER_HPP_ */
/* ------------------------------------------------------------------ */
