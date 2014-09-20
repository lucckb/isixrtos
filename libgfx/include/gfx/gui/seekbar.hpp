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
	//! Min value
	void min_value( short _min ) {
		m_min = _min;
	}
	//! Max value
	void max_value( short _max ) {
		m_max = _max;
	}
	//! Reporting mode setup
	void report_mode( bool mode) {
		m_report = mode;
	}
protected:
	//Repaint virtual function
	virtual void repaint( bool focus );
private:
	short m_min { 0 };			//Minimum value
	short m_max { 100 };		//Maximum value
	short m_value { };			//Current value
	short m_step { 1 };			//Step value
	bool m_report { true };		//On key report flag
};

/* ------------------------------------------------------------------ */
} /* namespace gui */
} /* namespace gfx */

/* ------------------------------------------------------------------ */
