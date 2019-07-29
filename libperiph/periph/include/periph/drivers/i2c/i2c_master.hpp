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
#include <atomic>

namespace periph::blk {
	class transfer;
}

namespace periph::drivers::i2c::_handlers {
	enum class htype : bool ;
}

namespace periph::drivers {
	class i2c_master final : public block_device
	{
	public:
		explicit i2c_master(const char name[]);
		virtual ~i2c_master();
		i2c_master(i2c_master&) = delete;
		i2c_master& operator=(i2c_master&) = delete;
		int transaction(int addr, const blk::transfer& data) override;
	protected:
		int do_open(int timeout) override;
		int do_close() override;
		int do_set_option(const option::device_option& opt) override;
	private:
		void interrupt_handler(i2c::_handlers::htype) noexcept;
		int periph_conf(bool en) noexcept;
		int get_hwerror() const noexcept;
		void ev_finalize(bool inv_state) noexcept;
	private:
		volatile uint8_t m_addr {};
		volatile uint8_t m_hw_error {};
		bool m_dma {};
		const volatile uint8_t* m_txdata {};
		volatile uint8_t* m_rxdata {};
		std::atomic<unsigned short> m_txdsize {};
		std::atomic<unsigned short> m_rxdsize {};
		std::atomic<unsigned short> m_datacnt {};
		unsigned short m_timeout {};
		isix::mutex m_mtx;
		isix::semaphore m_wait {0,1};
	};
}
