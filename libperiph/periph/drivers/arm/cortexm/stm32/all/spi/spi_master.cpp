/*
 * =====================================================================================
 *
 *       Filename:  spi_master.cpp
 *
 *    Description:  SPI master for stm32 implementation
 *
 *        Version:  1.0
 *        Created:  06.08.2018 20:01:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <periph/drivers/spi/spi_master.hpp>
#include <periph/dt/dts.hpp>
#include <periph/clock/clocks.hpp>
#include <periph/gpio/gpio.hpp>
#include <periph/core/device_option.hpp>
#include <stm32f3xx_ll_spi.h>

namespace periph::drivers {


//Constructor
spi_master::spi_master(const char name[])
	: block_device(block_device::type::spi, dt::get_periph_base_address(name))
{
}

spi_master::~spi_master() {
	close();
}

//Open device
int spi_master::do_open(int)
{
	int ret {};
	do {
		if(!io<void>()) {
			ret = error::init; break;
		}
		dt::clk_periph pclk;
		ret = dt::get_periph_clock(io<void>(), pclk); if(ret) break;
		ret = clk_conf(true); if(ret) break;
		ret = gpio_conf(true); if(ret) break;
	} while(0);
	error::expose<error::bus_exception>(ret);
	return ret;
}

//Close device
int spi_master::do_close()
{
	int ret {};
	do {
		if(!io<void>()) {
			ret = error::init; break;
		}
		dt::clk_periph pclk;
		ret = dt::get_periph_clock(io<void>(), pclk); if(ret) break;
		ret = clk_conf(false); if(ret) break;
		ret = gpio_conf(false); if(ret) break;
	} while(0);
	return ret;
}

//Make transaction
int spi_master::transaction(int addr, const blk::transfer& data)
{
	(void)addr;
	(void)data;
	return -1;
}

//Set device option
int spi_master::do_set_option(option::device_option& opt)
{
	switch(opt.ord) {
		case option::ord::speed: {
			auto hz = static_cast<option::speed&>(opt).hz;
			break;
		}
		case option::ord::phase: {
			auto ph = static_cast<option::phase&>(opt).ph;
			break;
		}
		case option::ord::polarity: {
			auto pol = static_cast<option::polarity&>(opt).pol;
			break;
		}
		case option::ord::dwidth: {
			auto dw = static_cast<option::dwidth&>(opt).dw;
			break;
		}
	}
}

// Clocks configuration
int spi_master::clk_conf(bool en)
{
	int ret {};
	dt::clk_periph pclk;
	if((ret=dt::get_periph_clock(io<void>(),pclk))<0) return ret;
	if(en) {
		if((ret=clock::device_enable(pclk))<0) return ret;
	} else {
		if((ret=clock::device_disable(pclk))<0) return ret;
	}
	return ret;
}

//Gpio configuration
int spi_master::gpio_conf(bool en)
{
	auto mux = dt::get_periph_pin_mux(io<void>());
	if(mux<0) return mux;
	for(auto it=dt::pinfunc::sck;it<=dt::pinfunc::mosi;++it) {
		int pin = dt::get_periph_pin(io<void>(),it);
		if(pin<0) return pin;
		if(en)
			gpio::setup(pin, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::high});
		else
			gpio::setup(pin, gpio::mode::in{gpio::pulltype::floating});
	}
	int csi {};
	for(auto it=dt::pinfunc::cs0;it<=dt::pinfunc::cs3;++it,++csi) {
		int pin=dt::get_periph_pin(io<void>(),it);
		if(pin<0 && pin!=error::nopin) return pin;
		if(pin) {
			if(en) {
				gpio::setup(pin, gpio::mode::out{gpio::outtype::pushpull,gpio::speed::medium} );
				m_cs[csi] = pin;
			} else {
				gpio::setup(pin, gpio::mode::in{gpio::pulltype::floating});
				m_cs[csi] = invcs;
			}
		}
	}
	return error::success;
}

}
