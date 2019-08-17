/*
 * =====================================================================================
 *
 *       Filename:  timer.hpp
 *
 *    Description:  Input timer implementation
 *
 *        Version:  1.0
 *        Created:  30.07.2014 18:58:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
 
#include <foundation/algo/noncopyable.hpp>
#include <isix.h>
 
namespace isix {
	struct vtimer_struct;
}
namespace gfx {
namespace gui {
class window;
class frame;
 
//! Timer class API for gui based on isix events
class timer : private fnd::noncopyable {
public:
	static constexpr auto id_any = -1;
	static constexpr auto einval = -512;
	/** Constructor for timer
	 */
	timer( gfx::gui::frame& frm, unsigned elapse, window* win=nullptr,
		   bool cyclic = true, int id = id_any );
	//! Destructor
	~timer();
	//! Start timer Periodic
	int start();
	//! Stop timer
	int stop();
	//! Restart timer
	int restart() {
		auto err = stop();
		if( !err ) {
			err = start();
		}
		return err;
	}
	//! Change value time
	void operator()( unsigned elapse ) {
		m_elapse = elapse;
	}
private:
	//! Raw timer callback
	static void raw_timer_callback( void* data );
private:
	osvtimer_t m_sys_timer;	 		//! Isix systimer ID
	gfx::gui::frame& m_frm;			//! Frame owner
	window* const m_win;			//! Target window
	const int m_id;						//! Identifier
	const bool m_cyclic;					//! Is cyclic timer
	bool m_started {};				//! Started
	unsigned m_elapse;				//! Elapse time
};
  
}}
 
