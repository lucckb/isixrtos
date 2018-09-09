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
#include <isix.h>
#include <stm32f3xx_ll_spi.h>
#include <foundation/sys/dbglog.h>
#include "spi_interrupt_handlers.hpp"
#include <type_traits>


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
			LL_SPI_Enable(io<SPI_TypeDef>());
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
	//dbg_info("spi_master::transaction");
	int ret {};
	if(addr<0 || addr>=int(sizeof(m_cs)/sizeof(m_cs[0]))) {
		dbg_err("Invalid address");
		return error::invaddr;
	}
	isix::mutex_locker _lock(m_mtx);
	ret = start_transfer(std::ref(data),std::ref(ret));
	if( ret ) {
		dbg_info("Invalid arguments");
		return ret;
	}
	cs(false,addr);
	periphint_config();
	ret = m_wait.wait(m_timeout);
	//Now finalize transfer
	constexpr auto duration = 50U;
	{
		const auto tb=isix::get_jiffies();
		while(!isix::timer_elapsed(tb,duration)) {
			if(!LL_SPI_IsActiveFlag_BSY(io<SPI_TypeDef>())) {
				break;
			}
		}
		if(isix::timer_elapsed(tb,duration)) {
			dbg_info("timeout");
			return error::bus_timeout;
		}
	}
	cs(true,addr);
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
			m_transfer_size = dw;
			break;
		}
		case option::ord::bitorder: {
			auto ord = static_cast<const option::bitorder&>(opt).order();
			LL_SPI_SetTransferBitOrder(io<SPI_TypeDef>(),
				ord==option::bitorder::msb?LL_SPI_MSB_FIRST:LL_SPI_LSB_FIRST
			);
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
	do {
		if((ret=dt::get_periph_clock(io<void>(),pclk))<0) break;
		if(en) {
			if((ret=clock::device_enable(pclk))<0) break;
		} else {
			if((ret=clock::device_disable(pclk))<0) break;
		}
	} while(0);
	dbg_info("clock setup status %i", ret);
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
	dbg_info("Pin exit success %i", en);
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
	enum tstat {srxfin=1,stxfin=2,sfin=0x03 };
	char stat { char((m_rxptr.p8?0:srxfin)|(m_txptr.p8?0:stxfin)) };
	if(LL_SPI_IsActiveFlag_RXNE(io<SPI_TypeDef>())) {
		if(m_rxptr.p8) {
			if(m_rxi<m_rxsiz) {
				if(m_transfer_size<=8)
					m_rxptr.p8[m_rxi++]=LL_SPI_ReceiveData8(io<SPI_TypeDef>());
				else
					m_rxptr.p16[m_rxi++]=LL_SPI_ReceiveData16(io<SPI_TypeDef>());
			} else {
				size_type cw = m_rxsiz;
				if(m_transfer_size>8) cw *= 2U;
				isix::inval_dcache_by_addr(m_rxptr.p8,cw);
				stat |= srxfin;
			}
		} else {
			if(m_transfer_size<=8)
				LL_SPI_ReceiveData8(io<SPI_TypeDef>());
			else
				LL_SPI_ReceiveData16(io<SPI_TypeDef>());
		}
	}
	if(m_txptr.p8) {
		if(LL_SPI_IsActiveFlag_TXE(io<SPI_TypeDef>())) {
			if(m_txi<m_txsiz) {
				if(m_transfer_size<=8)
					LL_SPI_TransmitData8(io<SPI_TypeDef>(),m_txptr.p8[m_txi++]);
				else
					LL_SPI_TransmitData16(io<SPI_TypeDef>(),m_txptr.p16[m_txi++]);
			} else {
				stat |= stxfin;
			}
		} else {
			if(m_transfer_size<=8)
				LL_SPI_TransmitData8(io<SPI_TypeDef>(),0xff);
			else
				LL_SPI_TransmitData16(io<SPI_TypeDef>(),0xffff);
		}
	}
	if(stat!=sfin && LL_SPI_IsActiveFlag_OVR(io<SPI_TypeDef>())) {
		finalize_transfer(error::overrun);
	}
	if((stat&sfin)==sfin)
	{
		finalize_transfer(error::success);
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
	m_rxptr.p8 = nullptr; m_txptr.p8 = nullptr;
	m_rxsiz = m_txsiz = 0;
	m_wait.signal_isr();
	periph_deconfig();
}

//! Start transfer data
int spi_master::start_transfer(const blk::transfer& data,int& ret) noexcept
{
	switch(data.type()) {
		case blk::transfer::rx: {
					auto& t = static_cast<const blk::rx_transfer_base&>(data);
					if(m_transfer_size<=8) {
						m_rxptr.p8 = reinterpret_cast<decltype(m_rxptr.p8)>(t.buf());
						m_rxsiz = t.size();
					} else {
						m_rxptr.p16 = reinterpret_cast<decltype(m_rxptr.p16)>(t.buf());
						m_rxsiz = t.size()/2;
					}
					m_txptr.p8 = nullptr;
					break;
		}
		case blk::transfer::tx: {
					auto& t = static_cast<const blk::tx_transfer_base&>(data);
					//dbg_info("TXSIZ %u", t.size());
					if(m_transfer_size<=8) {
						m_txptr.p8 = reinterpret_cast<decltype(m_txptr.p8)>(t.buf());
						m_txsiz = t.size();
					} else {
						m_txptr.p16 = reinterpret_cast<decltype(m_txptr.p16)>(t.buf());
						m_txsiz = t.size()/2;
					}
					isix::clean_dcache_by_addr(const_cast<void*>(t.buf()),t.size());
					m_rxptr.p8 = nullptr;
					break;
		}
		case blk::transfer::trx: {
					auto& t = static_cast<const blk::trx_transfer_base&>(data);
					if(m_transfer_size<=8) {
						m_txptr.p8 = reinterpret_cast<decltype(m_txptr.p8)>(t.tx_buf());
						m_rxptr.p8 = reinterpret_cast<decltype(m_rxptr.p8)>(t.rx_buf());
						m_rxsiz = m_txsiz = t.size();
					} else {
						m_txptr.p16 = reinterpret_cast<decltype(m_txptr.p16)>(t.tx_buf());
						m_rxptr.p16 = reinterpret_cast<decltype(m_rxptr.p16)>(t.rx_buf());
						m_rxsiz = m_txsiz = t.size()/2;
					}
					isix::clean_dcache_by_addr(const_cast<void*>(t.tx_buf()),t.size());
					break;
		}
	}
	m_rxi = m_txi = 0;
	m_ret = &ret;
	return (!m_rxptr.p8&&!m_txptr.p8&&!m_rxsiz&&!m_txsiz)?int(error::inval):int(error::success);
}

// Configure hardware peripherial when transfer mode is set
void spi_master::periphint_config() noexcept
{
	if(m_rxptr.p8) LL_SPI_EnableIT_RXNE(io<SPI_TypeDef>());
	if(m_txptr.p8) LL_SPI_EnableIT_TXE(io<SPI_TypeDef>());
	//dbg_info("RXNE %i TXE %i", !!m_rxptr, !!m_txptr);
	if(m_rxptr.p8||m_txptr.p8) LL_SPI_EnableIT_ERR(io<SPI_TypeDef>());
}

void spi_master::periph_deconfig() noexcept
{
	LL_SPI_DisableIT_RXNE(io<SPI_TypeDef>());
	LL_SPI_DisableIT_TXE(io<SPI_TypeDef>());
	LL_SPI_DisableIT_ERR(io<SPI_TypeDef>());
}

}
