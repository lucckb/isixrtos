/*
 * =====================================================================================
 *
 *       Filename:  i2c_bus.cpp
 *
 *    Description:  I2C bus isix specific implementation with DMA support
 *	  Some STM32F1 devices has a lot of I2C bug. Especially you need to read and write
 *	  minimum 2 bytes
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
#include <isixdrv/i2c_bus.hpp>
#include <foundation/sys/dbglog.h>
#include <stm32system.h>
#include <stm32i2c.h>
#include <stm32rcc.h>
#include <stm32gpio.h>
#include <cstdlib>
#include <new>
#include <cstring>
#include <config/conf.h>
#if !CONFIG_ISIXDRV_I2C_NODMA
#include "i2c_dma_helper.hpp"
#endif
#include "i2c_gpio_helper.hpp"

namespace stm32 {
namespace drv {

#if CONFIG_ISIXDRV_I2C_USE_FIXED_I2C && (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C!=0)
namespace {
#if CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1
	static void* const m_i2c = I2C1;
#elif CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2
	static void* const m_i2c = I2C2;
#endif
}
#endif

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
			case i2c_bus::busid::i2c1_alt: return I2C1;
			case i2c_bus::busid::i2c2: return I2C2;
			case i2c_bus::busid::i2c2_alt: return I2C2;
			default: return nullptr;
		}
	}
	//Is GPIO alternate mode
	inline auto is_alt_i2c( i2c_bus::busid id ) {
		return id==i2c_bus::busid::i2c1_alt ||
			   id==i2c_bus::busid::i2c2_alt;
	}

	inline void terminate() {
#ifdef __EXCEPTIONS
		throw std::bad_alloc();
#else
		std::abort();
#endif
	}
}
//! Objects for interrupt handlers
namespace {
#if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C
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

/** Constructor
	* @param[in] _i2c Interface bus ID
	* @param[in] clk_speed CLK speed in HZ
	*/
i2c_bus::i2c_bus( busid _i2c, unsigned clk_speed, unsigned pclk1 )
  :  ibus(fnd::drv::bus::ibus::type::spi),
#if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C
	m_i2c(to_i2c(_i2c))
#endif
{
#if CONFIG_ISIXDRV_I2C_USE_FIXED_I2C
	static_cast<void>(_i2c);
#endif
	using namespace stm32;
	if( m_i2c == I2C1 ) {
		rcc_apb1_periph_clock_cmd( RCC_APB1Periph_I2C1, true );
		gpio_clock_enable( I2C1_PORT, true );
		if( obj_i2c1 ) {
			terminate();
		}
#if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1)
		obj_i2c1 = this;
#endif
	} else if ( m_i2c == I2C2 ) {
		rcc_apb1_periph_clock_cmd( RCC_APB1Periph_I2C2, true );
		if( !is_alt_i2c(_i2c))  gpio_clock_enable( I2C2_PORT, true );
		else gpio_clock_enable( I2C2ALT_PORT, true );
		if(obj_i2c2) {
			terminate();
		}
#if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2)
		obj_i2c2 = this;
#endif
	}
	afio_config( dcast(m_i2c), is_alt_i2c(_i2c) );
	gpio_initialize( is_alt_i2c(_i2c) );
	//Some I2C ip cores raise errors during GPIO config so make the software reset
	i2c_software_reset_cmd( dcast(m_i2c), true ); dmb();
	i2c_software_reset_cmd( dcast(m_i2c), false ); dmb();
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
#if !CONFIG_ISIXDRV_I2C_NODMA
	i2c_dma_irq_on( dcast(m_i2c), IRQ_PRIO, IRQ_SUB );
	i2c_dma_cmd( dcast(m_i2c), true );
#endif
	i2c_cmd( dcast(m_i2c), true );
#ifdef STM32MCU_MAJOR_TYPE_F1
	//! STM32F1 errata PAGE 21
	//Some software events must be managed before the current byte is being transferred
	if( i2c_get_last_event(dcast(m_i2c)) & I2C_EVENT_SLAVE_STOP_DETECTED ) {
		i2c_software_reset_cmd( dcast(m_i2c), true );
		nop(); nop(); nop(); nop();
		i2c_software_reset_cmd( dcast(m_i2c), false );
	}
#endif
}

void i2c_bus::gpio_initialize( bool alt )
{
	if( m_i2c == I2C1 ) {
		if( !gpio_get( I2C1_PORT, alt?I2C1ALT_SDA_PIN_:I2C1_SDA_PIN_) ||
			!gpio_get( I2C1_PORT, alt?I2C1ALT_SCL_PIN_:I2C1_SCL_PIN_ ) ) {
			dbprintf("BUSY?");
		}
		gpio_abstract_config_ext( I2C1_PORT, alt?I2C1ALT_PINS:I2C1_PINS,
			AGPIO_MODE_ALTERNATE_OD_PULLUP, AGPIO_SPEED_LOW );
	} else if( m_i2c == I2C2 ) {
		if( !gpio_get( alt?I2C2ALT_PORT:I2C2_PORT, alt?I2C2ALT_SDA_PIN_:I2C2_SDA_PIN_) ||
				!gpio_get( alt?I2C2ALT_PORT:I2C2_PORT, alt?I2C2ALT_SCL_PIN_:I2C2_SCL_PIN_ ) ) {
			dbprintf("BUSY?");
		}
		gpio_abstract_config_ext( alt?I2C2ALT_PORT:I2C2_PORT, alt?I2C2ALT_PINS:I2C2_PINS,
			AGPIO_MODE_ALTERNATE_OD_PULLUP, AGPIO_SPEED_LOW );
	} else {
		//! Not supported yet
		terminate();
	}
}

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

/** Transfer one byte over i2c interface
* @param[in] addr I2C address
* @param[in] wbuffer Memory pointer for write
* @param[in] wsize  Size of write buffer
* @param[out] rbuffer Read data buffer pointer
* @param[in] rsize Read buffer sizes
* @return Error code or success */
int i2c_bus::transfer_impl(unsigned addr, const void* wbuffer,
		size_t wsize, void* rbuffer, size_t rsize)
{
	using namespace stm32;
	//dbprintf("i2c_bus::transfer(addr=%u wsize=%lu rsize=%lu)", addr, wsize, rsize );
	if( (addr>0xFF) || (addr&1) ) {
		return err_invaddr;
	}
	int ret;
	if( (ret=m_lock.wait(ISIX_TIME_INFINITE))<0 ) {
		return ret;
	}
	//Disable I2C irq
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR|I2C_IT_BUF, false );
#if !CONFIG_ISIXDRV_I2C_NODMA
	if( wbuffer )
		i2c_dma_tx_config( dcast(m_i2c), wbuffer, wsize );
	if( rsize > 1)
		i2c_dma_rx_config( dcast(m_i2c), rbuffer, rsize );
#else
		if( wbuffer ) {
			m_tx_buf = reinterpret_cast<const uint8_t*>(wbuffer);
			m_tx_len = wsize;
		} else {
			m_tx_buf = nullptr;
			m_tx_len = 0;
		}
		m_rx_cnt = m_tx_cnt = 0;
#endif
	if(wbuffer)
		m_addr = addr & ~(I2C_OAR1_ADD0);
	else if(rbuffer)
		m_addr = addr | I2C_OAR1_ADD0;
	m_rx_len = (rbuffer!=nullptr)?(rsize):(0);
	m_rx_buf = reinterpret_cast<uint8_t*>(rbuffer);
	//Clear status flags
	i2c_get_last_event( dcast(m_i2c) );
	//Enable I2C irq
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR, true );
#if !CONFIG_ISIXDRV_I2C_NODMA
	//DMA last transfer
	i2c_dma_last_transfer_cmd( dcast(m_i2c), false );
#endif
	i2c_acknowledge_config( dcast(m_i2c), true );
	i2c_generate_start(dcast(m_i2c), true );
	ret = m_notify.wait( TRANSACTION_TIMEOUT );
	if( ret != ISIX_EOK ) {
		i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR|I2C_IT_BUF, false );
		i2c_generate_stop(dcast(m_i2c), true );
		nop(); nop(); nop(); nop();
		m_lock.signal();
		return err_timeout;
	}
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR|I2C_IT_BUF, false );
	if( (ret=get_hwerror()) )
	{
		i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR|I2C_IT_BUF, false );
		m_err_flag = 0;
		m_lock.signal();
		return ret;
	}
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR|I2C_IT_BUF, false );
	m_lock.signal();
	return ret;
}


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
	m_tx2_buf = const_cast<const uint8_t * volatile>(reinterpret_cast<const uint8_t*>(wbuf2));
#if CONFIG_ISIXDRV_I2C_NODMA
	m_tx2_cnt = 0;
#endif
	return transfer( addr, wbuf1, wsize1, nullptr, 0 );
}

//! IRQ event handler
void i2c_bus::ev_irq()
{
	using namespace stm32;
	auto event = i2c_get_last_event(dcast(m_i2c));
	switch( event )
	{
	//Send address
	case I2C_EVENT_MASTER_MODE_SELECT:		//EV5
	{
		i2c_send_f7bit_address(dcast(m_i2c), m_addr );
	}
	break;
	//Send bytes in tx mode
	case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:	//EV6
	{
#if !CONFIG_ISIXDRV_I2C_NODMA
		i2c_dma_tx_enable( dcast(m_i2c) );
#else
		i2c_send_data( dcast(m_i2c), m_tx_buf[m_tx_cnt++] );
#endif
		//dbprintf("I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED");
	}
	break;

	case I2C_EVENT_MASTER_BYTE_TRANSMITTED:	//EV8
	{
#if !CONFIG_ISIXDRV_I2C_NODMA
		i2c_dma_tx_disable( dcast(m_i2c) );
		if( m_rx_len )
		{
			//Change address to read
			m_addr |= I2C_OAR1_ADD0;
			i2c_generate_start( dcast(m_i2c), true );
			//ACK config
			//dbprintf("I2C_EVENT_MASTER_BYTE_TRANSMITTEDtoRX");
		}
		else
		{
			if( m_tx2_len == 0 )
				ev_finalize();
			else
			{
				i2c_dma_tx_config( dcast(m_i2c), m_tx2_buf, m_tx2_len );
				i2c_dma_tx_enable( dcast(m_i2c) );
				m_tx2_len = 0;
				m_tx2_buf = nullptr;
			}
		}
#else
		if( m_tx_cnt < m_tx_len ) {
			i2c_send_data( dcast(m_i2c), m_tx_buf[m_tx_cnt++] );
		}
		if( m_tx_cnt >= m_tx_len ) {
			if( m_tx2_cnt < m_tx2_len ) {
				i2c_send_data( dcast(m_i2c), m_tx2_buf[m_tx2_cnt++] );
			}
			if( m_tx2_cnt >= m_tx2_len ) {
				if( m_rx_cnt < m_rx_len ) {
					m_addr |= I2C_OAR1_ADD0;
					i2c_acknowledge_config( dcast(m_i2c), true );
					i2c_generate_start( dcast(m_i2c), true );
				} else {
					ev_finalize();
				}
			}
		}
#endif
		//dbprintf("I2C_EVENT_MASTER_BYTE_TRANSMITTEDAfterTX");
		dsb(); isb(); nop(); nop();
	}
	break;

	//Master mode selected
	case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:	//EV7
#if !CONFIG_ISIXDRV_I2C_NODMA
		if( m_rx_len > 1 ) {
			i2c_dma_last_transfer_cmd( dcast(m_i2c), true );
			i2c_dma_rx_enable( dcast(m_i2c) );
		} else {
			i2c_acknowledge_config( dcast(m_i2c), false );
			i2c_it_config(dcast(m_i2c), I2C_IT_BUF, true );
		}
#else
		if( m_rx_len == 1 ) {
			i2c_acknowledge_config( dcast(m_i2c), false );
		}
		i2c_it_config(dcast(m_i2c), I2C_IT_BUF, true );
#endif
		//dbprintf("I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED [%i] ", m_rx_len );
	break;

	case (I2C_EVENT_MASTER_BYTE_RECEIVED|0x04):		//+BTF Byte transfer finished
	case I2C_EVENT_MASTER_BYTE_RECEIVED:
#if !CONFIG_ISIXDRV_I2C_NODMA
		m_rx_buf[0] = i2c_receive_data( dcast(m_i2c) );
		ev_finalize();
#else
	if( m_rx_cnt < m_rx_len )
	{
		m_rx_buf[m_rx_cnt++] = i2c_receive_data( dcast(m_i2c) );
		if( m_rx_cnt==m_rx_len-1 ) {
			i2c_acknowledge_config( dcast(m_i2c), false );
		}
		if( m_rx_cnt >= m_rx_len ) {
			ev_finalize();
		}
	}
#endif
	break;
	case 0x00030084:	//Handle extra states during DMA transfer
	case 0x00030000:
	break;
	default:
		//dbprintf("Unknown event %08x", event );
		ev_finalize( true );
	break;
	}
}

//! Finalize transaction
void i2c_bus::ev_finalize( bool inv_state )
{
	i2c_generate_stop(dcast(m_i2c), true );
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR|I2C_IT_BUF, false );
#if !CONFIG_ISIXDRV_I2C_NODMA
	i2c_dma_rx_disable( dcast(m_i2c) );
	i2c_dma_tx_disable( dcast(m_i2c) );
	i2c_dma_last_transfer_cmd( dcast(m_i2c), false );
	i2c_acknowledge_config( dcast(m_i2c), true );
#endif
	if( inv_state ) {
		static constexpr auto inv_state_bit = 0x80;
		m_err_flag |= inv_state_bit;
	}
	//ACK config
	m_notify.signal_isr();
}


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
		i2c_it_config( dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR|I2C_IT_BUF, false );
		i2c_generate_stop( dcast(m_i2c),true );
#if !CONFIG_ISIXDRV_I2C_NODMA
		i2c_dma_rx_disable( dcast(m_i2c) );
		i2c_dma_tx_disable( dcast(m_i2c) );
#endif
	}
	m_notify.signal_isr();
}

#if !CONFIG_ISIXDRV_I2C_NODMA
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
#endif

void i2c_bus::mdelay( unsigned timeout )
{
	isix_wait_ms( timeout );
}

extern "C" {
#if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1)
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
#if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2)
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
#if !CONFIG_ISIXDRV_I2C_NODMA
#ifndef STM32MCU_MAJOR_TYPE_F1
	//I2C1 RX
#	if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1)
	__attribute__((interrupt)) void dma1_stream0_isr_vector()
	{
		dma_clear_flag( DMA1_Stream0, DMA_FLAG_TCIF0|DMA_FLAG_TEIF0 );
		if( obj_i2c1 ) obj_i2c1->ev_dma_tc();
	}
#	endif
#	if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2)
	//I2C2DMARX
	__attribute__((interrupt)) void dma1_stream2_isr_vector()
	{
		dma_clear_flag( DMA1_Stream2, DMA_FLAG_TCIF2|DMA_FLAG_TEIF2 );
		if( obj_i2c2 ) obj_i2c2->ev_dma_tc();
	}
#	endif
#else
#	if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_1)
	//I2C1 RX
	__attribute__((interrupt)) void dma1_channel7_isr_vector()
	{
		stm32::dma_clear_flag( DMA1_FLAG_TC7 );
		if( obj_i2c1 ) obj_i2c1->ev_dma_tc();
	}
#	endif
#	if !CONFIG_ISIXDRV_I2C_USE_FIXED_I2C || (CONFIG_ISIXDRV_I2C_USE_FIXED_I2C==CONFIG_ISIXDRV_I2C_2)
	//I2C2 RX
	__attribute__((interrupt)) void dma1_channel5_isr_vector()
	{
		stm32::dma_clear_flag( DMA1_FLAG_TC5 );
		if( obj_i2c2 ) obj_i2c2->ev_dma_tc();
	}
#	endif
#endif
#endif
}

}
}

