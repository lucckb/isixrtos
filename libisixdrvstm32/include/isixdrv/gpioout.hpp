/*
 * gpio.hpp
 *
 *  Created on: 23 gru 2017
 *      Author: lucck
 */


#pragma once

#include <foundation/drv/bus/gpioout.hpp>

#include <stm32gpio.h>

namespace stm32 {
namespace drv {

class gpio_out final : public fnd::drv::bus::gpio_out {
public:
	//! GPIO out constructor
	gpio_out( GPIO_TypeDef* port, unsigned pin,
	stm32::e_abstract_gpio_speed speed=stm32::AGPIO_SPEED_LOW)
		: m_port(port, pin)
	{
		stm32::gpio_abstract_config(m_port.port(),m_port.ord(),
			stm32::AGPIO_MODE_OUTPUT_PP, speed );
	}
	//! Output data
	void operator()(bool en) noexcept override
	{
		if(en)
			stm32::gpio_set(m_port.port(), m_port.ord());
		else
			stm32::gpio_clr(m_port.port(), m_port.ord());
	}
private:
	const gpio::pin_desc m_port;
};

} /* namespace drv */
} /* namespace stm32 */

