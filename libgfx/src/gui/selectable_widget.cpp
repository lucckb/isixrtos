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
	}
	if( ret ) {
		modified();
	}
}
 
} /* namespace gui */
} /* namespace gfx */
 
