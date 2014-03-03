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
#include <isixdrv/i2c_bus.hpp>
#include <stm32system.h>
#include <stm32i2c.h>
#include <stm32rcc.h>
#include <stm32dma.h>
#include <stm32gpio.h>
#include <cstdlib>
#include <config.h>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */ 
namespace stm32 {
namespace drv {
/* ------------------------------------------------------------------ */ 
//! Unnnamed namespace for internal functions
namespace {
	//Cast to device type
	inline I2C_TypeDef* dcast( void* p ) {
		return reinterpret_cast<I2C_TypeDef*>(p);
	}
	//Cast to i2c type
	inline void* to_i2c( i2c_host::busid id ) {
		switch( id ) {
			case i2c_host::busid::i2c1: return I2C1;
			case i2c_host::busid::i2c2: return I2C2;
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
}
/** DMA Function depends on device STM32F1 and F2/F4 have incompatibile DMA controller */
#ifdef STM32MCU_MAJOR_TYPE_F1
namespace {
	constexpr DMA_Channel_TypeDef* i2c2rxdma( I2C_TypeDef * const i2c )
	{
		return i2c==I2C1?(DMA1_Channel7):(DMA1_Channel5);
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
		stm32::dma_channel_disable( i2c2rxdma(i2c) );
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
			return (1);
		} else if( i2c == I2C2 ) {
			return (7);
		}
		return 0;
	}

	inline void i2c_dma_tx_config( I2C_TypeDef * const i2c, const void *buf, unsigned short len )
	{
		using namespace stm32;
		dma_init( _i2c_stream(i2c,dma_dir::tx), _i2c_chn(i2c)| DMA_DIR_MemoryToPeripheral |
				  DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
				  DMA_PeripheralDataSize_Byte | DMA_Mode_Normal | DMA_Priority_High |
				  DMA_MemoryBurst_Single | DMA_PeripheralBurst_Single,
				  DMA_FIFOMode_Disable | DMA_FIFOThreshold_HalfFull,
				  len, &i2c->DR, const_cast<volatile void*>(buf) );
	}
	inline void i2c_dma_rx_config( I2C_TypeDef * const i2c, void *buf, unsigned short len )
	{
		using namespace stm32;
		dma_init( _i2c_stream(i2c,dma_dir::rx), _i2c_chn(i2c)| DMA_DIR_PeripheralToMemory |
				  DMA_PeripheralInc_Disable | DMA_MemoryInc_Enable |
				  DMA_PeripheralDataSize_Byte | DMA_Mode_Normal | DMA_Priority_High |
				  DMA_MemoryBurst_Single | DMA_PeripheralBurst_Single,
				  DMA_FIFOMode_Disable | DMA_FIFOThreshold_HalfFull,
				  len, &i2c->DR, const_cast<volatile void*>(buf) );
	}
	inline void i2c_dma_tx_enable( I2C_TypeDef * const i2c )
	{
		stm32::dma_cmd( _i2c_stream(i2c,dma_dir::tx), true );
	}
	inline void i2c_dma_rx_enable( I2C_TypeDef * const i2c )
	{
		stm32::dma_cmd( _i2c_stream(i2c,dma_dir::rx), true );
	}
	inline void i2c_dma_tx_disable( I2C_TypeDef * const i2c )
	{
		stm32::dma_cmd( _i2c_stream(i2c,dma_dir::tx), false );
	}
	inline void i2c_dma_rx_disable( I2C_TypeDef * const i2c )
	{
		stm32::dma_cmd( _i2c_stream(i2c,dma_dir::rx), false );
	}
}
#endif
//! Defines
namespace {
	i2c_host* obj_i2c1;
	i2c_host* obj_i2c2;

	static constexpr auto I2C1_PORT = GPIOB;
	static constexpr auto I2C1_SDA_PIN = 1U<<7;
	static constexpr auto I2C1_SCL_PIN = 1U<<6;
	static constexpr auto I2C2_PORT = GPIOB;
	static constexpr auto I2C2_SDA_PIN = 1U<<11;
	static constexpr auto I2C2_SCL_PIN = 1U<<10;
	static constexpr auto I2C1_PINS = I2C1_SDA_PIN|I2C1_SCL_PIN;
	static constexpr auto I2C2_PINS = I2C2_SDA_PIN|I2C2_SCL_PIN;
}
/* ------------------------------------------------------------------ */ 
/** Constructor
	* @param[in] _i2c Interface bus ID
	* @param[in] clk_speed CLK speed in HZ
	*/
i2c_host::i2c_host( busid _i2c, unsigned clk_speed )
	: m_i2c( to_i2c(_i2c) )
{
	using namespace stm32;
	if( m_i2c == I2C1 ) {
		rcc_apb1_periph_clock_cmd( RCC_APB1Periph_I2C1, true );
		gpio_clock_enable( I2C1_PORT, true );
		gpio_abstract_config_ext( I2C1_PORT, I2C1_PINS, AGPIO_MODE_ALTERNATE_OD_PULLUP, AGPIO_SPEED_HALF );
		if( obj_i2c1 ) {
			terminate();
		}
		obj_i2c1 = this;
	} else if( m_i2c == I2C2 ) {
		rcc_apb1_periph_clock_cmd( RCC_APB1Periph_I2C2, true );
		gpio_clock_enable( I2C2_PORT, true );
		gpio_abstract_config_ext( I2C2_PORT, I2C2_PINS, AGPIO_MODE_ALTERNATE_OD_PULLUP, AGPIO_SPEED_HALF );
		if(obj_i2c2) {
			terminate();
		}
		obj_i2c2 = this;
	} else {
		//! Not supported yet
		terminate();
	}
	i2c_cmd( dcast(m_i2c), true );
	i2c_init( dcast(m_i2c), clk_speed, I2C_Mode_I2C, I2C_DutyCycle_2, 1, 
			  I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit, CONFIG_PCLK1_HZ );
	if( m_i2c == I2C1 ) {
		nvic_set_priority( I2C1_EV_IRQn, IRQ_PRIO, IRQ_SUB );
		nvic_set_priority( I2C1_ER_IRQn, IRQ_PRIO, IRQ_SUB );
		nvic_irq_enable( I2C1_EV_IRQn, true );
		nvic_irq_enable( I2C1_ER_IRQn, true );
	} else if( m_i2c == I2C2 ) {
		nvic_set_priority( I2C2_EV_IRQn, IRQ_PRIO, IRQ_SUB );
		nvic_set_priority( I2C2_ER_IRQn, IRQ_PRIO, IRQ_SUB );
		nvic_irq_enable( I2C2_EV_IRQn, true );
		nvic_irq_enable( I2C2_ER_IRQn, true );
	}
	//Enable DMA in i2c
	i2c_dma_cmd( dcast(m_i2c), true );
}
/* ------------------------------------------------------------------ */ 
/** Destructor */
i2c_host::~i2c_host()
{
	if( obj_i2c1 == this ) {
		nvic_irq_enable( I2C1_EV_IRQn, false );
		nvic_irq_enable( I2C1_ER_IRQn, false );
	} else {
		nvic_irq_enable( I2C2_EV_IRQn, false );
		nvic_irq_enable( I2C2_ER_IRQn, false );
	}
	i2c_deinit( dcast(m_i2c) );
}
/* ------------------------------------------------------------------ */ 
/* Get hwerror */
int i2c_host::get_hwerror(void) const
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
	};
	for(int i=0; i<7; i++)
	{
		if(m_err_flag & (1<<i))
			return err_tbl[i];
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
int i2c_host::transfer_7bit(uint8_t addr, const void* wbuffer, short wsize, void* rbuffer, short rsize)
{
	using namespace stm32;
	int ret;
	if( (ret=m_lock.wait(isix::ISIX_TIME_INFINITE))<0 )
	{
		return ret;
	}
	//Disable I2C irq
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, false );
	if( wbuffer )
		i2c_dma_tx_config( dcast(m_i2c), wbuffer, wsize );
	if( rbuffer )
		i2c_dma_rx_config( dcast(m_i2c), rbuffer, rsize );
	if(wbuffer)
	{
		m_addr = addr & ~(I2C_OAR1_ADD0);
	}
	else if(rbuffer)
	{
		m_addr = addr | I2C_OAR1_ADD0;
	}
	m_rx_trans = (rbuffer!=nullptr);
	m_tx_trans = (wbuffer!=nullptr);
	//ACK config
	i2c_acknowledge_config( dcast(m_i2c), true );
	//Clear status flags
	i2c_check_event(dcast(m_i2c), 0);
	i2c_receive_data(dcast(m_i2c));
	//Enable I2C irq
	i2c_it_config(dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, true );
	//Send the start
	i2c_generate_start(dcast(m_i2c), true );
	ret = m_notify.wait( TRANSACTION_TIMEOUT );
	if( ret != isix::ISIX_EOK ) {
		i2c_it_config(dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, false );
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
void i2c_host::ev_irq()
{
	using namespace stm32;
	uint32_t event = i2c_get_last_event(dcast(m_i2c));
	switch( event )
	{

	//Send address
	case I2C_EVENT_MASTER_MODE_SELECT:		//EV5
		i2c_send_f7bit_address(dcast(m_i2c), m_addr );
	break;

	//Send bytes in tx mode
	case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:	//EV6
		if( m_tx_trans )
			i2c_dma_tx_enable( dcast(m_i2c) );
		else if( m_rx_trans )
			i2c_dma_rx_enable( dcast(m_i2c) );
	break;
	case I2C_EVENT_MASTER_BYTE_TRANSMITTED:	//EV8
	{
		i2c_dma_tx_disable( dcast(m_i2c) );
		if(m_rx_trans)
		{
			//Change address to read
			m_addr |= I2C_OAR1_ADD0;
			i2c_dma_rx_enable( dcast(m_i2c) );
			i2c_generate_start( dcast(m_i2c), true );
		}
		else
		{
			i2c_generate_stop(dcast(m_i2c),true);
			i2c_it_config(dcast(m_i2c), I2C_IT_EVT|I2C_IT_ERR, false );
			m_notify.signal_isr();
		}
	}
	break;
	//Master mode selected
	case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:	//EV7
	break;
	//Master byte rcv
	case I2C_EVENT_MASTER_BYTE_RECEIVED:
		i2c_generate_stop(dcast(m_i2c),true);
		i2c_it_config(dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, false );
		m_notify.signal_isr();
		i2c_dma_rx_disable( dcast(m_i2c) );
	break;
	//Stop generated event
	default:
		i2c_it_config( dcast(m_i2c), I2C_IT_EVT| I2C_IT_ERR, false );
		i2c_generate_stop( dcast(m_i2c), true );
		m_notify.signal_isr();
	break;
	}
}
/* ------------------------------------------------------------------ */
//Error event handler
void i2c_host::err_irq()
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
extern "C" {
	/* Irq ev handler */
	void __attribute__ ((interrupt)) i2c1_ev_isr_vector(void)
	{
		dbprintf("XX");
		if( obj_i2c1 ) obj_i2c1->ev_irq();
	}
	/* Irq err handler */
	void __attribute__ ((interrupt)) i2c1_er_isr_vector(void)
	{
		dbprintf("YY");
		if( obj_i2c1 ) obj_i2c1->err_irq();
	}
	/* Irq ev handler */
	void __attribute__ ((interrupt)) i2c2_ev_isr_vector(void)
	{
		if( obj_i2c2 ) obj_i2c2->ev_irq();
	}
	/* Irq err handler */
	void __attribute__ ((interrupt)) i2c2_er_isr_vector(void)
	{
		if( obj_i2c2 ) obj_i2c2->err_irq();
	}
}
/* ------------------------------------------------------------------ */ 
}
}
/* ------------------------------------------------------------------ */ 
