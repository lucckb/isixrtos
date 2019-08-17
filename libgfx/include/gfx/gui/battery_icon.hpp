/*
 * =====================================================================================
 *
 *       Filename:  battery_icon.hpp
 *
 *    Description:  Vector graphics battery icon implementation
 *
 *        Version:  1.0
 *        Created:  14.05.2014 19:49:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
  
#include <gfx/gui/widget.hpp>
  
namespace gfx {
namespace gui {
 
//Raster battery icon widget implementation
class battery_icon : public widget {
public:
	/** Battery icon widget implementatation 
	 * @param[in] rect Icon size
	 * @param[in] layout Default layout
	 * @param[in] win Parent window
	 */
	battery_icon( rectangle const& rect, layout const& layout, window& win );
	//! Destructor
	virtual ~battery_icon() {
	}
	//! Set percent value
	void percent( unsigned char value ) {
		if( m_percent > 100 ) {
			m_percent = 100;
		}
		m_percent = value;
	}
	void status(bool charging, bool full) {
		m_charging = charging;
		m_full = full;
	}
	
protected:
	//! Repaint the window
	virtual void repaint( bool focus );
private:
	unsigned char m_percent {};			//! Private percent value 
	bool m_charging { false };
	bool m_full { false };
};
 
} }
  
