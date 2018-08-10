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

#include <periph/core/block_device.hpp>

namespace periph::drivers {

	class spi_master final : public block_device {
		static constexpr auto invcs = -1;
	public:
		explicit spi_master(const char name[]);
		virtual ~spi_master();
		int transaction(int addr, const blk::transfer& data) override;
	protected:
		int do_open(int timeout) override;
		int do_close() override;
	private:
		int do_set_option(option::device_option& opt) override;
		int clk_conf(bool en);
		int gpio_conf(bool en);
		int clk_to_presc(unsigned hz);
		void interrupt_handler() noexcept;
	private:
		int m_cs[4] {invcs,invcs,invcs,invcs};
	};
}
