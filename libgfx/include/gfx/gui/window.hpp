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
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
class window  : private fnd::noncopyable
{
public:
	 //Create window constructor
	 explicit window( coord_t new_x, coord_t new_y, coord_t new_cx, coord_t new_cy, frame &mngr)
	 	 : m_x(new_x), m_y(new_y), m_cx(new_cx), m_cy(new_cy),
	 	   m_mngr( mngr ), m_gdi( mngr.get_display() )
	 {
		 m_mngr.add_window( this );
	 }
	 //Resize the window
	 void resize(coord_t new_width, coord_t new_height);
	 //Move the window
	 void move( coord_t new_x, coord_t new_y );
	 //Repaint the window
	// On repaint the window return true when changed
	virtual bool repaint();
protected:
	//Get GDI
	disp::gdi& gdi() { return m_gdi; };
private:
	coord_t m_x {}, m_y {};							/* Position X */
	coord_t m_cx {}, m_cy {};				/* Position Y */
	frame &m_mngr;							/* GUI manager */
	disp::gdi m_gdi;								/* Graphics content for window */
	bool m_visible {};								/* The window is visible */
	bool m_changed {};								/* The window is changed */
};
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* WINDOW_HPP_ */
