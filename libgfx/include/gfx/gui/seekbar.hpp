/*
 * seekbar.hpp
 *
 *  Created on: 15 paź 2013
 *      Author: lucck
 */

#pragma once
/* ------------------------------------------------------------------ */ 
#include <gfx/gui/widget.hpp>

/* ------------------------------------------------------------------ */
namespace gfx {
namespace gui {

/* ------------------------------------------------------------------ */
class seekbar: public widget
{
public:
	//Constructor
	explicit seekbar( rectangle const& rect,layout const& layout, window &win, bool selectable = true )
		: widget( rect, layout, win, selectable )
	{}
	//Destructor
	virtual ~seekbar() {};
	//* Report input event
	virtual void report_event( const input::event_info& ev );
	//Set/get min value
	void value( short val ) { m_value = val; }
	short value() const { return m_value; }
	//Set step
	void step(short step) { m_step = step; }
protected:
	//Repaint virtual function
	virtual void repaint();
private:
	short m_min { 0 };			//Minimum value
	short m_max { 100 };		//Maximum value
	short m_value { };			//Current value
	short m_step { 1 };			//Step value
	short m_psex { 0 };			//Previous X value
};

/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */

/* ------------------------------------------------------------------ */
