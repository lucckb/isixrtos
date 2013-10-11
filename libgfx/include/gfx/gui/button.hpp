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
#include "selectable_widget.hpp"

/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
class button: public selectable_widget
{
public:
	explicit button( rectangle const& rect,layout const& layout ,window &win )
		: selectable_widget( rect, layout, win )
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
	const std::string& caption() const
	{
		return m_caption;
	}
protected:
	// On repaint the widget return true when changed
	virtual void repaint();
private:
	detail::string m_caption;
};
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */

#endif
/* ------------------------------------------------------------------ */