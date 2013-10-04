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
	explicit button( rectangle const& rect,layout const& layout ,frame &mngr )
		: window( rect, layout, mngr )
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
	void pushed( bool pushed )
	{
		m_pushed = pushed;
	}
	void set_pushkey( short key )
	{
		m_push_key = key;
	}
protected:
	// On repaint the window return true when changed
	virtual bool repaint();
	//* Report input event
	virtual bool report_event( const input::event_info& ev );
private:
	detail::string m_caption;
	bool m_pushed {};
	short m_push_key { -1 };
};
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */

#endif
/* ------------------------------------------------------------------ */
