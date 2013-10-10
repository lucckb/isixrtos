/*
 * window.hpp
 *
 *  Created on: 7 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GUI_WINDOW_HPP_
#define GFX_GUI_WINDOW_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/gui/object.hpp>
#include <gfx/types.hpp>
#include <gfx/gui/primitives.hpp>
#include <gfx/gui/detail/defines.hpp>
#include <gfx/input/event_info.hpp>
#include <gfx/gui/frame.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
class widget;

/* ------------------------------------------------------------------ */
//Window class
class window : public object
{
public:
	struct flags
	{
		enum  : unsigned
		{
			fill = 			0x01,			//Fill background
			border = 		0x02,			//Draw border
			selectborder =  0x04			//Select border
		};
	};
	//Get window
	window( const rectangle &coord, frame &frm, unsigned flags = 0 )
		: m_coord( coord ),m_frm ( frm ), m_flags(flags)
	{
		m_frm.add_window( this );
	}
	// On repaint the widget return true when changed
	virtual void repaint();
	//* Report input event
	virtual bool report_event( const input::event_info& ev );
	void add_widget( widget * const w );
	void delete_widget( widget * const w );
	const rectangle& get_coord() const { return m_coord; }
	frame& get_owner() { return m_frm; }
	void set_layout( const layout &lay ) { m_layout = lay; }
	//Select next item
	void select_next();
	//Select prev item
	void select_prev();
	//Get base layout
	const layout& get_layout() const { return m_layout.inherit()?m_frm.get_def_win_layout():m_layout; }
private:
	detail::container<widget*> m_widgets;
	detail::container<widget*>::iterator m_current_widget { m_widgets.end() };
	detail::container<widget*>::iterator m_redraw_widget { m_widgets.end() };
	layout m_layout;
	rectangle m_coord;
	frame &m_frm;
	unsigned m_flags;
};
/* ------------------------------------------------------------------ */

}}

/* ------------------------------------------------------------------ */

#endif /* GFX_GUI_WINDOW_HPP_ */
/* ------------------------------------------------------------------ */
