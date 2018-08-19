/*
 * display_config.hpp
 *
 *  Created on: 19 sie 2018
 *      Author: lucck
 */

#pragma once

#include <periph/dt/types.hpp>


namespace periph::display {

	//! Devops configuration
	struct config : public periph::dt::device_conf_base {
		uint8_t max_x;
		uint8_t max_y;
	};

}
