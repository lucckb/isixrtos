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
#include <foundation/drv/bus/ibus.hpp>
#include <cstdint>
#include <isix.h>
#include <config/conf.h>
#define CONFIG_ISIXDRV_I2C_1 1
#define CONFIG_ISIXDRV_I2C_2 2
/**
 * CONFIG_ISIXDRV_I2C_NODMA 1 disable DMA handling
 * If channel is defined only one ch is enabled
 * #define CONFIG_ISIXDRV_I2C_USE_FIXED_I2C 1
 * Value 1 , 2 , 3  are allowed if not defined all is enabled
 */
/** CAUTION On some F20x devices exists bug in I2C hardware !!
	Description
	In case of a repeated Start, the “Setup time for a repeated Start condition” (named Tsu;sta in
	the I2C specification) can be slightly violated when the I2C operates in Master Standard
	mode at a frequency between 88 kHz and 100 kHz.
	The issue can occur only in the following configuration:
	 - in Master mode
	in Standard mode at a frequency between 88 kHz and 100 kHz (no issue in Fast-mode)
	SCL rise time:
	–
	If the slave does not stretch the clock and the SCL rise time is more than 300 ns (if
	the SCL rise time is less than 300 ns, the issue cannot occur)
	–
	If the slave stretches the clock
	The setup time can be violated independently of the APB peripheral frequency.
 */
//#define CONFIG_ISIXDRV_I2C_USE_FIXED_I2C CONFIG_ISIXDRV_I2C_1

namespace stm32 {
namespace drv {

extern "C" {
#if defined(STM32MCU_MAJOR_TYPE_F1) && !CONFIG_ISIXDRV_I2C_NODMA
	void dma1_channel7_isr_vector();
	void dma1_channel5_isr_vector();
#endif
#if CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1
	void i2c1_ev_isr_vector() ;
	void i2c1_er_isr_vector() ;
#	if !CONFIG_ISIXDRV_I2C_NODMA
	void dma1_stream0_isr_vector();
#	endif
#elif CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2
	void i2c2_ev_isr_vector() ;
	void i2c2_er_isr_vector() ;
#	if !CONFIG_ISIXDRV_I2C_NODMA
	void dma1_stream2_isr_vector();
#	endif
#else
	void i2c1_ev_isr_vector();
	void i2c1_er_isr_vector();
	void i2c2_ev_isr_vector();
	void i2c2_er_isr_vector();
#	if !CONFIG_ISIXDRV_I2C_NODMA
	void dma1_stream0_isr_vector();
	void dma1_stream2_isr_vector();
#	endif
#endif
}

class i2c_bus : public fnd::drv::bus::ibus {
	static constexpr auto IRQ_PRIO = 1;
	static constexpr auto IRQ_SUB = 2;
	static constexpr auto TRANSACTION_TIMEOUT = 5000;

#if CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1
	friend void i2c1_ev_isr_vector();
	friend void i2c1_er_isr_vector();
#if	!CONFIG_ISIXDRV_I2C_NODMA
	friend void dma1_stream0_isr_vector();
#endif
#elif CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2
	friend void i2c2_ev_isr_vector();
	friend void i2c2_er_isr_vector();
#	if !CONFIG_ISIXDRV_I2C_NODMA
	friend void dma1_stream2_isr_vector();
#	endif
#else
	friend void i2c1_ev_isr_vector();
	friend void i2c1_er_isr_vector();
	friend void i2c2_ev_isr_vector();
	friend void i2c2_er_isr_vector();
#if !CONFIG_ISIXDRV_I2C_NODMA
	friend void dma1_stream0_isr_vector();
	friend void dma1_stream2_isr_vector();
#	endif
#endif
#	if !CONFIG_ISIXDRV_I2C_NODMA
	friend void dma1_channel7_isr_vector();
	friend void dma1_channel5_isr_vector();
#	endif
public:
	enum class busid { //! Interface independent bus ID
		i2c1,			//I2C1
		i2c2,			//I2C2
		i2c1_alt,		//Alternate PINS on i2c1 bus
		i2c2_alt		//I2C2 on the alternate pinout
	};
	/** Constructor
	 * @param[in] _i2c Interface bus ID
	 * @param[in] clk_speed CLK speed in HZ
	 */
#ifdef CONFIG_PCLK1_HZ
	i2c_bus( busid _i2c, unsigned clk_speed=100000, unsigned pclk1 = CONFIG_PCLK1_HZ );
#else
	i2c_bus( busid _i2c, unsigned clk_speed, unsigned pclk1 );
#endif
	/**
	 * Destructor
	 */
	virtual ~i2c_bus();
	/** Transfer one byte over i2c interface
	 * @param[in] addr I2C address
	 * @param[in] wbuffer Memory pointer for write
	 * @param[in] wsize  Size of write buffer
	 * @param[out] rbuffer Read data buffer pointer
	 * @param[in] rsize Read buffer sizes
	 * @return Error code or success */
	int transfer(unsigned addr, const void* wbuffer,
			size_t wsize, void* rbuffer, size_t rsize) override
	{
		//STM32F1 Medium density Errata
		//Some software events must be managed before the current byte is being transferred
		//Workarround1 (USE DMA but number of bytest must be > 1)
#		if defined(STM32MCU_MAJOR_TYPE_F1)
		if( rsize == 1 ) {
			unsigned char tmprx[2];
			const auto ret = transfer_impl( addr, wbuffer, wsize, tmprx, sizeof tmprx );
			if( !ret ) {
				*reinterpret_cast<unsigned char*>(rbuffer) = tmprx[0];
			}
			return ret;

		} else {
			return transfer_impl( addr, wbuffer, wsize, rbuffer, rsize );
		}
#		else
			return transfer_impl( addr, wbuffer, wsize, rbuffer, rsize );
#		endif
	}

	/** Double non continous transaction write
	 * @param[in] addr I2C address
	 * @param[in] wbuf1 Write buffer first transaction
	 * @param[in] wsize1 Transaction size 1
	 * @param[in] wbuf2 Write buffer first transaction
	 * @param[in] wsize2 Transaction size 1
	 * @return error code or success */
	int write( unsigned addr, const void* wbuf1, size_t wsize1,
			const void* wbuf2, size_t wsize2 ) override;

	/** Mdelay bus tout impl */
	void mdelay( unsigned timeout ) noexcept override;
private:
	//! Hardware init
	void gpio_initialize( bool alt );
	//! Get hardware error code
	int get_hwerror(void) const;
	//Interrupt event handler
	void ev_irq();
	//Error event handler
	void err_irq();
#if !CONFIG_ISIXDRV_I2C_NODMA
	//! Event DMA transfer complete
	void ev_dma_tc();
#endif
	//! Finalize transaction
	void ev_finalize( bool state_err = false );
	int transfer_impl(unsigned addr, const void* wbuffer,
			size_t wsize, void* rbuffer, size_t rsize);
private:
#if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C
	void* const m_i2c;					//! I2C
#endif
	volatile uint8_t m_err_flag {};		//! Error code
	volatile uint8_t m_addr {};			//! Current addr
	volatile uint16_t m_rx_len {};		//! RX trans
	uint8_t* volatile m_rx_buf {};		//! RX buffer
#if CONFIG_ISIXDRV_I2C_NODMA
	volatile uint16_t m_tx_len {};		//! TX trans
	const uint8_t* volatile m_tx_buf {};//! TX buffer
	volatile uint16_t m_rx_cnt {};
	volatile uint16_t m_tx_cnt {};
	volatile uint16_t m_tx2_cnt {};
#endif
	volatile uint16_t m_tx2_len {};		//! RX trans
	const uint8_t* volatile m_tx2_buf {};		//! RX buffer
	isix::semaphore m_lock {1, 1};		//! Lock semaphore
	isix::semaphore m_notify { 0, 1 };	//! Notify semaphore
};

}
}
