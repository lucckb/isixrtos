/*
 * =====================================================================================
 *
 *       Filename:  i2c_bus.cpp
 *
 *    Description:  I2C bus isix specific implementation with DMA support
 *
 *        Version:  1.0
 *        Created:  03.03.2014 18:06:20
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
//TODO: Code should be revieved when eeprom is complete 
//TODO: Check for STM32F1 platform
#include <isixdrv/i2c_bus.hpp>
#include <stm32system.h>
#include <stm32i2c.h>
#include <stm32rcc.h>
#include <stm32dma.h>
#include <stm32gpio.h>
#include <cstdlib>
#include <new>
#ifdef _HAVE_CONFIG_H
#include <config.h>
#endif
/* ------------------------------------------------------------------ */ 
#ifdef CONFIG_ISIXDRV_I2C_DEBUG 
#include <foundation/dbglog.h>
#else
#define dbprintf(...) do {} while(0)
#endif
/* ------------------------------------------------------------------ */ 
namespace stm32 {
namespace drv {
/* ------------------------------------------------------------------ */
#ifdef CONFIG_ISIXDRV_I2C_USE_FIXED_I2C
namespace {
#if CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1
	static constexpr void* m_i2c = I2C1;
#elif CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2
	static constexpr void* m_i2c = I2C2;
#endif
}
#endif
/* ------------------------------------------------------------------ */ 
//! Unnnamed namespace for internal functions
namespace {
	//Cast to device type
	inline I2C_TypeDef* dcast( void* p ) {
		return reinterpret_cast<I2C_TypeDef*>(p);
	}
	//Cast to i2c type
	inline void* to_i2c( i2c_bus::busid id ) {
		switch( id ) {
			case i2c_bus::busid::i2c1: return I2C1;
			case i2c_bus::busid::i2c2: return I2C2;
			default: return nullptr;
		}
	}
	inline void terminate() {
#ifdef __EXCEPTIONS
		throw std::bad_alloc();
#else
		std::abort();
#endif
	}
	static constexpr auto I2C1_PORT = GPIOB;
	static constexpr auto I2C1_SDA_PIN_ = 7;
	static constexpr auto I2C1_SCL_PIN_ = 6;
	static constexpr auto I2C1_SDA_PIN = 1U<<I2C1_SDA_PIN_;
	static constexpr auto I2C1_SCL_PIN = 1U<<I2C1_SCL_PIN_;
	static constexpr auto I2C2_PORT = GPIOB;
	static constexpr auto I2C2_SDA_PIN_ = 11;
	static constexpr auto I2C2_SCL_PIN_ = 10;
	static constexpr auto I2C2_SDA_PIN = 1U<<I2C2_SDA_PIN_;
	static constexpr auto I2C2_SCL_PIN = 1U<<I2C2_SCL_PIN_;
	static constexpr auto I2C1_PINS = I2C1_SDA_PIN|I2C1_SCL_PIN;
	static constexpr auto I2C2_PINS = I2C2_SDA_PIN|I2C2_SCL_PIN;
}
/** DMA Function depends on device STM32F1 and F2/F4 have incompatibile DMA controller */
#ifdef STM32MCU_MAJOR_TYPE_F1
namespace {
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
	//AFIO optional config
	inline void afio_config( I2C_TypeDef * const  ) {}
	
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
}
#else
namespace {
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
		dma_it_config( _i2c_stream(i2c,dma_dir::rx), DMA_IT_TC, true );
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
		dma_it_config( _i2c_stream(i2c,dma_dir::rx), DMA_IT_TC, false );
		stm32::dma_cmd( _i2c_stream(i2c,dma_dir::rx), false );
	}
	//AFIO optional config
	inline void afio_config( I2C_TypeDef * const i2c )
	{
		if( i2c == I2C1 ) {
			gpio_pin_AF_config( I2C1_PORT, I2C1_SDA_PIN_, GPIO_AF_I2C1 );
			gpio_pin_AF_config( I2C1_PORT, I2C1_SCL_PIN_, GPIO_AF_I2C1 );
		} else if( i2c == I2C2 ) {
			gpio_pin_AF_config( I2C2_PORT, I2C2_SDA_PIN_, GPIO_AF_I2C2 );
			gpio_pin_AF_config( I2C2_PORT, I2C2_SCL_PIN_, GPIO_AF_I2C2 );

		}
	}
	//DMA nvic on
	inline void i2c_dma_irq_on( I2C_TypeDef * const i2c, int prio, int sub ) {
		if( i2c == I2C1 ) {
			nvic_set_priority( DMA1_Stream0_IRQn, prio, sub );
			nvic_irq_enable( DMA1_Stream0_IRQn, true );
		} else if( i2c == I2C2 ) {
			nvic_set_priority( DMA1_Stream7_IRQn, prio, sub );
			nvic_irq_enable( DMA1_Stream7_IRQn, true );
		}
	}
}
#endif
//! Objects for interrupt handlers
namespace {
#ifndef CONFIG_ISIXDRV_I2C_USE_FIXED_I2C
	i2c_bus* obj_i2c1;
	i2c_bus* obj_i2c2;
#else
#if CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1
	i2c_bus* obj_i2c1;
	constexpr i2c_bus* obj_i2c2 = nullptr;
#elif CONFIG_ISIXDRV_I2C_USE_FIXED_I2C ==CONFIG_ISIXDRV_I2C_2
	i2c_bus* obj_i2c2;
	constexpr i2c_bus* obj_i2c1 = nullptr;
#endif
#endif
}
/* ------------------------------------------------------------------ */ 
/** Constructor
	* @param[in] _i2c Interface bus ID
	* @param[in] clk_speed CLK speed in HZ
	*/
i2c_bus::i2c_bus( busid _i2c, unsigned clk_speed, unsigned pclk1 )
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C)
	: m_i2c( to_i2c(_i2c) )
#endif
{
#if defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C) 
	static_cast<void>(_i2c);
#endif
	using namespace stm32;
	if( m_i2c == I2C1 ) {
		rcc_apb1_periph_clock_cmd( RCC_APB1Periph_I2C1, true );
		gpio_clock_enable( I2C1_PORT, true );
		if( obj_i2c1 ) {
			terminate();
		}
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C) || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1)
		obj_i2c1 = this;
#endif
	} else if ( m_i2c == I2C2 ) {
		rcc_apb1_periph_clock_cmd( RCC_APB1Periph_I2C2, true );
		gpio_clock_enable( I2C2_PORT, true );
		if(obj_i2c2) {
			terminate();
		}
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C) || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2)
		obj_i2c2 = this;
#endif
	}
	afio_config( dcast(m_i2c) );
	if( m_i2c == I2C1 ) {
		gpio_abstract_config_ext( I2C1_PORT, I2C1_PINS, AGPIO_MODE_ALTERNATE_OD_PULLUP, AGPIO_SPEED_HALF );
	} else if( m_i2c == I2C2 ) {
		gpio_abstract_config_ext( I2C2_PORT, I2C2_PINS, AGPIO_MODE_ALTERNATE_OD_PULLUP, AGPIO_SPEED_HALF );
	} else {
		//! Not supported yet
		terminate();
	}
	i2c_init( dcast(m_i2c), clk_speed, I2C_Mode_I2C, I2C_DutyCycle_2, 1, 
			  I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit, pclk1 );
	i2c_acknowledge_config( dcast(m_i2c), true );
	if( m_i2c == I2C1 ) {
		nvic_set_priority( I2C1_EV_IRQn, IRQ_PRIO, IRQ_SUB );
		nvic_set_priority( I2C1_ER_IRQn, IRQ_PRIO, IRQ_SUB );
		nvic_irq_enable( I2C1_ER_IRQn, true );
		nvic_irq_enable( I2C1_EV_IRQn, true );
	} else if( m_i2c == I2C2 ) {
		nvic_set_priority( I2C2_ER_IRQn, IRQ_PRIO, IRQ_SUB );
		nvic_irq_enable( I2C2_ER_IRQn, true );
		nvic_set_priority( I2C2_EV_IRQn, IRQ_PRIO, IRQ_SUB );
		nvic_irq_enable( I2C2_EV_IRQn, true );
	}
	i2c_dma_irq_on( dcast(m_i2c), IRQ_PRIO, IRQ_SUB );
	//Enable DMA in i2c
	i2c_dma_cmd( dcast(m_i2c), true );
	i2c_cmd( dcast(m_i2c), true );
}
/* ------------------------------------------------------------------ */ 
/** Destructor */
i2c_bus::~i2c_bus()
{
	if( dcast(m_i2c) == I2C1 ) {
		nvic_irq_enable( I2C1_EV_IRQn, false );
		nvic_irq_enable( I2C1_ER_IRQn, false );
	} else if( dcast(m_i2c) == I2C2 ) {
		nvic_irq_enable( I2C2_EV_IRQn, false );
		nvic_irq_enable( I2C2_ER_IRQn, false );
	}
	i2c_deinit( dcast(m_i2c) );
}
/* ------------------------------------------------------------------ */ 
/* Get hwerror */
int i2c_bus::get_hwerror(void) const
{
	static const int err_tbl[] =
	{
		err_bus,
		err_arbitration_lost,
		err_ack_failure,
		err_overrun,
		err_pec,
		err_unknown,
		err_bus_timeout,
		err_invstate
	};
	for(int i=0; i<8; i++) {
		if(m_err_flag & (1<<i)) {
			return err_tbl[i];
		}
	}
	return 0;
}
/* ------------------------------------------------------------------ */
/** Transfer one byte over i2c interface
* @param[in] addr I2C address
* @param[in] wbuffer Memory pointer for write
* @param[in] wsize  Size of write buffer
* @param[out] rbuffer Read data buffer pointer
* @param[in] rsize Read buffer sizes
* @return Error code or success */
int i2c_bus::transfer(unsigned addr, const void* wbuffer, size_t wsize, void* rbuffer, size_t rsize)
{
	using namespace stm32;
	if( (addr>0xFF) || (addr&1) ) {
		return err_invaddr;
	}
	int ret;
	if( (ret=m_lock.wait(isix::ISIX_TIME_INFINITE))<0 ) {
		return ret;
	}
	//Disable I2C irq
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR, false );
	if( wbuffer )
		i2c_dma_tx_config( dcast(m_i2c), wbuffer, wsize );
	if( rsize > 1)
		i2c_dma_rx_config( dcast(m_i2c), rbuffer, rsize );
	if(wbuffer)
		m_addr = addr & ~(I2C_OAR1_ADD0);
	else if(rbuffer)
		m_addr = addr | I2C_OAR1_ADD0;
	m_rx_len = (rbuffer!=nullptr)?(rsize):(0);
	m_rx_buf = reinterpret_cast<uint8_t*>(rbuffer);
	//Clear status flags
	i2c_get_last_event( dcast(m_i2c));
	//Enable I2C irq
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, true );
	//DMA last transfer
	i2c_dma_last_transfer_cmd( dcast(m_i2c), false );
	i2c_acknowledge_config( dcast(m_i2c), true );
	//Send the start
	i2c_generate_start(dcast(m_i2c), true );
	ret = m_notify.wait( TRANSACTION_TIMEOUT );
	if( ret != isix::ISIX_EOK ) {
		i2c_it_config(dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, false );
		i2c_generate_stop(dcast(m_i2c), true );
		m_lock.signal();
		return err_timeout;
	}
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, false );
	if( (ret=get_hwerror())  )
	{
		i2c_it_config(dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, false );
		m_err_flag = 0;
		m_lock.signal();
		return ret;
	}
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, false );
	m_lock.signal();
	return ret;
}
/* ------------------------------------------------------------------ */

/** Double non continous transaction write 
	* @param[in] addr I2C address
	* @param[in] wbuf1 Write buffer first transaction
	* @param[in] wsize1 Transaction size 1
	* @param[in] wbuf2 Write buffer first transaction
	* @param[in] wsize2 Transaction size 1
	* @return error code or success */
int i2c_bus::write( unsigned addr, const void* wbuf1, size_t wsize1, const void* wbuf2, size_t wsize2 )
{
	m_tx2_len = (wbuf2!=nullptr)?(wsize2):(0);
	m_tx2_buf = reinterpret_cast<const uint8_t*>(wbuf2);
	return transfer( addr, wbuf1, wsize1, nullptr, 0 );
}
/* ------------------------------------------------------------------ */ 
void i2c_bus::ev_irq()
{
	using namespace stm32;
	auto event = i2c_get_last_event(dcast(m_i2c));
	switch( event )
	{
	//Send address
	case I2C_EVENT_MASTER_MODE_SELECT:		//EV5
		i2c_send_f7bit_address(dcast(m_i2c), m_addr );
		dbprintf("I2C_EVENT_MASTER_MODE_SELECT");
	break;

	//Send bytes in tx mode
	case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:	//EV6
			i2c_dma_tx_enable( dcast(m_i2c) );
			dbprintf("I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED");
	break;

	case I2C_EVENT_MASTER_BYTE_TRANSMITTED:	//EV8
	{
		i2c_dma_tx_disable( dcast(m_i2c) );
		if( m_rx_len ) {
			//Change address to read
			m_addr |= I2C_OAR1_ADD0;
			i2c_generate_start( dcast(m_i2c), true );
			//ACK config
			dbprintf("I2C_EVENT_MASTER_BYTE_TRANSMITTEDtoRX");
		}
		else {
			if( m_tx2_len == 0 ) {
				ev_finalize();
			} else {
				i2c_dma_tx_config( dcast(m_i2c), m_tx2_buf, m_tx2_len );
				i2c_dma_tx_enable( dcast(m_i2c) );
				m_tx2_len = 0;
				m_tx2_buf = nullptr;
			}
			dbprintf("I2C_EVENT_MASTER_BYTE_TRANSMITTEDAfterTX");
		}
		//Data synch barrier
		dsb(); isb(); nop(); nop();
	}
	break;

	//Master mode selected
	case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:	//EV7
			if( m_rx_len > 1 ) {
				i2c_dma_last_transfer_cmd( dcast(m_i2c), true );
				i2c_dma_rx_enable( dcast(m_i2c) );
			} else {
				i2c_acknowledge_config( dcast(m_i2c), false );
				i2c_it_config(dcast(m_i2c), I2C_IT_BUF, true );
			}
		dbprintf("I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED [%i]", m_rx_len );
	break;

	case I2C_EVENT_MASTER_BYTE_RECEIVED:
		m_rx_buf[0] = i2c_receive_data( dcast(m_i2c) );
		ev_finalize();
		dbprintf("I2C_EVENT_MASTER_BYTE_RECEIVED ");
	break;
		
	default:
		dbprintf("Unknown event %08x", event );
		ev_finalize( true );
		break;
	}
}
/* ------------------------------------------------------------------ */
//! Finalize transaction
void i2c_bus::ev_finalize( bool inv_state )
{
	i2c_generate_stop(dcast(m_i2c),true);
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR|I2C_IT_BUF, false );
	i2c_dma_rx_disable( dcast(m_i2c) );
	i2c_dma_last_transfer_cmd( dcast(m_i2c), false );
	i2c_acknowledge_config( dcast(m_i2c), true );
	if( inv_state ) {
		static constexpr auto inv_state_bit = 0x80;
		m_err_flag |= inv_state_bit;
	}
	//ACK config
	m_notify.signal_isr();
}
/* ------------------------------------------------------------------ */
//Error event handler
void i2c_bus::err_irq()
{
	static constexpr auto EVENT_ERROR_MASK = 0xff00;
	using namespace stm32;
	//SR0 only don't care about SR1
	uint16_t event = i2c_get_last_event( dcast(m_i2c) );
	if(event & EVENT_ERROR_MASK)
	{
		i2c_clear_it_pending_bit( dcast(m_i2c), EVENT_ERROR_MASK );
		m_err_flag = event >> 8;
		i2c_it_config( dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, false );
		i2c_generate_stop( dcast(m_i2c),true );
		m_notify.signal_isr();
		i2c_dma_rx_disable( dcast(m_i2c) );
		i2c_dma_tx_disable( dcast(m_i2c) );
	}
}
/* ------------------------------------------------------------------ */ 
//Dma trasfer complete
void i2c_bus::ev_dma_tc() 
{
	i2c_generate_stop(dcast(m_i2c),true);
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR, false );
	i2c_dma_rx_disable( dcast(m_i2c) );
	i2c_dma_last_transfer_cmd( dcast(m_i2c), false );
	//ACK config
	m_notify.signal_isr();
}
/* ------------------------------------------------------------------ */
void i2c_bus::mdelay( unsigned timeout ) 
{
	isix::isix_wait_ms( timeout );
}
/* ------------------------------------------------------------------ */ 
extern "C" {
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C) || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1)
	/* Irq ev handler */
	void __attribute__ ((interrupt)) i2c1_ev_isr_vector()
	{
		if( obj_i2c1 ) obj_i2c1->ev_irq();
	}
	/* Irq err handler */
	void __attribute__ ((interrupt)) i2c1_er_isr_vector()
	{
		if( obj_i2c1 ) obj_i2c1->err_irq();
	}
#endif
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C) || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2)
	/* Irq ev handler */
	void __attribute__ ((interrupt)) i2c2_ev_isr_vector()
	{
		if( obj_i2c2 ) obj_i2c2->ev_irq();
	}
	/* Irq err handler */
	void __attribute__ ((interrupt)) i2c2_er_isr_vector()
	{
		if( obj_i2c2 ) obj_i2c2->err_irq();
	}
#endif
#ifndef STM32MCU_MAJOR_TYPE_F1
	//I2C1 RX
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C) || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1)
	__attribute__((interrupt)) void dma1_stream0_isr_vector()
	{
		dma_clear_flag( DMA1_Stream0, DMA_FLAG_TCIF0|DMA_FLAG_TEIF0 );
		if( obj_i2c1 ) obj_i2c1->ev_dma_tc();
	}
#endif
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C) || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2)
	//I2C2DMARX
	__attribute__((interrupt)) void dma1_stream7_isr_vector()
	{
		dma_clear_flag( DMA1_Stream7, DMA_FLAG_TCIF7|DMA_FLAG_TEIF7 );
		if( obj_i2c2 ) obj_i2c2->ev_dma_tc();
	}
#endif
#else
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C) || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1)
	//I2C1 RX
	__attribute__((interrupt)) void dma1_channel7_isr_vector() 
	{
		stm32::dma_clear_flag( DMA1_FLAG_TC7 );	
		if( obj_i2c1 ) obj_i2c1->ev_dma_tc();
	}
#endif
#if !defined(CONFIG_ISIXDRV_I2C_USE_FIXED_I2C) || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2)
	//I2C2 RX
	__attribute__((interrupt)) void dma1_channel5_isr_vector() 
	{
		stm32::dma_clear_flag( DMA1_FLAG_TC5 );
		if( obj_i2c2 ) obj_i2c2->ev_dma_tc();
	}
#endif
#endif
}
/* ------------------------------------------------------------------ */ 
}
}
/* ------------------------------------------------------------------ */ 
