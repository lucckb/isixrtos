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
	drv::disp_base& get_display() const
	{
		return m_disp;
	}
	//Add window to frame
	void add_window( window* window );
	//Delete the window
	void delete_window( window* window );
protected:
	/* Repaint the all windows */
	int repaint_all();
	//Repaint visible
	int repaint();
private:
	//Private events queue
	isix::fifo<input::event_info> m_events_queue;
	//Windows container
	detail::windows_container<window*> m_windows;
	//Display
	drv::disp_base& m_disp;
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* GUI_MANAGER_HPP_ */
/* ------------------------------------------------------------------ */
