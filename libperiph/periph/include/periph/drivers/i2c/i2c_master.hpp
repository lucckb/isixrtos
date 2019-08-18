/*
 * =====================================================================================
 *
 *       Filename:  i2c_master.hpp
 *
 *    Description:  I2C master driver for ISIX RTOS
 *
 *        Version:  1.0
 *        Created:  21.06.2019 16:08:34
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <periph/core/block_device.hpp>
#include <isix/mutex.hpp>
#include <isix/semaphore.hpp>
#include <periph/core/block_device.hpp>
#include <periph/dma/controller.hpp>
#include <atomic>

//! Forward declaration for transfer
namespace periph::blk {
	class transfer;
}

//Handler type forward declaration
namespace periph::drivers::i2c::_handlers {
	enum class htype : bool ;
}

namespace periph::drivers {
	// I2C master classs
	class i2c_master final : public block_device
	{
	public:
		/** Construct device by name
		 * @param[in] name Device name
		 */
		explicit i2c_master(const char name[]);
		//! Destructor
		virtual ~i2c_master();
		//! Deleted copy constructor
		i2c_master(i2c_master&) = delete;
		//! Deleted assigne operator
		i2c_master& operator=(i2c_master&) = delete;
		/** Make single bus transaction
		 * @param[in] addr i2c transfer
		 * @param[in] data Transfer data
		 * @return error code
		 */
		int transaction(int addr, const blk::transfer& data) override;
	protected:
		//! Do open internal implementation
		int do_open(int timeout) override;
		//! Do close internal implementation
		int do_close() override;
		//! Do set option internal implementation
		int do_set_option(const option::device_option& opt) override;
	private:
		//! Interrupt handler called from interrupt context
		void interrupt_handler(i2c::_handlers::htype);
		//! Interrupt handler when DMA is used
		void interrupt_dma_handler(i2c::_handlers::htype);
		//! Handler used by interrupt controller
		void interrupt_dma_rx_controller_handler(bool err);
		//! Handler used by transmit end controller handler
		void interrupt_dma_tx_controller_handler(bool err);
		//! Internal peripheral settings
		int periph_conf(bool en) noexcept;
		//! Get hardware error from i2c bus
		int get_hwerror() const noexcept;
		//! Finalize transfer from irq context
		void ev_finalize(bool inv_state) noexcept;
		//! Handle event error
		void handle_ev_error() noexcept;
		//! Determine dma mode
		bool check_dma_mode();
	private:
		//! Bus address to send
		volatile uint8_t m_addr {};
		//! Last hardware error
		volatile uint16_t m_hw_error {};
		//! Need dma transfer
		const bool m_dma {};
		//! TX data internal pointer
		const volatile uint8_t* m_txdata {};
		//! RX data internal pointer
		volatile uint8_t* m_rxdata {};
		//! TX transfer size
		volatile unsigned short m_txdsize {};
		//! RX transfer size
		volatile unsigned short m_rxdsize {};
		//! Internal data transfer during dma
		volatile unsigned short m_datacnt {};
		//! Timeout settings
		unsigned short m_timeout {};
		isix::mutex m_mtx;
		//! Notification semaphore
		isix::semaphore m_wait {0,1};
		//! Dma receive channel
		periph::dma::controller::channel_ptr_t m_dma_rx;
		//! DMA transmit channel 
		periph::dma::controller::channel_ptr_t m_dma_tx;
	};
}
