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
#ifdef _HAVE_CONFIG_H
#include <config.h>
#endif
#define CONFIG_ISIXDRV_I2C_1 1
#define CONFIG_ISIXDRV_I2C_2 2
/**
 * If channel is defined only one ch is enabled
 * #define CONFIG_ISIXDRV_I2C_USE_FIXED_I2C 1 
 * Value 1 , 2 , 3  are allowed if not defined all is enabled
 */
//#define CONFIG_ISIXDRV_I2C_USE_FIXED_I2C CONFIG_ISIXDRV_I2C_1 
/* ------------------------------------------------------------------ */ 
namespace stm32 {
namespace drv {
/* ------------------------------------------------------------------ */ 
extern "C" {
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C)
	void i2c1_ev_isr_vector() __attribute__ ((interrupt));
	void i2c1_er_isr_vector() __attribute__ ((interrupt));
	void i2c2_ev_isr_vector() __attribute__ ((interrupt));
	void i2c2_er_isr_vector() __attribute__ ((interrupt));
	void dma1_stream0_isr_vector() __attribute__((interrupt));
	void dma1_stream7_isr_vector() __attribute__((interrupt));
#elif CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1
	void i2c1_ev_isr_vector() __attribute__ ((interrupt));
	void i2c1_er_isr_vector() __attribute__ ((interrupt));
	void dma1_stream0_isr_vector() __attribute__((interrupt));
#elif CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2
	void i2c2_ev_isr_vector() __attribute__ ((interrupt));
	void i2c2_er_isr_vector() __attribute__ ((interrupt));
	void dma1_stream7_isr_vector() __attribute__((interrupt));
#else
#error Unknown I2C
#endif
}
/* ------------------------------------------------------------------ */ 
class i2c_host : private fnd::noncopyable {
	static constexpr auto IRQ_PRIO = 1;
	static constexpr auto IRQ_SUB = 7;
	static constexpr auto TRANSACTION_TIMEOUT = 5000;
	
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C)
	friend void i2c1_ev_isr_vector();
	friend void i2c1_er_isr_vector();
	friend void i2c2_ev_isr_vector();
	friend void i2c2_er_isr_vector();
	friend void dma1_stream0_isr_vector();
	friend void dma1_stream7_isr_vector();
#elif CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1 
	friend void i2c1_ev_isr_vector();
	friend void i2c1_er_isr_vector();
	friend void dma1_stream0_isr_vector();
#elif CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2
	friend void i2c2_ev_isr_vector();
	friend void i2c2_er_isr_vector();
	friend void dma1_stream7_isr_vector();
#endif
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
		err_invstate = - 5007,			//! Invalid machine state
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
	//!Ev irq DMA version
	void ev_irq_dma();
	//!Ev irq noDMA version
	void ev_irq_no_dma();
	//! Event DMA transfer complete
	void ev_dma_tc();
	//! Finalize transaction
	void ev_finalize( int err = 0 );
private:
#ifndef CONFIG_ISIXDRV_I2C_USE_FIXED_I2C
	void* const m_i2c;					//! I2C
#endif
	volatile uint8_t m_err_flag {};		//! Error code
	volatile uint8_t m_addr {};			//! Current addr
	volatile bool m_use_dma {};			//! Use DMA on BIG tran
	volatile uint16_t m_rx_len {};		//! RX trans
	uint8_t* m_rx_buf {};				//! RX buffer
	isix::semaphore m_lock {1, 1};		//! Lock semaphore
	isix::semaphore m_notify { 0, 1 };	//! Notify semaphore

};
/* ------------------------------------------------------------------ */ 

}
}
