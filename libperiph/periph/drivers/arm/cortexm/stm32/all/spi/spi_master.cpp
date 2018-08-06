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
#include <stm32f3xx_ll_spi.h>

namespace periph::drivers {


//Constructor
spi_master::spi_master(const char name[])
	: block_device(block_device::type::spi, dt::get_periph_base_address(name))
{
	error::expose<error::bus_exception>(error::init);
}
spi_master::~spi_master() {
	close();
}

//Open device
int spi_master::open(unsigned , int )
{
	int ret;
	do {
		dt::clk_periph pclk;
		auto ret = dt::get_periph_clock(io<void>(), pclk);
		error::expose<error::bus_exception>(ret); if(ret) break;
		ret = setup(true);
	} while(0);
	return ret;
}

//Close device
int spi_master::close()
{
	return -1;
}

//Make transaction
int spi_master::transaction(int addr, const blk::transfer& data)
{
	(void)addr;
	(void)data;
	return -1;
}

//Set device option
int spi_master::do_set_option(device_option& opt)
{
	(void)opt;
	return -1;
}

//Setup gpio and clocks
int spi_master::setup(bool en)
{
	int ret {};
	do {
		//MISO
		auto pin = dt::get_periph_pin(io<void>(),dt::pinfunc::miso);
		auto mux = dt::get_periph_pin_mux(io<void>());
		if((ret=pin)||(ret=mux)) break;
		gpio::setup(pin, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::high});
		//MOSI
		pin = dt::get_periph_pin(io<void>(),dt::pinfunc::mosi);
		if((ret=pin)) break;
		gpio::setup(pin, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::high});
		//SCK
		pin = dt::get_periph_pin(io<void>(),dt::pinfunc::sck);
		if((ret=pin)) break;
		gpio::setup(pin, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::high});

	} while(0);
	return ret;
}

}
