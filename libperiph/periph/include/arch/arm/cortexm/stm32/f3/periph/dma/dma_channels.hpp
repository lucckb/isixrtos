/*
 * =====================================================================================
 *
 *       Filename:  dma_channels.hpp
 *
 *    Description:  DMA channels for stm32
 *
 *        Version:  1.0
 *        Created:  26.07.2018 21:44:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <periph/dma/types.hpp>

namespace periph::dma::devid {

	enum _devid : chnid_t {
		mem,
		adc1, adc2, spi1_rx, spi1_tx, usart3_tx, usart3_rx, //a
		usart1_rx, usart1_tx, usart2_tx, usart2_rx,		//b
		i2c1_rx, i2c1_tx, tim1_ch1, tim1_ch2, tim1_ch4,	//c
		tim1_trig,tim1_com, tim1_up, tim1_ch3, tim2_ch3,	//d
		tim2_up, tim2_ch1, tim2_ch2, tim2_ch4, tim3_ch3,	//e
		tim3_ch4, tim3_up, tim3_ch1, tim3_trig, tim6_up,	//f
		dac1_ch1, tim7_up, dac1_ch2, dac2_ch1, tim15_ch1,	//g
		tim15_up, tim15_trig, tim15_com, tim16_ch1, tim16_up, //h
		tim17_ch1, tim17_up, hrtim1_m, hrtim1_a, hrtim1_b,	//i
		hrtim1_c, hrtim1_d, hrtim1_e, //j
		_devid_end	//Enumeration termination mark
	};

	namespace detail {
			//! Standard mapping
			template <typename... T> constexpr auto _chb(T... args) {
				return ( ... | (1U<<(args-1)) );
			}
		}
		//Remaping channel for version 1
		struct alt_remap {
			chnid_t devid;			//! Transfer device identifier
			uint8_t chn;			//! Number of channel mapped
			uint32_t set_mask;		//! Set mask
			uint32_t clr_mask;		//! Clear mask
		};


	/** DMA channel mapping to the device assignment
	 */
	namespace detail {
		static constexpr unsigned char dev_chn_map [[maybe_unused]] [] = {
			0xff,
			_chb(1), _chb(2,4), _chb(2,4,6), _chb(3,5,7),_chb(2),_chb(2), //a
			_chb(5), _chb(4), _chb(7), _chb(6),	//b
			_chb(3,5,7), _chb(2,4,6), _chb(2), _chb(3), _chb(4),	//c
			_chb(4), _chb(4), _chb(5), _chb(6), _chb(1),	//d
			_chb(2), _chb(5), _chb(7), _chb(7), _chb(2),	//e
			_chb(3), _chb(3), _chb(6), _chb(6), _chb(3),	//f
			_chb(3), _chb(4), _chb(4), _chb(5), _chb(5),	//g
			_chb(5), _chb(5), _chb(5), _chb(3), _chb(3),	//h
			_chb(1), _chb(1), _chb(2), _chb(3), _chb(4),	//i
			_chb(5), _chb(6), _chb(7)						//j
		};
		/** Remapping table */
		static constexpr alt_remap remaping_table [[maybe_unused]] [] = {
			{ adc2, 2,  0x200, 0x100 },
			{ adc2, 4,  0x300, 0x000 },
			{ spi1_rx, 2,  0x0, 0x3 },
			{ spi1_rx, 4,  0x1, 0x2 },
			{ spi1_rx, 6,  0x2, 0x1 },
			{ spi1_tx, 3,  0x0, 0xC },
			{ spi1_tx, 5,  0x4, 0x8 },
			{ spi1_tx, 7,  0x8, 0x4 },
			{ i2c1_tx, 2,  0x40, 0x80 },
			{ i2c1_tx, 4,  0x80, 0x40 },
			{ i2c1_tx, 6,  0xc0, 0x00 },
			{ i2c1_rx, 3,  0x1, 0x2 },
			{ i2c1_rx, 5,  0x1, 0x2 },
			{ i2c1_rx, 7,  0x1, 0x2 },
		};
	}
}

