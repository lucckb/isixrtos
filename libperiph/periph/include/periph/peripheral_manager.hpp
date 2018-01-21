/*
 * peripheral_manager.hpp
 *
 *  Created on: 19 sty 2018
 *      Author: lucck
 */

#pragma once

namespace periph {

	//! Peripheral manager class
	class peripheral_manager {
	public:
		//! Peripheral manager instance
		static peripheral_manager& instance();
		//! Non copy able
		peripheral_manager(peripheral_manager&) = delete;
		peripheral_manager& operator=(peripheral_manager&) = delete;
		//! Default constructor
		peripheral_manager() {
		}
		//! Create device tree by config
		void create_devices_tree();
	};

}
