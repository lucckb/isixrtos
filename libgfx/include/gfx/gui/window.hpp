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
#include <foundation/noncopyable.hpp>
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
class window : public fnd::noncopyable
{
public:
	//Get window
	window( const rectangle &coord, frame &frm )
		: m_coord( coord ), m_frm ( frm )
	{
		m_frm.add_window( this );
	}
	// On repaint the widget return true when changed
	virtual bool repaint();
	//* Report input event
	virtual bool report_event( const input::event_info& /*ev*/ )
	{
		//TODO: FIXME THIS
		return false;
	}
	void add_widget( widget * const w )
	{
		m_widgets.push_front( w );
	}
	void delete_widget( widget * const w )
	{
		m_widgets.remove( w );
	}
	const rectangle& get_coord() const { return m_coord; }
	frame& get_owner() { return m_frm; }
private:
	detail::windows_container<widget*> m_widgets;
	rectangle m_coord;
	frame &m_frm;
};
/* ------------------------------------------------------------------ */

}}

/* ------------------------------------------------------------------ */

#endif /* GFX_GUI_WINDOW_HPP_ */
/* ------------------------------------------------------------------ */
