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
#include <isix/arch/irq_platform.h>
#include <isix/arch/irq.h>
#include <isix/arch/cache.h>
#include <foundation/sys/dbglog.h>


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
		dt::device_conf cnf;
		if((ret=dt::get_periph_devconf(io<void>(),cnf))<0) break;
		isix::set_irq_priority(cnf.irqnum, {uint8_t(cnf.irqfh), uint8_t(cnf.irqfl)});
		isix::request_irq(cnf.irqnum); error::expose<error::bus_exception>(ret);
		//+1 ERR IRQN
		isix::set_irq_priority(cnf.irqnum+1, {uint8_t(cnf.irqfh), uint8_t(cnf.irqfl)});
		isix::request_irq(cnf.irqnum+1); error::expose<error::bus_exception>(ret);
		LL_I2C_Disable(io<I2C_TypeDef>());
		if((ret=dt::get_periph_clock(io<void>()))<0) break;
		//Default speed configuration
		LL_I2C_Disable(io<I2C_TypeDef>());
		LL_I2C_ConfigSpeed(io<I2C_TypeDef>(),ret,100'000,LL_I2C_DUTYCYCLE_2);
		LL_I2C_Enable(io<I2C_TypeDef>());
		LL_I2C_EnableIT_EVT(io<I2C_TypeDef>());
		LL_I2C_EnableIT_ERR(io<I2C_TypeDef>());
		//! Bug!
		if(LL_I2C_IsActiveFlag_STOP(io<I2C_TypeDef>())) {
			LL_I2C_EnableReset(io<I2C_TypeDef>());
			for(int i=0;i<8;++i) asm volatile("nop\t\n");
			LL_I2C_DisableReset(io<I2C_TypeDef>());
		}
	} while(0);
	return ret;
}

//! Make transaction
/** Należy wykorzystać 3 rodzaje transferu jako tylko RX tylko TX albo TRX
 * i w tej zależności RX TX lub TRX
 */
int i2c_master::transaction(int addr, const blk::transfer& /*data*/)
{
	//1 Means RO 0xff MAX I2C addr
	if(addr<0 || addr>=0xff || (addr&1)) {
		dbg_err("Invalid address");
		return error::invaddr;
	}
	isix::mutex_locker _lock(m_mtx);

	return error::inval;
}


//! Do close
int i2c_master::do_close()
{
	int ret {};
	LL_I2C_DisableIT_EVT(io<I2C_TypeDef>());
	LL_I2C_DisableIT_ERR(io<I2C_TypeDef>());
	LL_I2C_Disable(io<I2C_TypeDef>());
	do {
		dt::device_conf cnf;
		if((ret=dt::get_periph_devconf(io<void>(),cnf))<0) break;
		isix::free_irq(cnf.irqnum+1); error::expose<error::bus_exception>(ret);
	} while(0);
	return ret;
}

// Set option
int i2c_master::do_set_option(const option::device_option& opt)
{
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
		// Some F1 devices IP core have bugs so reset controller
		// After GPIO config
		LL_I2C_EnableReset(io<I2C_TypeDef>());
		for(int i=0;i<8;++i) asm volatile("nop\t\n");
		LL_I2C_DisableReset(io<I2C_TypeDef>());
	} while(0);
	return ret;
}

}



