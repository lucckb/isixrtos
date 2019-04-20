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

//! FIXME: Setup proper dma channel config not working yet not merged yet

namespace periph::dma::devid {

	enum _devid : chnid_t {
	mem,																	 //:D1
    spi3_rx,     spi3_tx,       spi2_rx,    spi2_tx,                         //Ch0
    i2c1_rx,     i2c1_tx,       tim7_up,                                     //Ch1
    tim4_ch1,    tim4_ch2,      tim4_ch3,   tim4_up,                         //Ch2
    i2s3_ext_rx, i2s3_ext_tx,   i2s2_ext_tx,                                 //---
    i2s2_ext_rx, tim2_up,       tim2_ch4,   tim2_ch3,                        //Ch3
    tim2_ch2,    tim2_ch1,      i2c3_rx,    i2c3_tx,                         //---
    uart5_rx,    uart5_tx,      uart4_rx,   uart4_tx,                        //Ch4
    usart3_rx,   usart3_tx,     usart2_rx,  usart2_tx,                       //---
    uart8_rx,    uart8_tx,      uart7_rx,   uart7_tx,                        //Ch5
    tim3_ch4,    tim3_ch3,      tim3_ch2,   tim3_ch1, tim3_up,               //---
    tim5_ch4,    tim5_ch3,      tim5_ch2,   tim5_ch1,                        //Ch6
    tim5_up,     tim5_trig,                                                  //---
    tim6_up,     i2c2_rx,       i2c2_tx,    dac1,     dac2,                  //Ch7
                                                                             //:D2
    adc1,        sai1_a,        sai1_b,     tim1_ch1, tim1_ch2, tim1_ch3,    //Ch0
    tim8_ch1,    tim8_ch2,      tim8_ch3,                                    //---
    dcmi,        adc2,          spi6_rx,    spi6_tx,                         //Ch1
    adc3,        spi5_rx,       spi5_tx,    cryp_out, cryp_in, hash_in,      //Ch2
    spi1_rx,     spi1_tx,                                                    //Ch3
    spi4_rx,     spi4_tx,       usart1_rx,  usart1_tx, sdio,                 //Ch4
    usart6_rx,   usart6_tx,                                                  //Ch5
    tim1_trig,   tim1_up,                                                    //Ch6
    tim8_up,     tim8_com,      tim8_trig,                                   //Ch7
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
				: dma(_dma),chn(_chn), strm(detail::_chb(strms...))
			{
			}
			constexpr chn_ext()
				: dma(0),chn(0),strm(0)
			{}
			unsigned chn_msk() const {
				return strm << (dma==2?8:0);
			}
			const unsigned char dma : 2;
			const unsigned char chn : 6;
			const unsigned char strm;
		};

	/** DMA channel mapping to the device assignment
	  { dma=1,2; channel=0-7 stream_no1=0-7, stream_no2=0-7 ... }
	 */
		static constexpr chn_ext dev_chn_map [[maybe_unused]] [] = {
				{2,0,0,1,2,3,4,5,6,7},		//MEM DMA2 only is supported
				{1,0,0,2},                  //spi3_rx     Ch0
				{1,0,5,7},                  //spi3_tx     Ch0
				{1,0,3},                    //spi2_rx     Ch0
				{1,0,4},                    //spi2_tx     Ch0
				{1,1,0,6},                  //i2c1_rx     Ch1
				{1,1,6,7},                  //i2c1_tx     Ch1
				{1,1,2,4},                  //tim7_up     Ch1
				{1,2,0},                    //tim4_ch1    Ch2
				{1,2,3},                    //tim4_ch2    Ch2
				{1,2,7},                    //tim4_ch3    Ch2
				{1,2,6},                    //tim8_up     Ch2
				{1,2,2},                    //i2s3_ext_rx Ch2
				{1,2,5},                    //i2s3_ext_tx Ch2
				{1,2,4},                    //i2s2_ext_tx Ch2
				{1,3,3},                    //i2s2_ext_rx Ch3
				{1,3,2},                    //tim2_up     Ch3
				{1,3,7},                    //tim2_ch4    Ch3
				{1,3,1},                    //tim2_ch3    Ch3
				{1,3,6},                    //tim2_ch2    Ch3
				{1,3,5},                    //tim2_ch1    Ch3
				{1,3,2},                    //i2c3_rx     Ch3
				{1,3,4},                    //i2c3_tx     Ch3
				{1,4,0},                    //uart5_rx    Ch4
				{1,4,7},                    //uart5_tx    Ch4
				{1,4,2},                    //uart4_rx    Ch4
				{1,4,4},                    //uart4_tx    Ch4
				{1,4,1},                    //usart3_rx   Ch4
				{1,4,3},                    //usart3_tx   Ch4
				{1,4,5},                    //usart2_rx   Ch4
				{1,4,6},                    //usart2_tx   Ch4
				{1,5,6},                    //uart8_rx    Ch5
				{1,5,0},                    //uart8_tx    Ch5
				{1,5,3},                    //uart7_rx    Ch5
				{1,5,1},                    //uart7_tx    Ch5
				{1,5,2},                    //tim3_ch4    Ch5
				{1,5,7},                    //tim3_ch3    Ch5
				{1,5,5},                    //tim3_ch2    Ch5
				{1,5,4},                    //tim3_ch1    Ch5
				{1,5,2},                    //tim3_up     Ch5
				{1,6,1},                    //tim5_ch4    Ch6
				{1,6,0},                    //tim5_ch3    Ch6
				{1,6,4},                    //tim5_ch2    Ch6
				{1,6,2},                    //tim5_ch1    Ch6
				{1,6,0,6},                  //tim5_up     Ch6
				{1,6,1,3},                  //tim5_trig   Ch6
				{1,7,1},                    //tim6_up     Ch7
				{1,7,2,3},                  //i2c2_rx     Ch7
				{1,7,7},                    //i2c2_tx     Ch7
				{1,7,5},                    //dac1        Ch7
				{1,7,6},                    //dac2        Ch7

				{2,0,0,4},                  //adc1        Ch0
				{2,0,1,3},                  //sai1_a      Ch0
				{2,0,5},                    //sai1_b      Ch0
				{2,0,6},                    //tim1_ch1    Ch0
				{2,0,6},                    //tim1_ch2    Ch0
				{2,0,6},                    //tim1_ch3    Ch0
				{2,0,2},                    //tim8_ch1    Ch0
				{2,0,2},                    //tim8_ch2    Ch0
				{2,0,2},                    //tim8_ch3    Ch0
				{2,1,1,7},                  //dcmi        Ch1
				{2,1,2,3},                  //adc2        Ch1
				{2,1,6},                    //spi6_rx     Ch1
				{2,1,5},                    //spi6_tx     Ch1
				{2,2,0,1},                  //adc3        Ch2
				{2,2,3},                    //spi5_rx     Ch2
				{2,2,4},                    //spi5_tx     Ch2
				{2,2,5},                    //cryp_out    Ch2
				{2,2,6},                    //cryp_in     Ch2
				{2,2,7},                    //hash_in     Ch2
				{2,3,0,2},                  //spi1_rx     Ch3
				{2,3,3,5},                  //spi1_tx     Ch3
				{2,4,0},                    //spi4_rx     Ch4
				{2,4,1},                    //spi4_tx     Ch4
				{2,4,2,5},                  //usart1_rx   Ch4
				{2,4,7},                    //usart1_tx   Ch4
				{2,4,3,6},                  //sdio        Ch4
				{2,5,1,2},                  //usart6_rx   Ch5
				{2,5,6,7},                  //usart6_tx   Ch5
				{2,6,0,4},                  //tim1_trig   Ch6
				{2,6,5},                    //tim1_up     Ch6
				{2,7,1},                    //tim8_up     Ch7
				{2,7,7},                    //tim8_com    Ch7
				{2,7,7},                    //tim8_trig   Ch7

		};
	}
}

