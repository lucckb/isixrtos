/*
 * input.cpp
 *
 *  Created on: 28-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/input/input.hpp>
#include <list>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace inp {

/* ------------------------------------------------------------------ */
input_queue_t input_class::m_queue( config::QUEUE_SIZE );
/* ------------------------------------------------------------------ */
int input_class::input_report_key( input::key_code code, unsigned char key, unsigned char skeys  )
{
	//TODO: FIXME Pointer to sender
	const input::event ev
	{
		isix::isix_get_jiffies(),
		//shared_from_this(),
		this,
		static_cast<unsigned short>(code),
		input::EV_KEY,
		{{key,skeys}}
	};
	return m_queue.push_isr( ev );
}
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
