/*
 * =====================================================================================
 *
 *       Filename:  timer.cpp
 *
 *    Description:  Input GUI timer implementation
 *
 *        Version:  1.0
 *        Created:  30.07.2014 19:13:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gfx/gui/timer.hpp>
#include <gfx/gui/frame.hpp>
#include <gfx/input/input.hpp>
#include <isix.h>
#include <foundation/sys/dbglog.h>
 
namespace gfx {
namespace gui {
 
/** Constructor for timer
	*/
timer::timer( gfx::gui::frame& frm, unsigned elapse, window* win, 
		bool cyclic, int id ) 
	:  m_sys_timer( isix_vtimer_create() ), 
	   m_frm( frm ), m_win( win ), m_id( id ), 
	   m_cyclic( cyclic ), m_elapse( elapse )
{
}
 
//! Destructor
timer::~timer() {
	if( m_sys_timer ) {
		isix_vtimer_destroy( m_sys_timer );
	}
}
 
//! Raw timer callback
void timer::raw_timer_callback( void* data ) {
	using evinfo = gfx::input::event_info;
	if( data == nullptr ) {
		return;
	}
	auto& tp = *static_cast<timer*>( data );
	//! Prepare post FFT ready event
	evinfo ev {
		isix_get_jiffies(),	//! Current jiffies
		evinfo::EV_TIMER,			//! Sent event user
		tp.m_win,					//! Target is my window
		{}
	};
	ev.timer.id = tp.m_id;
	ev.timer.owner = &tp;
	tp.m_frm.report_event( ev );
}
  
int timer::start()
{
	int ret { einval };
	if( m_cyclic ) {
		if( !m_started ) {
			ret =  isix_vtimer_start( m_sys_timer, raw_timer_callback, this, 
					isix_ms2tick(m_elapse), true );
			m_started = !ret;
		} 
	} else {
		ret = isix_vtimer_start( m_sys_timer, 
			raw_timer_callback, this, isix_ms2tick(m_elapse), false );
	}
	return ret;
}
 
//! Stop timer
int timer::stop()
{
	int ret { einval };
	if( m_started || !m_cyclic ) {
		ret =  isix_vtimer_cancel( m_sys_timer );
		m_started = false;
	}
	return ret;
}
  
}}

