/*
 * dts_internal.hpp
 *
 *  Created on: 28 lut 2018
 *      Author: lucck
 */

#pragma once

#include <cstdint>

#include "dts.hpp"


namespace periph {
namespace dt {

namespace _dts_config {

	//! Clock and assigned speed
	struct clock {
		bus busid;
		unsigned (*speedfn)();
	};

	//! GPIO function assignment to number
	struct pin {
		pinfunc gpiofun;
		unsigned gpio;
	};

	//! Device tree mapper
	struct device {
		const char* name;	//! Device name
		uintptr_t addr;		//! Device  mapped address
		bus devbus;			//! Pin used map
		unsigned mux;		//! Which mux
		unsigned clken;		//! Clock enable pin mark number
		const pin* pins;	//! Pin configuration
	};

	//! Global device configuration structure
	struct configuration {
		const clock*  const clk;		//! Clocks array
		const device* const devs;		//! Device array
	};
	//! Machine configuration global config
	extern const configuration the_machine_config;

}


}}
