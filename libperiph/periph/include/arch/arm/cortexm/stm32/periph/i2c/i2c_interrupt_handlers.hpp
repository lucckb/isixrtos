/*
 * =====================================================================================
 *
 *       Filename:  i2c_interrupt_handlers.hpp
 *
 *    Description:  I2c Interrupt handlers drivers
 *
 *        Version:  1.0
 *        Created:  21.06.2019 18:13:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <stm32_ll_i2c.h>
#include <functional>

namespace periph::drivers::i2c::_handlers {

	//! I2C event type event or error
	enum class htype : bool { ev, err };
	int register_handler(const I2C_TypeDef * const i2c, std::function<void(htype)> callback);
}


