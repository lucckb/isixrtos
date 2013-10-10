/*
 * label.hpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GUI_LABEL_HPP_
#define GFX_GUI_LABEL_HPP_
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
	virtual bool report_event( const input::event_info& /*ev*/ )
	{
		return false;
	}
	//Repaint the label
	virtual void repaint();
};
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */
/* ------------------------------------------------------------------ */
#endif /* LABEL_HPP_ */
/* ------------------------------------------------------------------ */
