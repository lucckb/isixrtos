/*
 * =====================================================================================
 *
 *       Filename:  stm32_i2c_v1.cpp
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  21.06.2019 17:41:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <periph/drivers/i2c/i2c_master.hpp>
#include <periph/clock/clocks.hpp>
#include <periph/i2c/i2c_interrupt_handlers.hpp>
#include <periph/gpio/gpio.hpp>


namespace periph::drivers {

//! Constructor
i2c_master::i2c_master(const char name[])
	: block_device(block_device::type::i2c, dt::get_periph_base_address(name))
{
	if(io<void>()) {
		const auto ret = i2c::_handlers::register_handler(io<I2C_TypeDef>(),
		std::bind(&i2c_master::interrupt_handler,std::ref(*this), std::placeholders::_1) );
		error::expose<error::bus_exception>(ret);
	}
}

//! Destructor
i2c_master::~i2c_master()
{
	close();
}

//! DO open
int i2c_master::do_open(int /*timeout*/)
{
	int ret {};
	do {
		ret = periph_conf(true); if(ret) break;
	} while(0);
	return ret;
}

//! Make transaction
int i2c_master::transaction(int /*addr*/, const blk::transfer& /*data*/)
{
	return error::inval;
}


//! Do close
int i2c_master::do_close()
{
	return error::inval;
}

// Set option
int i2c_master::do_set_option(const option::device_option& /*opt*/)
{
	return error::inval;
}

//! Common interrupt handler for i2c device
void i2c_master::interrupt_handler(i2c::_handlers::htype /*type*/) noexcept
{
}


//! Peripheral configuration
int i2c_master::periph_conf(bool en) noexcept
{
	int ret {};
	do {
		ret = clock::device_enable(io<void>(),en);
		if(ret) break;
		const auto mux = dt::get_periph_pin_mux(io<void>());
		if(mux<0) { ret=mux; break; }
		const int scl = dt::get_periph_pin(io<void>(),dt::pinfunc::scl);
		if(scl<0) { ret=scl; break; }
		const int sda = dt::get_periph_pin(io<void>(),dt::pinfunc::sda);
		if(sda<0) { ret=sda; break; }
		if(en) {
			gpio::setup(scl, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::medium});
			gpio::setup(sda, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::medium});
		} else {
			gpio::setup(scl, gpio::mode::in{gpio::pulltype::floating});
			gpio::setup(sda, gpio::mode::in{gpio::pulltype::floating});
		}
	} while(0);
	return ret;
}

}



