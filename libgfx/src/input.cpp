/*
 * input.cpp
 *
 *  Created on: 28-06-2013
 *      Author: lucck
 */

#include <gfx/input/input.hpp>
#include <isix.h>

namespace gfx {
namespace input {

int input_class::input_report_key(  detail::keyboard_tag::status status ,
				detail::keyboard_tag::key_type key,
				detail::keyboard_tag::control_key_type ctl  )
{
	//TODO: FIXME Pointer to sender
	const event_info ev
	{
		isix_get_jiffies(),
		event_info::evtype::EV_KEY,
		nullptr,
		{
			status,
			key,
			0,
			ctl
		}
	};
	m_evt_report( ev );
	return 0;
}

}}

