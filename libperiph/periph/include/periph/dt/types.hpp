/*
 * =====================================================================================
 *
 *       Filename:  types.hpp
 *
 *    Description:  Device tree types
 *
 *        Version:  1.0
 *        Created:  05.03.2018 21:03:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

namespace periph::dt {

	//! Available buses
	enum class bus : short {
		_empty,
        unspec,
		axi,
		ahb1,
		apb1,
		apb2,
		cpu,
		ahb3
	};

	//! Pin function type
	enum class pinfunc : short {
		_empty,
		sck, miso, mosi,
		cs0, cs1, cs2, cs3,
		rxd, txd,
        rst, rw,
		sda, scl,
		fmc_sdcke1, fmc_sdne1, fmc_sdnwe, fmc_d2, fmc_d3, fmc_d13, fmc_d14, fmc_d15,
		fmc_d0, fmc_d1, fmc_nbl0, fmc_nbl1, fmc_d4, fmc_d5, fmc_d6, fmc_d7, fmc_d8,
		fmc_d9, fmc_d10, fmc_d11, fmc_d12, fmc_a0, fmc_a1, fmc_a2, fmc_a3, fmc_a4,
		fmc_a5, fmc_sdnras, fmc_a6, fmc_a7, fmc_a8, fmc_a9, fmc_a10, fmc_a11, fmc_a14,
		fmc_a15, fmc_sdclk, fmc_sdncas, fmc_sdcke0, fmc_sdne0, fmc_d16, fmc_d17, fmc_d18,
		fmc_d19, fmc_d20, fmc_d21, fmc_d22, fmc_d23, fmc_d24, fmc_d25, fmc_d26, fmc_d27, 
		fmc_nbl2, fmc_nbl3, fmc_d28, fmc_d29, fmc_d30, fmc_d31,
		dsi_te, lcd_reset, lcd_backlight
	};

	//++ operator
	inline pinfunc operator++(pinfunc& pf) {
		pf = static_cast<pinfunc>(static_cast<int>(pf)+1);
		return pf;
	}

	//! Clk periph information structure
	struct clk_periph {
		bus xbus;				//!	Assigned bus
		unsigned bit;			//! Peripheral bit number for enable device
	};

    struct device_conf_base {
    };

	//! Device configuration internal hardware
	struct device_conf : public device_conf_base {
		enum _flags {
			fl_dma  = 1U<<0U,		//Use DMA flags
		};
		int irqnum;				//! Irq number
		unsigned short irqfl;	//! Irq low flags
		unsigned short irqfh;	//! Irq hi con
        unsigned long flags;	//! Base flags
	};

}
