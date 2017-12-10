/*
 * =====================================================================================
 *
 *       Filename:  i2c_dma_helper.hpp
 *
 *    Description:  I2C Dma helper
 *
 *        Version:  1.0
 *        Created:  20.05.2017 18:29:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <stm32dma.h>
#include <stm32system.h>

namespace stm32 {
namespace drv {
namespace {

/** DMA Function depends on device STM32F1 and F2/F4 have incompatibile DMA controller */
#ifdef STM32MCU_MAJOR_TYPE_F1
	//! RXDMA channel config
	constexpr DMA_Channel_TypeDef* i2c2rxdma( I2C_TypeDef * const i2c )
	{
		return i2c==I2C1?(DMA1_Channel7):(DMA1_Channel5);
	}
	//! TXDMA coannel config
	constexpr DMA_Channel_TypeDef* i2c2txdma( I2C_TypeDef * const i2c )
	{
		return i2c==I2C1?(DMA1_Channel6):(DMA1_Channel4);
	}
	//i2c TX DMA config
	inline void i2c_dma_tx_config( I2C_TypeDef * const i2c, const void *buf, unsigned short len )
	{
		using namespace stm32;
		dma_channel_config( i2c2txdma(i2c), DMA_DIR_PeripheralDST|DMA_PeripheralInc_Disable|
			DMA_MemoryInc_Enable|DMA_PeripheralDataSize_Byte|DMA_MemoryDataSize_Byte|DMA_Mode_Normal|
			DMA_Priority_Medium|DMA_M2M_Disable, const_cast<void*>(buf), &i2c->DR, len );
	}
	//i2c RX DMA config
	inline void i2c_dma_rx_config( I2C_TypeDef * const i2c, void *buf, unsigned short len )
	{
		using namespace stm32;
		dma_channel_config( i2c2rxdma(i2c), DMA_DIR_PeripheralSRC|DMA_PeripheralInc_Disable|
			DMA_MemoryInc_Enable|DMA_PeripheralDataSize_Byte|DMA_MemoryDataSize_Byte|DMA_Mode_Normal|
			DMA_Priority_Medium|DMA_M2M_Disable, const_cast<void*>(buf), &i2c->DR, len );
	}
	//DMA TX enable
	inline void i2c_dma_tx_enable( I2C_TypeDef * const i2c )
	{
		stm32::dma_channel_enable( i2c2txdma(i2c) );
	}
	//DMA RX enable
	inline void i2c_dma_rx_enable( I2C_TypeDef * const i2c )
	{
		stm32::dma_irq_enable( i2c2rxdma(i2c), DMA_IT_TC );
		stm32::dma_channel_enable( i2c2rxdma(i2c) );
	}
	//DMA TX enable
	inline void i2c_dma_tx_disable( I2C_TypeDef * const i2c )
	{
		stm32::dma_channel_disable( i2c2txdma(i2c) );
	}
	//DMA RX enable
	inline void i2c_dma_rx_disable( I2C_TypeDef * const i2c )
	{
		stm32::dma_irq_disable( i2c2rxdma(i2c), DMA_IT_TC );
		stm32::dma_channel_disable( i2c2rxdma(i2c) );
	}

	//I2C dma NVIC On
	inline void i2c_dma_irq_on( I2C_TypeDef * const i2c , int prio, int sub )
	{
		if( i2c == I2C1 ) {
			stm32::nvic_set_priority( DMA1_Channel7_IRQn, prio, sub );
			stm32::nvic_irq_enable( DMA1_Channel7_IRQn, true );
		} else if( i2c == I2C2 ) {
			stm32::nvic_set_priority( DMA1_Channel5_IRQn, prio, sub );
			stm32::nvic_irq_enable( DMA1_Channel5_IRQn, true );
		}
	}
#else /* !STM32MCU_MAJOR_TYPE_F1 */

	enum class dma_dir : bool {
		rx,
		tx
	};
	inline DMA_Stream_TypeDef* _i2c_stream( I2C_TypeDef* i2c, dma_dir d ) {
		if( i2c == I2C1 ) {
			if( d == dma_dir::rx ) {
				return DMA1_Stream0;
			} else {
				return DMA1_Stream6;
			}
		} else if( i2c == I2C2 ) {
			if( d == dma_dir::rx ) {
				return DMA1_Stream2;
			} else {
				return  DMA1_Stream7;
			}
		}
		return nullptr;
	}
	inline unsigned _i2c_chn( I2C_TypeDef* i2c ) {
		if( i2c == I2C1 ) {
			return (DMA_Channel_1);
		} else if( i2c == I2C2 ) {
			return (DMA_Channel_7);
		}
		return 0;
	}
	// DMA tx config
	inline void i2c_dma_tx_config( I2C_TypeDef * const i2c, const void *buf, unsigned short len )
	{
		using namespace stm32;
		dma_deinit( _i2c_stream(i2c,dma_dir::tx) );
		dma_init( _i2c_stream(i2c,dma_dir::tx), _i2c_chn(i2c)| DMA_DIR_MemoryToPeripheral |
				  DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
				  DMA_PeripheralDataSize_Byte | DMA_Mode_Normal | DMA_Priority_High |
				  DMA_MemoryBurst_Single | DMA_PeripheralBurst_Single | DMA_MemoryDataSize_Byte,
				  DMA_FIFOMode_Disable | DMA_FIFOThreshold_HalfFull,
				  len, &i2c->DR, const_cast<volatile void*>(buf) );
	}
	//DMA rx config
	inline void i2c_dma_rx_config( I2C_TypeDef * const i2c, void *buf, unsigned short len )
	{
		using namespace stm32;
		dma_deinit( _i2c_stream(i2c,dma_dir::rx) );
		dma_init( _i2c_stream(i2c,dma_dir::rx), _i2c_chn(i2c)| DMA_DIR_PeripheralToMemory |
				  DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
				  DMA_PeripheralDataSize_Byte | DMA_Mode_Normal | DMA_Priority_High |
				  DMA_MemoryBurst_Single | DMA_PeripheralBurst_Single | DMA_MemoryDataSize_Byte,
				  DMA_FIFOMode_Disable | DMA_FIFOThreshold_HalfFull,
				  len, &i2c->DR, const_cast<volatile void*>(buf) );
	}
	//DMA tx enable
	inline void i2c_dma_tx_enable( I2C_TypeDef * const i2c )
	{
		stm32::dma_cmd( _i2c_stream(i2c,dma_dir::tx), true );
	}
	//DMA rx enable
	inline void i2c_dma_rx_enable( I2C_TypeDef * const i2c )
	{
		stm32::dma_it_config( _i2c_stream(i2c,dma_dir::rx), DMA_IT_TC, true );
		stm32::dma_cmd( _i2c_stream(i2c,dma_dir::rx), true );
	}
	//DMA tx disable
	inline void i2c_dma_tx_disable( I2C_TypeDef * const i2c )
	{
		stm32::dma_cmd( _i2c_stream(i2c,dma_dir::tx), false );
	}
	//DMA rx disable
	inline void i2c_dma_rx_disable( I2C_TypeDef * const i2c )
	{
		stm32::dma_it_config( _i2c_stream(i2c,dma_dir::rx), DMA_IT_TC, false );
		stm32::dma_cmd( _i2c_stream(i2c,dma_dir::rx), false );
	}
	//DMA nvic on
	inline void i2c_dma_irq_on( I2C_TypeDef * const i2c, int prio, int sub ) {
		if( i2c == I2C1 ) {
			stm32::nvic_set_priority( DMA1_Stream0_IRQn, prio, sub );
			stm32::nvic_irq_enable( DMA1_Stream0_IRQn, true );
		} else if( i2c == I2C2 ) {
			stm32::nvic_set_priority( DMA1_Stream2_IRQn, prio, sub );
			stm32::nvic_irq_enable( DMA1_Stream2_IRQn, true );
		}
	}
#endif
}}}
