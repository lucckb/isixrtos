/*
 * widget.hpp
 *
 *  Created on: 30 wrz 2013
 *      Author: lucck
 */

#pragma once
 
#include <gfx/types.hpp>
#include <memory>
#include <gfx/gui/object.hpp>
#include <gfx/gui/detail/defines.hpp>
#include <gfx/disp/gdi.hpp>
#include <gfx/gui/window.hpp>
#include <gfx/gui/primitives.hpp>
 
namespace gfx {
namespace gui {
 
class widget  : public object
{
public:
	//Create widget constructor
	widget( rectangle const& rect,layout const& layout,
			window &win, bool selectable = true )
		: m_coord(rect), m_layout(layout), 
		m_win(win), m_selectable(selectable)
	{
		m_win.add_widget( this );
	}

	//Remove widget
	virtual ~widget() {
		m_win.delete_widget( this );
	}

	//Set widget color
	void set_layout( layout const& lay ) {
		m_layout = lay;
	}

	//* Report input event
	virtual void report_event( const input::event_info& /*ev*/ ) {
	}

	// Get client coordinate
	const rectangle& get_coord() const { 
		return m_coord; 
	}

	//Get selectable flag
	bool selectable() const { 
		return m_selectable; 
	}

	// selectable flag
	void selectable(bool select_mode) {
		m_selectable = select_mode;
	}

	//! Widget is changed
	bool is_modified() const {
		return m_modified;
	}

	/** Redraw the window only if windows state is changed */
	void redraw( bool force, bool win_changed ) {
		if( force || win_changed || is_modified() ) {
			repaint( force && m_win.has_focus() );
			m_modified = false;
		}
	}

protected:
	//! Set modified flag
	void modified() {
		m_modified = true;
	}

	// On repaint the widget return true when changed
	virtual void repaint( bool focus ) = 0;

	//Get base layout
	const layout& get_layout() const { 
		return m_layout.inherit()?
			m_win.get_owner().get_def_layout():
			m_layout; 
	}

	//! Get parent object
	const window& get_owner() const { 
		return m_win; 
	 }

	//Make gdi
	disp::gdi make_gdi( ) {
		const auto l = m_layout.inherit()?m_win.get_owner().get_def_layout():m_layout;
		return std::move(
			disp::gdi( m_win.get_owner().get_display(), l.fg(), l.bg(), l.font() )
		);
	}

	//Make win gdi
	disp::gdi make_wgdi() {
		const auto l = m_win.get_layout();
		return std::move(
			disp::gdi(m_win.get_owner().get_display(), l.fg(),l.bg(),l.font() )
		);
	}
private:
	rectangle m_coord;								/* Current coordinate */
	layout m_layout;								/* Component layout */
	window &m_win;									/* GUI manager */
	bool m_selectable;  							/* Widget is selectable */
	bool m_modified {};								/* Widget is changed  */
};
 
}}
 
