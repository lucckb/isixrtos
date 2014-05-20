/*
 * =====================================================================================
 *
 *       Filename:  waterfall.hpp
 *
 *    Description:  Waterfall implementation for example fft watterfall
 *
 *        Version:  1.0
 *        Created:  19.05.2014 20:56:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

/* ------------------------------------------------------------------ */
#include <gfx/gui/widget.hpp>
/* ------------------------------------------------------------------ */ 
namespace gfx {
namespace gui {
/* ------------------------------------------------------------------ */ 
/** Watterfall class widget implementation. 
 *  Watterfall is an waterfall update widget for 
 *  example for FFT watterfal function for detect frequencies*/
class waterfall : public widget {
	static constexpr auto c_margin = 2;
public:
	/** Waterfall constructor 
	 * @param[in] rect Widget size
	 * @param[in] layout Widget layout colors and fonts
	 * @param[in] win Parent window owner
	 * @param[in] in_len Message input size 
	 */
	waterfall( rectangle const& rect, layout const& layout, 
			   window &win, size_t in_len )
		: widget( rect, layout, win ), m_length( in_len )
	{
	}
	//! Virtual destructor
	virtual ~waterfall() {
	}
	 //! Report an event
	virtual void report_event( const input::event_info& ev );
	//! New fft data
	void new_data( const unsigned short* ptr ) {
		m_data_ptr =  ptr;
	}
protected:
	 //! On repaint the widget return true when changed
	 virtual void repaint();
private:
	 //! Draw GUI frame
	 void draw_frame();
private:
	const size_t m_length;			//! Handle length of input waterfal msg
	const unsigned short *m_data_ptr {};	//! Data pointer
};
/* ------------------------------------------------------------------ */ 
}	//gui
}	//gfx
