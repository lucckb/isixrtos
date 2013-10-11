/*
 * icon.hpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GUI_ICON_HPP_
#define GFX_GUI_ICON_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/gui/widget.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */
class icon: public widget
{
public:
	//Constructor
	icon( rectangle const& rect,layout const& layout ,window &win );
	//Destructor
	virtual ~icon() {}
	//Set image
	void image( const bitmap_t& bitmap )
	{
		m_bitmap = &bitmap;
	}
	void image()
	{
		m_bitmap = nullptr;
	}
	//Repaint the label
	virtual void repaint();
private:
	const bitmap_t *m_bitmap {};
};
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
#endif /* GFX_GUI_ICON_HPP_ */
/* ------------------------------------------------------------------ */