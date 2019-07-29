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
		int ret = i2c::_handlers::register_handler(io<I2C_TypeDef>(),
		std::bind(&i2c_master::interrupt_handler,std::ref(*this), std::placeholders::_1) );
		error::expose<error::bus_exception>(ret);
		dt::device_conf cnf;
		ret = dt::get_periph_devconf(io<void>(),cnf);
		error::expose<error::bus_exception>(ret);
		m_dma = (cnf.flags&dt::device_conf::fl_dma)?(true):(false);
	}
}

//! Destructor
i2c_master::~i2c_master()
{
	close();
}

//! DO open
int i2c_master::do_open(int timeout)
{
	int ret {};
	do {
		// Check for timeout limits
		if(timeout<0 || timeout>=std::numeric_limits<decltype(timeout)>::max()) {
			ret = error::inval;
			dbg_err("Invalid timeout arg");
			break;
		}
		m_timeout = timeout;
		ret = periph_conf(true);
		if(ret) {
			dbg_err("perip config failed %i",ret);
			break;
		}
		dt::device_conf cnf;
		if((ret=dt::get_periph_devconf(io<void>(),cnf))<0) {
			dbg_err("Get periph devconf fail %i",ret);
			break;
		}
		isix::set_irq_priority(cnf.irqnum, {uint8_t(cnf.irqfh), uint8_t(cnf.irqfl)});
		isix::request_irq(cnf.irqnum); error::expose<error::bus_exception>(ret);
		//+1 ERR IRQN
		isix::set_irq_priority(cnf.irqnum+1, {uint8_t(cnf.irqfh), uint8_t(cnf.irqfl)});
		isix::request_irq(cnf.irqnum+1); error::expose<error::bus_exception>(ret);
		LL_I2C_Disable(io<I2C_TypeDef>());
		if((ret=dt::get_periph_clock(io<void>()))<0) {
			dbg_err("Get periph clock fail %i",ret);
			break;
		}
		//Default speed configuration
		LL_I2C_Disable(io<I2C_TypeDef>());
		LL_I2C_ConfigSpeed(io<I2C_TypeDef>(),ret,100'000,LL_I2C_DUTYCYCLE_2);
		ret = 0;
		LL_I2C_SetMode(io<I2C_TypeDef>(),LL_I2C_MODE_I2C);
		LL_I2C_Enable(io<I2C_TypeDef>());
		LL_I2C_DisableIT_EVT(io<I2C_TypeDef>());
		LL_I2C_DisableIT_ERR(io<I2C_TypeDef>());
		//! Bug!
		if(LL_I2C_IsActiveFlag_STOP(io<I2C_TypeDef>())) {
			dbg_info("Active I2c controller bug");
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
int i2c_master::transaction(int addr, const blk::transfer& data)
{
	//1 Means RO 0xff MAX I2C addr
	if(addr<0 || addr>=0xff || (addr&1)) {
		dbg_err("Invalid address %i",addr);
		return error::invaddr;
	}
	isix::mutex_locker _lock(m_mtx);
	m_addr = addr;
	m_hw_error = 0;
	switch(data.type()) {
		//Rx transfer
		case blk::transfer::rx: {
			auto& t = static_cast<const blk::rx_transfer_base&>(data);
			m_addr |= I2C_OAR1_ADD0;
			m_rxdata = reinterpret_cast<volatile uint8_t*>(t.buf());
			m_txdata = nullptr;
			m_rxdsize = t.size();
			m_txdsize = 0;
			m_rxsw = true;
			break;
		}
		//Tx transfer
		case blk::transfer::tx: {
			auto& t = static_cast<const blk::tx_transfer_base&>(data);
			m_addr &= ~(I2C_OAR1_ADD0);
			m_txdata = reinterpret_cast<const volatile uint8_t*>(t.buf());
			m_rxdata = nullptr;
			m_txdsize = t.size();
			m_rxdsize = 0;
			m_rxsw = false;
			isix::clean_dcache_by_addr(const_cast<void*>(t.buf()),t.size());
			break;
		}
		case blk::transfer::trx: {
			auto& t = static_cast<const blk::trx_transfer_base&>(data);
			m_txdata = reinterpret_cast<decltype(m_txdata)>(t.tx_buf());
			m_rxdata = reinterpret_cast<decltype(m_rxdata)>(t.rx_buf());
			m_txdsize = t.tx_size();
			m_rxdsize = t.rx_size();
			isix::clean_dcache_by_addr(const_cast<void*>(t.tx_buf()),t.size());
			// Transmit part first
			m_addr &= ~(I2C_OAR1_ADD0);
			m_rxsw = false;
			break;
		}
		default:
			return error::not_supported;
	}
	m_datacnt = 0;
	LL_I2C_EnableIT_ERR(io<I2C_TypeDef>());
	LL_I2C_EnableIT_EVT(io<I2C_TypeDef>());
	LL_I2C_AcknowledgeNextData(io<I2C_TypeDef>(),LL_I2C_ACK);
	LL_I2C_GenerateStartCondition(io<I2C_TypeDef>());
	int ret = m_wait.wait(m_timeout);
	if(ret<0) {
		LL_I2C_GenerateStopCondition(io<I2C_TypeDef>());
	} else {
		ret = get_hwerror();
	}
	dbg_info("Transaction result %i", ret );
	return ret;
}

//Translate error flag to bus error
int i2c_master::get_hwerror(void) const
{
	static constexpr int err_tbl[] =
	{
		error::bus,
		error::arbitration_lost,
		error::ack_failure,
		error::overrun,
		error::pec,
		error::unknown,
		error::bus_timeout,
		error::invstate
	};
	for(int i=0; i<8; i++) {
		if(m_hw_error & (1<<i)) {
			return err_tbl[i];
		}
	}
	return 0;
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
	int ret { error::inval };
	switch(opt.ord()) {
		// Configure I2C speed
		case option::ord::speed: {
			dt::clk_periph pclk;
			if((ret=dt::get_periph_clock(io<void>(),pclk))<0) break;
			if((ret=dt::get_bus_clock(pclk.xbus))<0) break;
			const auto i2spd = static_cast<const option::speed&>(opt).hz();
			LL_I2C_Disable(io<I2C_TypeDef>());
			LL_I2C_ConfigSpeed(io<I2C_TypeDef>(), ret, i2spd, LL_I2C_DUTYCYCLE_2);
			LL_I2C_Enable(io<I2C_TypeDef>());
			ret = error::success;
		}
	}
	return ret;
}

//! Common interrupt handler for i2c device
void i2c_master::interrupt_handler(i2c::_handlers::htype type) noexcept
{
	if(type==i2c::_handlers::htype::ev)
	{
		auto rx_next = [this]() {
			if(m_rxdsize>0) {
				m_rxdata[m_datacnt++] = LL_I2C_ReceiveData8(io<I2C_TypeDef>());
				--m_rxdsize;
			}
		};
		//! Start byte send
		if(LL_I2C_IsActiveFlag_SB(io<I2C_TypeDef>())) {
			LL_I2C_TransmitData8(io<I2C_TypeDef>(), m_addr);
		}
		//! Receive mode callback
		if(is_rx()) {
			if(LL_I2C_IsActiveFlag_ADDR(io<I2C_TypeDef>()))  {
				if(m_rxdsize==1) {
					LL_I2C_AcknowledgeNextData(io<I2C_TypeDef>(),LL_I2C_NACK);
				}
				LL_I2C_DisableIT_BUF(io<I2C_TypeDef>());
				LL_I2C_ClearFlag_ADDR(io<I2C_TypeDef>());
			}
			if(LL_I2C_IsActiveFlag_BTF(io<I2C_TypeDef>()) ||
				LL_I2C_IsActiveFlag_RXNE(io<I2C_TypeDef>()))
			{
					rx_next();
					if(m_rxdsize==1) {
						LL_I2C_AcknowledgeNextData(io<I2C_TypeDef>(),LL_I2C_NACK);
					}
					if(m_rxdsize==0) {
						LL_I2C_GenerateStopCondition(io<I2C_TypeDef>());
					}
					if(m_rxdsize==0)  {
						LL_I2C_DisableIT_EVT(io<I2C_TypeDef>());
						LL_I2C_DisableIT_ERR(io<I2C_TypeDef>());
						LL_I2C_DisableIT_BUF(io<I2C_TypeDef>());
						m_wait.signal_isr();
					}
			}
		} else {	// TX callback
			if(LL_I2C_IsActiveFlag_ADDR(io<I2C_TypeDef>())) {
				if(m_txdsize>1) {
					LL_I2C_EnableIT_BUF(io<I2C_TypeDef>());
				}
				LL_I2C_ClearFlag_ADDR(io<I2C_TypeDef>());
			}
			if(LL_I2C_IsActiveFlag_TXE(io<I2C_TypeDef>())) {
				LL_I2C_TransmitData8(io<I2C_TypeDef>(),m_txdata[m_datacnt++]);
				--m_txdsize;
				if(m_txdsize<=0) {
					if(m_rxdsize>0) {
						//Switch to read mode
						m_addr |= I2C_OAR1_ADD0;
						LL_I2C_GenerateStartCondition(io<I2C_TypeDef>());
						LL_I2C_DisableIT_BUF(io<I2C_TypeDef>());
						m_datacnt = 0;
						return;
					} else {
						LL_I2C_GenerateStopCondition(io<I2C_TypeDef>());
						LL_I2C_DisableIT_BUF(io<I2C_TypeDef>());
						LL_I2C_DisableIT_EVT(io<I2C_TypeDef>());
						LL_I2C_DisableIT_ERR(io<I2C_TypeDef>());
						m_wait.signal_isr();

					}
				}
			}
			if(LL_I2C_IsActiveFlag_BTF(io<I2C_TypeDef>())&&(m_addr&I2C_OAR1_ADD0))
			{
				m_rxsw = true;
			}
		}
	}
	else	/* type */
	{
		static constexpr auto err_mask = 0xff00U;
		//Get hardware error flags
		m_hw_error = io<I2C_TypeDef>()->SR1 >> 8U;
		//Clear all hardware errors
		io<I2C_TypeDef>()->SR1 &= ~err_mask;
	}
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



