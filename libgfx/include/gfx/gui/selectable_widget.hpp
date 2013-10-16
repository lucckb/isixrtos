/* ------------------------------------------------------------------ */
/*
 * selectable_widget.hpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef GFX_GUI_SELECTABLE_WIDGET_HPP_
#define GFX_GUI_SELECTABLE_WIDGET_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/gui/widget.hpp>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
class selectable_widget: public widget
{
public:
	explicit selectable_widget( rectangle const& rect,layout const& layout ,window &win )
		: widget( rect, layout, win )
	{}
	virtual ~selectable_widget() {}
	void pushed( bool pushed ) { m_pushed = pushed; }
	bool pushed() const { return m_pushed; }
	void pushkey( short key ) {m_push_key = key; }
public:
	//* Report input event
	virtual bool report_event( const input::event_info& ev );
private:
	bool m_pushed {};
	short m_push_key { -1 };
};
/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */

/* ------------------------------------------------------------------ */
#endif /* SELECTABLE_WIDGET_HPP_ */
