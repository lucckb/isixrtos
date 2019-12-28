/*
 * selectable_widget.cpp
 *
 *  Created on: 10 paź 2013
 *      Author: lucck
 */
 
#include <gfx/gui/selectable_widget.hpp>
#include <foundation/sys/dbglog.h>
 
namespace gfx {
namespace gui {

 
//* Report input event
void selectable_widget::report_event( const input::event_info& ev )
{
	using evinfo = gfx::input::event_info;
	using tevent = gfx::input::touchevents;
	bool ret {};
	if( ev.type == evinfo::EV_KEY ) {
		if( m_push_key > 0 && m_push_key == ev.keyb.key ) {
			const auto cpush = ( ev.keyb.stat == input::detail::keyboard_tag::status::DOWN );
			if( cpush ) {
				event btn_event( this, event::evtype::EV_CLICK );
				ret |= emit( btn_event );
			}
			ret |= cpush != m_pushed;
			m_pushed = cpush;
		}
	} else if( ev.type == evinfo::EV_TOUCH ) {
		if( ev.touch.eventid == tevent::press_down ) {
			const auto c = get_coord() + get_owner().get_coord();
			if(c.inside({ev.touch.x, ev.touch.y})) {
				event btn_event( this, event::evtype::EV_CLICK );
				emit( btn_event );
				m_pushed = true;
				ret = true;
			}
		} else if( ev.touch.eventid == tevent::press_up ) {
			ret = true;
			m_pushed = false;
		}
	}
	if( ret ) {
		modified();
	}
}
 
} /* namespace gui */
} /* namespace gfx */
 
