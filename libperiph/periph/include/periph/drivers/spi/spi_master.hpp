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

#include <isix/mutex.hpp>
#include <isix/semaphore.hpp>
#include <periph/core/block_device.hpp>
#include <periph/dma/controller.hpp>
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
		int gpio_conf(bool en);
		int clk_to_presc(unsigned hz);
		void interrupt_handler();
		void cs(bool state,int no) noexcept;
		int start_transfer(const blk::transfer& tran,int& ret) noexcept;
		void finalize_transfer(int err) noexcept;
		int periphint_config() noexcept;
		void periph_deconfig() noexcept;
		void dma_interrupt_handler(bool err, bool tx);
	private:
		int m_cs[4] {invcs,invcs,invcs,invcs};
		std::atomic<size_type> m_rxsiz{}, m_txsiz{}, m_rxi {}, m_txi{};
		std::atomic<int*> m_ret;
		isix::semaphore m_wait {0,1};
		dptr m_rxptr {};
		cdptr m_txptr {};
		isix::mutex m_mtx;
		int m_timeout {};
		unsigned char m_transfer_size {};
		bool m_dma {};
		periph::dma::controller::channel_ptr_t m_dma_rx;
		periph::dma::controller::channel_ptr_t m_dma_tx;
	};
}
