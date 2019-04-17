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
 	mem,                                                //:D1
    spi3_rx, spi3_tx, spi2_rx, spi2_tx,                 //Ch0
    i2c1_rx, i2c1_tx, tim7_up,                          //Ch1
    tim4_ch1, tim4_ch2, tim4_ch3, tim4_up,              //Ch2
    i2s3_ext_rx, i2s3_ext_tx, i2s2_ext_tx,              //---
    i2s2_ext_rx, tim2_up, tim2_ch4, tim2_ch3,           //Ch3
    tim2_ch2, tim2_ch1, i2c3_rx, i2c3_tx,               //--- 
    uart5_rx, uart5_tx, uart4_rx, uart4_tx,             //Ch4
    usart3_rx, usart3_tx, usart2_rx, usart2_tx,         //---
    uart8_rx, uart8_tx, uart7_rx, uart7_tx,             //Ch5
    tim3_ch4, tim3_ch3, tim3_ch2, tim3_ch1, tim3_up,    //---
    tim5_ch4, tim5_ch3, tim5_ch2, tim5_ch1,             //Ch6
    tim5_up, tim5_trig,                                 //---
    tim6_up, i2c2_rx, i2c2_tx, dac1, dac2,              //Ch7
                                                        //:D2
    adc1, sai1_a, sai1_b, tim1_ch1, tim1_ch2, tim1_ch3, //Ch0
    tim8_ch1, tim8_ch2, tim8_ch3,                       //---
    dcmi, adc2, spi6_rx, spi6_tx,                       //Ch1
    adc3, spi5_rx, spi5_tx, cryp_out, cryp_in, hash_in, //Ch2
    spi1_rx, spi1_tx,                                   //Ch3
    spi4_rx, spi4_tx, usart1_rx, usart1_tx, sdio,       //Ch4
    usart6_rx, usart6_tx,                               //Ch5
    tim1_trig,  tim1_up,                                //Ch6
    tim8_up, tim8_com, tim8_trig,                       //Ch7
	_devid_end
	};

	namespace detail {
		//! Standard mapping
		template <typename... T> constexpr auto _chb(T... args) {
			return ( ... | (1U<<args ) );
		}
		// For version2 yet another mapping
		struct chn_ext {
			template<
				typename... T,
				typename E = std::enable_if_t<(std::is_same_v<T,int> && ...)>
			>
			constexpr chn_ext( unsigned char _dma, unsigned char _chn, T... strms)
				: dma(_dma),chn(_chn), strm( detail::_chb(strms...))
			{
			}
			constexpr chn_ext()
				: dma(0x03),chn(0x3f),strm(0xff)
			{}
			const unsigned char dma : 2;
			const unsigned char chn : 6;
			const unsigned char strm;
		};

	/** DMA channel mapping to the device assignment
	  { dma=1,2; channel=0-7 stream_no1=0-7, stream_no2=0-7 ... }
	 */
		static constexpr chn_ext dev_chn_map [[maybe_unused]] [] = {
			{},							//MEM
			{1,1,1,2,3}
		};
	}
}

