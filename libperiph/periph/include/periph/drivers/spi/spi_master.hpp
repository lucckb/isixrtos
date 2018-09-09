/*
 * =====================================================================================
 *
 *       Filename:  spi_master.hpp
 *
 *    Description:  SPI master driver with new model
 *
 *        Version:  1.0
 *        Created:  06.08.2018 18:26:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <isix/mutex.h>
#include <isix/semaphore.h>
#include <isix/cpp/mutex.hpp>
#include <isix/cpp/semaphore.hpp>
#include <periph/core/block_device.hpp>
#include <atomic>

namespace periph::blk {
	class transfer;
}

namespace periph::drivers {

	class spi_master final : public block_device {
		static constexpr auto invcs = -1;
		using size_type = unsigned short;
		union dptr {
			unsigned char*  p8;
			unsigned short* p16;
		};
		union cdptr {
			const unsigned char*  p8;
			const unsigned short* p16;
		};
	public:
		explicit spi_master(const char name[]);
		virtual ~spi_master();
		int transaction(int addr, const blk::transfer& data) override;
	protected:
		int do_open(int timeout) override;
		int do_close() override;
	private:
		int do_set_option(const option::device_option& opt) override;
		int clk_conf(bool en);
		int gpio_conf(bool en);
		int clk_to_presc(unsigned hz);
		void interrupt_handler() noexcept;
		void cs(bool state,int no) noexcept;
		int start_transfer(const blk::transfer& tran,int& ret);
		void finalize_transfer(int err) noexcept;
		void periphint_config() noexcept;
		void periph_deconfig() noexcept;
	private:
		int m_cs[4] {invcs,invcs,invcs,invcs};
		std::atomic<size_type> m_rxsiz{}, m_txsiz{}, m_rxi {}, m_txi{};
		std::atomic<int*> m_ret;
		isix::semaphore m_wait {0,1};
		dptr m_rxptr {};
		cdptr m_txptr {};
		isix::mutex m_mtx;
		int m_timeout {};
		unsigned char m_transfer_size {8};
	};
}
