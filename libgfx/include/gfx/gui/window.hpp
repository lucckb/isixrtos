/*
 * window.hpp
 *
 *  Created on: 30 wrz 2013
 *      Author: lucck
 */

#ifndef GFX_GUI_WINDOW_HPP_
#define GFX_GUI_WINDOW_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/types.hpp>
#include <memory>
#include <foundation/noncopyable.hpp>
#include <gfx/gui/detail/defines.hpp>
#include <gfx/disp/gdi.hpp>
#include <gfx/gui/frame.hpp>
#include <gfx/gui/primitives.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
class window  : private fnd::noncopyable
{
public:
	 //Create window constructor
	 explicit window( rectangle const& rect,layout const& layout ,frame &mngr)
	 	 : m_coord(rect), m_layout(layout), m_mngr(mngr)
	 {
		 m_mngr.add_window( this );
	 }
	 //Remove window
	 virtual ~window()
	 {
		 m_mngr.delete_window( this );
	 }
	 //Resize the window
	 void resize(coord_t new_width, coord_t new_height);
	 //Move the window
	 void move( coord_t new_x, coord_t new_y );
	 //Set window color
	 void set_layout( layout const& lay )
	 {
		m_layout = lay;
	 }
	// On repaint the window return true when changed
	virtual bool repaint();
	//* Report input event
	virtual bool report_event( const input::event_info& /*ev*/ )
	{
		return false;
	}
	//On event
protected:
	//Get
	const rectangle& get_coord() const { return m_coord; }
	const layout& get_layout() const { return m_layout; }
	frame& get_frame() { return m_mngr; }
	//Make gdi
	disp::gdi make_gdi( layout const& lay )
	{
		const auto l = lay.inherit()?m_mngr.get_def_layout():lay;
		return std::move(disp::gdi( m_mngr.get_display(), l.fg(), l.bg(), l.font() ));
	}
private:
	rectangle m_coord;
	layout m_layout;
	frame &m_mngr;									/* GUI manager */
	bool m_changed {};								/* The window is changed */
};
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* WINDOW_HPP_ */
