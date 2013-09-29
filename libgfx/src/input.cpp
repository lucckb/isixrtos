/*
 * input.cpp
 *
 *  Created on: 28-06-2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include <gfx/input/input.hpp>

/* ------------------------------------------------------------------ */
namespace gfx {
namespace input {

/* ------------------------------------------------------------------ */
int input_class::input_report_key(  gui::detail::keyboard_tag::status status ,
				gui::detail::keyboard_tag::key_type key,
				gui::detail::keyboard_tag::control_key_type ctl  )
{
	//TODO: FIXME Pointer to sender
	const gui::event_info ev
	{
		isix::isix_get_jiffies(),
		gui::event_info::evtype::EV_KEY,
		{
			status,
			key,
			ctl
		}
	};
	return m_queue?(m_queue->push_isr(ev)):(isix::ISIX_ENOTSUP);
}
/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
