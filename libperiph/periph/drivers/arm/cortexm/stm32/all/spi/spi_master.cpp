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
#include <periph/blk/transfer.hpp>
#include <periph/gpio/gpio.hpp>
#include <isix/arch/irq_platform.h>
#include <isix/arch/irq.h>
#include <isix/arch/cache.h>
#include <stm32f3xx_ll_spi.h>
#include <foundation/sys/dbglog.h>
#include "spi_interrupt_handlers.hpp"

namespace periph::drivers {


//Constructor
spi_master::spi_master(const char name[])
	: block_device(block_device::type::spi, dt::get_periph_base_address(name))
{
	if(io<void>()) {
		const auto ret =_handlers::register_handler(io<SPI_TypeDef>(),
		std::bind(&spi_master::interrupt_handler,std::ref(*this)) );
		error::expose<error::bus_exception>(ret);
	}
}

spi_master::~spi_master() {
	close();
}

//Open device
int spi_master::do_open(int timeout)
{
	int ret {};
	do {
		if(!io<void>()) {
			ret = error::init; break;
		}
		{
			//Clock config
			dt::clk_periph pclk;
			ret = dt::get_periph_clock(io<void>(), pclk); if(ret) break;
			ret = clk_conf(true); if(ret) break;
			ret = clk_conf(true); if(ret) break;
			ret = gpio_conf(true); if(ret) break;
		}
		{
			//Configure interrupt
			dt::device_conf cnf;
			if((ret=dt::get_periph_devconf(io<void>(),cnf))<0) break;
			dbg_info("Set irq: %i prio: %i:%i", cnf.irqnum, cnf.irqfh, cnf.irqfl);
			isix::set_irq_priority(cnf.irqnum, {uint8_t(cnf.irqfh), uint8_t(cnf.irqfl)});
			isix::request_irq(cnf.irqnum);
			error::expose<error::bus_exception>(ret);
			//Rest of the config
			LL_SPI_SetTransferDirection(io<SPI_TypeDef>(),LL_SPI_FULL_DUPLEX);
			LL_SPI_SetNSSMode(io<SPI_TypeDef>(), LL_SPI_NSS_SOFT);
			LL_SPI_SetRxFIFOThreshold(io<SPI_TypeDef>(), LL_SPI_RX_FIFO_TH_QUARTER);
			LL_SPI_SetMode(io<SPI_TypeDef>(), LL_SPI_MODE_MASTER);
			LL_SPI_EnableIT_ERR(io<SPI_TypeDef>());
			m_timeout = timeout;
		}
	} while(0);
	return ret;
}

//Close device
int spi_master::do_close()
{
	int ret {};
	LL_SPI_DisableIT_RXNE(io<SPI_TypeDef>());
	LL_SPI_DisableIT_TXE(io<SPI_TypeDef>());
	LL_SPI_DisableIT_ERR(io<SPI_TypeDef>());
	LL_SPI_Disable(io<SPI_TypeDef>());
	do {
		if(!io<void>()) {
			ret = error::init; break;
		}
		dt::clk_periph pclk;
		ret = dt::get_periph_clock(io<void>(), pclk); if(ret) break;
		ret = clk_conf(false); if(ret) break;
		ret = gpio_conf(false); if(ret) break;
		{
			//Configure interrupt
			dt::device_conf cnf;
			if((ret=dt::get_periph_devconf(io<void>(),cnf))<0) break;
			isix::free_irq(cnf.irqnum);
		}
	} while(0);
	return ret;
}

//Make transaction
int spi_master::transaction(int addr, const blk::transfer& data)
{
	int ret {};
	if(addr<0 || addr>=int(sizeof(m_cs)/sizeof(m_cs[0]))) {
		dbg_err("Invalid address");
		return error::invaddr;
	}
	dbg_info("Start transfer addr %i", addr);
	if(!busy()) {
		isix::mutex_locker _lock(m_mtx);
		start_transfer(std::make_tuple(addr,data,std::ref(ret)));
		periphint_config();
		LL_SPI_Enable(io<SPI_TypeDef>());
		cs(false,addr);
		dbg_info("Not busy");
	} else {
		if(!m_transq.try_push(std::make_tuple(addr,data,std::ref(ret)))) {
			ret = error::again;
			return ret;
		}
		dbg_info("Busy push");
	}
	ret = m_wait.wait(m_timeout);
	dbg_info("Fin transfer with code %i", ret);
	return ret;
}

//Set device option
int spi_master::do_set_option(const option::device_option& opt)
{
	int ret {};
	static constexpr unsigned d2w[] = {
		LL_SPI_DATAWIDTH_4BIT, LL_SPI_DATAWIDTH_5BIT,LL_SPI_DATAWIDTH_6BIT,LL_SPI_DATAWIDTH_7BIT,
		LL_SPI_DATAWIDTH_8BIT, LL_SPI_DATAWIDTH_9BIT,LL_SPI_DATAWIDTH_10BIT,LL_SPI_DATAWIDTH_11BIT,
		LL_SPI_DATAWIDTH_12BIT, LL_SPI_DATAWIDTH_13BIT,LL_SPI_DATAWIDTH_14BIT, LL_SPI_DATAWIDTH_15BIT,
		LL_SPI_DATAWIDTH_16BIT,
	};
	switch(opt.ord()) {
		case option::ord::speed: {
			if((ret=clk_to_presc(static_cast<const option::speed&>(opt).hz()))<0) break;
			LL_SPI_SetBaudRatePrescaler(io<SPI_TypeDef>(),ret); ret = 0;
			break;
		}
		case option::ord::phase: {
			const auto ph = static_cast<const option::phase&>(opt).ph();
			LL_SPI_SetClockPhase(io<SPI_TypeDef>(),
				ph==option::phase::_1_edge?LL_SPI_PHASE_1EDGE:LL_SPI_PHASE_2EDGE
			);
			break;
		}
		case option::ord::polarity: {
			const auto pol = static_cast<const option::polarity&>(opt).pol();
			LL_SPI_SetClockPolarity(io<SPI_TypeDef>(),
				pol==option::polarity::high?LL_SPI_POLARITY_HIGH:LL_SPI_POLARITY_LOW
			);
			break;
		}
		case option::ord::dwidth: {
			auto dw = static_cast<const option::dwidth&>(opt).dw();
			if(dw<4||dw>16) {
				ret = error::inval;
				break;
			}
			LL_SPI_SetDataWidth(io<SPI_TypeDef>(),d2w[dw-4]);
			break;
		}
	}
	return ret;
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
		if(pin>0) {
			if(en) {
				gpio::setup(pin, gpio::mode::out{gpio::outtype::pushpull,gpio::speed::medium} );
				m_cs[csi] = pin;
			} else {
				gpio::setup(pin, gpio::mode::in{gpio::pulltype::floating});
				m_cs[csi] = invcs;
			}
		}
	}
	dbg_info("Pin exit success");
	return error::success;
}


// Convert to prescaller speed
int spi_master::clk_to_presc(unsigned hz)
{
	int ret {};
	dt::clk_periph pclk;
	do {
		if((ret=dt::get_periph_clock(io<void>(),pclk))<0) break;
		if((ret=dt::get_bus_clock(pclk.xbus))<0) break;
		dbg_info("PCLK.XBUS=%i HZ=%i",ret,hz);
		ret = (ret/1000)/(hz/1000U);
	} while(0);
	if( ret <= 2 )
	{
		ret = LL_SPI_BAUDRATEPRESCALER_DIV2;
	}
	else if( ret > 2 && ret <= 4 )
	{
		ret = LL_SPI_BAUDRATEPRESCALER_DIV4;
	}
	else if( ret > 4 && ret <= 8 )
	{
		ret = LL_SPI_BAUDRATEPRESCALER_DIV8;
	}
	else if( ret > 8 && ret <= 16 )
	{
		ret = LL_SPI_BAUDRATEPRESCALER_DIV16;
	}
	else if( ret > 16 && ret <= 32 )
	{
		ret = LL_SPI_BAUDRATEPRESCALER_DIV32;
	}
	else if( ret > 32 && ret <= 64 )
	{
		ret = LL_SPI_BAUDRATEPRESCALER_DIV64;
	}
	else if( ret > 64 && ret <= 128 )
	{
		ret = LL_SPI_BAUDRATEPRESCALER_DIV128;
	}
	else if( ret > 128 )
	{
		ret = LL_SPI_BAUDRATEPRESCALER_DIV256;
	}
	return ret;
}

//! SPI Interrupt handler
void spi_master::interrupt_handler() noexcept
{
	char stat { char((m_rxptr?0:1)|(m_txptr?0:2)) };
	if(LL_SPI_IsActiveFlag_RXNE(io<SPI_TypeDef>()) && m_rxptr) {
		if(m_rxi<m_rxsiz) {
			m_rxptr[m_rxi++]=LL_SPI_ReceiveData8(io<SPI_TypeDef>());
		} else {
			isix::inval_dcache_by_addr(m_rxptr,m_rxsiz);
			stat |= 1;
		}
	}
	if(LL_SPI_IsActiveFlag_TXE(io<SPI_TypeDef>()) && m_txptr) {
		if(m_txi<m_txsiz) {
			LL_SPI_TransmitData8(io<SPI_TypeDef>(),m_txptr[m_txi++]);
		} else {
			stat |= 2;
		}
	}
	if(LL_SPI_IsActiveFlag_OVR(io<SPI_TypeDef>())) {
		stat |= 0x80;
	}
	if(stat&0x80 || stat&0x03) {
		cs(true, m_ccs);
	}
	if(stat&0x80) {
		LL_SPI_Disable(io<SPI_TypeDef>());
		finalize_transfer(error::overrun);
	}
	else if(stat&0x03)
	{
		auto item = m_transq.front();
		if( item ) {
			m_transq.pop();
			start_transfer(*item);
			periphint_config();
		} else {
			finalize_transfer(error::success);
		}
	}
}

//! SPI master chip select
inline void spi_master::cs(bool state,int no) noexcept
{
	gpio::set(m_cs[no], state);
}

//! Finalize transfer
void spi_master::finalize_transfer(int err) noexcept
{
	*m_ret =  err;
	m_rxptr = nullptr; m_txptr = nullptr;
	m_rxsiz = m_txsiz = 0;
	m_wait.signal_isr();
}

//! Start transfer data
void spi_master::start_transfer(trans_type trans) noexcept
{
	auto [addr,data,ret] = trans;
	switch(data.type()) {
		case blk::transfer::rx: {
									auto t = static_cast<const blk::rx_transfer_base&>(data);
									m_rxptr = reinterpret_cast<char*>(t.buf());
									m_rxsiz = t.size();
									m_txptr = nullptr;
									break;
								}
		case blk::transfer::tx: {
									auto t = static_cast<const blk::tx_transfer_base&>(data);
									m_txptr = reinterpret_cast<const char*>(t.buf());
									m_txsiz = t.size();
									isix::clean_dcache_by_addr(const_cast<char*>(m_txptr),m_txsiz);
									m_rxptr = nullptr;
									break;
								}
		case blk::transfer::trx: {
										auto t = static_cast<const blk::trx_transfer_base&>(data);
										m_txptr = reinterpret_cast<const char*>(t.tx_buf());
										m_rxptr = reinterpret_cast<char*>(t.rx_buf());
										m_rxsiz = m_txsiz = t.size();
										isix::clean_dcache_by_addr(const_cast<char*>(m_txptr),m_txsiz);
										break;
									}
	}
	m_rxi = m_txi = 0;
	m_ccs = addr;
	m_ret = &ret;
}

// Configure hardware peripherial when transfer mode is set
void spi_master::periphint_config() noexcept
{
	if(m_rxptr) LL_SPI_EnableIT_RXNE(io<SPI_TypeDef>());
	if(m_txptr) LL_SPI_EnableIT_TXE(io<SPI_TypeDef>());
}

}
