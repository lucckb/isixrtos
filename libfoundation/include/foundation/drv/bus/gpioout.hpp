/*
 * gpio.hpp
 *
 *  Created on: 23 gru 2017
 *      Author: lucck
 */

#pragma once

namespace fnd {
namespace drv {
namespace bus {

//! Virtual gpio abstract class for set and get
class gpio_out {
public:
	virtual ~gpio_out() {}
	gpio_out(gpio_out&) = delete;
	gpio_out& operator=(gpio_out&) = delete;
	gpio_out() {}
	virtual void operator()(bool en) noexcept = 0;
};

} /* namespace bus */
} /* namespace drv */
} /* namespace fnd */

