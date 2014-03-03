/*
 * =====================================================================================
 *
 *       Filename:  i2c_bus.hpp
 *
 *    Description:  I2C bus isix specific implementation with DMA support
 *
 *        Version:  1.0
 *        Created:  03.03.2014 17:21:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <foundation/noncopyable.hpp>
#include <cstdint>
#include <isix.h>
/* ------------------------------------------------------------------ */ 
namespace stm32 {
namespace drv {
/* ------------------------------------------------------------------ */ 
extern "C" {
	void i2c1_ev_isr_vector(void) __attribute__ ((interrupt));
	void i2c1_er_isr_vector(void) __attribute__ ((interrupt));
	void i2c2_ev_isr_vector(void) __attribute__ ((interrupt));
	void i2c2_er_isr_vector(void) __attribute__ ((interrupt));
}
/* ------------------------------------------------------------------ */ 
class i2c_host : private fnd::noncopyable {
	static constexpr auto IRQ_PRIO = 1;
	static constexpr auto IRQ_SUB = 7;
	static constexpr auto TRANSACTION_TIMEOUT = 5000;
	
	friend void i2c1_ev_isr_vector(void);
	friend void i2c1_er_isr_vector(void);
	friend void i2c2_ev_isr_vector(void);
	friend void i2c2_er_isr_vector(void);
public:
	enum class busid { //! Interface independent bus ID
		i2c1,			//I2C1 
		i2c2			//I2C2
	};
	//! Error code
	enum err {
		err_ok = 0,						//! all is ok
		err_bus = -5000,				//! bus error
		err_arbitration_lost = -5001,
		err_ack_failure = -5002,
		err_overrun = - 5003,
		err_pec = - 5004,				//! parity check error
		err_bus_timeout = -5005, 		//! bus timeout
		err_timeout = - 5006,			//! timeout error
		err_unknown = - 5008,
	};
	/** Constructor
	 * @param[in] _i2c Interface bus ID
	 * @param[in] clk_speed CLK speed in HZ
	 */
	i2c_host( busid _i2c, unsigned clk_speed=100000 );
	/** 
	 * Destructor
	 */
	~i2c_host();
	/** Transfer one byte over i2c interface
	 * @param[in] addr I2C address
	 * @param[in] wbuffer Memory pointer for write
	 * @param[in] wsize  Size of write buffer
	 * @param[out] rbuffer Read data buffer pointer
	 * @param[in] rsize Read buffer sizes
	 * @return Error code or success */
	int transfer_7bit(uint8_t addr, const void* wbuffer, short wsize, void* rbuffer, short rsize);
private:
	//! Get hardware error code
	int get_hwerror(void) const;
	//Interrupt event handler
	void ev_irq();
	//Error event handler
	void err_irq();
private:
	void* const m_i2c;					//! I2C 
	volatile uint8_t m_err_flag {};		//! Error code
	volatile uint8_t m_addr {};			//! Current addr
	volatile bool m_rx_trans {};		//! RX trans
	volatile bool m_tx_trans {};		//! TX trans
	isix::semaphore m_lock {1, 1};		//! Lock semaphore
	isix::semaphore m_notify { 0, 1 };	//! Notify semaphore

};
/* ------------------------------------------------------------------ */ 

}
}
