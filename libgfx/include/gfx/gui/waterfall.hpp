/*
 * =====================================================================================
 *
 *       Filename:  waterfall.hpp
 *
 *    Description:  I Waterfall implementation for example fft watterfall
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
class waterfall :  public widget {
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
private:
	size_t m_length;
};
/* ------------------------------------------------------------------ */ 
}	//gui
}	//gfx
