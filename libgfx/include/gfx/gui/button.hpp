/*
 * button.hpp
 *
 *  Created on: 2 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GUI_BUTTON_HPP_
#define GFX_GUI_BUTTON_HPP_
/* ------------------------------------------------------------------ */
#include "window.hpp"

/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {

class button: public window
{
public:
	explicit button( coord_t new_x, coord_t new_y, coord_t new_cx, coord_t new_cy, frame &mngr)
		: window( new_x, new_y, new_cx, new_cy, mngr )
	{}
	//Destructor
	virtual ~button()
	{}
	//Set caption
	template< typename T>
	void caption( const T caption )
	{
		m_caption = caption;
	}
protected:
	// On repaint the window return true when changed
	virtual bool repaint();
	//* Report input event
	virtual bool report_event( const input::event_info& ev );
private:
	detail::string m_caption;
};
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */

#endif
/* ------------------------------------------------------------------ */
