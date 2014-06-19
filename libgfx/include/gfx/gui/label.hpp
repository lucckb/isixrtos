/*
 * label.hpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#pragma once
/* ------------------------------------------------------------------ */
#include <gfx/gui/widget.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
class label: public widget
{
public:
	//Destructor
	virtual ~label(){ }
	//Constructor
	label(rectangle const& rect,layout const& layout ,window &win);
	//Label no reports any events
	template< typename T>
	void caption( const T caption ) {
		m_caption = caption;
	}
	const detail::string& caption() const {
		return m_caption;
	}
protected:
	//Repaint the label
	virtual void repaint();
private:
	detail::string m_caption;
	coord_t draw_text_wdt;
};
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
