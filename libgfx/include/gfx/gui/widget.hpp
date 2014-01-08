/*
 * widget.hpp
 *
 *  Created on: 30 wrz 2013
 *      Author: lucck
 */

#ifndef GFX_GUI_WIDGET_HPP_
#define GFX_GUI_WIDGET_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/types.hpp>
#include <memory>
#include <gfx/gui/object.hpp>
#include <gfx/gui/detail/defines.hpp>
#include <gfx/disp/gdi.hpp>
#include <gfx/gui/window.hpp>
#include <gfx/gui/primitives.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
class widget  : public object
{
public:
	//Create widget constructor
	widget( rectangle const& rect,layout const& layout,
			window &win, bool selectable = true )
		: m_coord(rect), m_layout(layout), 
		m_win(win), m_selectable(selectable)
	{
		//TODO: FIXME THIS
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
	virtual bool report_event( const input::event_info& /*ev*/ ) {
		return false;
	}

	// Get client coordinate
	const rectangle& get_coord() const { 
		return m_coord; 
	}

	//Get selectable flag
	bool selectable() const { 
		return m_selectable; 
	}


	// On repaint the widget return true when changed
	virtual void repaint() = 0;

protected:

	//Get base layout
	const layout& get_layout() const { 
		return m_layout.inherit()?
			m_win.get_owner().get_def_layout():
			m_layout; 
	}

//	window& get_owner() { return m_win; }
	const window& get_owner() const { 
		return m_win; 
	 }

	//Make gdi
	disp::gdi make_gdi( ) {
		const auto l = m_layout.inherit()?m_win.get_owner().get_def_layout():m_layout;
		return std::move(disp::gdi( m_win.get_owner().get_display(), l.fg(), l.bg(), l.font() ));
	}

	//Make win gdi
	disp::gdi make_wgdi() {
		const auto l = m_win.get_layout();
		return std::move(
			disp::gdi(m_win.get_owner().get_display(),
				l.fg(),l.bg(),l.font() )
		);
	}
private:
	rectangle m_coord;								/* Current coordinate */
	layout m_layout;								/* Component layout */
	window &m_win;									/* GUI manager */
	const bool m_selectable;  						/* The widget is changed */
};
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* GFX_GUI_WIDGET_HPP_ */
