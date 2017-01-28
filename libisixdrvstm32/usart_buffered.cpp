/*
 * usart_buffered.cpp
 *
 *  Created on: 2009-12-09
 *      Author: lucck
 */

#include "isixdrv/usart_buffered.hpp"
#include <stm32system.h>
#include <stm32usart.h>

namespace stm32 {
namespace dev {

namespace	//Object pointers for interrupt
{
	usart_buffered *usart1_obj;
	usart_buffered *usart2_obj;
#ifdef USART3
	usart_buffered *usart3_obj;
#endif
#ifdef USART4
	usart_buffered *usart4_obj;
#endif
#ifdef USART5
	usart_buffered *usart5_obj;
#endif
}


namespace {
	//USART1 port
	const unsigned USART1_TX_BIT = 9;
	const unsigned USART1_RX_BIT = 10;
	const unsigned USART1_CTS_BIT = 11;
	const unsigned USART1_RTS_BIT = 12;
	GPIO_TypeDef * const USART1_PORT = GPIOA;
	//Alternate (1) usart1
	const unsigned USART1_ALT1_TX_BIT = 6;
	const unsigned USART1_ALT1_RX_BIT = 7;
	GPIO_TypeDef * const USART1_ALT1_PORT = GPIOB;
	//Alternate (2) usart 1
	const unsigned USART1_ALT2_TX_BIT = 4;
	const unsigned USART1_ALT2_RX_BIT = 5;
	GPIO_TypeDef * const USART1_ALT2_PORT = GPIOC;
	//USART2 port
	const unsigned USART2_TX_BIT = 2;
	const unsigned USART2_RX_BIT = 3;
	const unsigned USART2_CTS_BIT = 0;
	const unsigned USART2_RTS_BIT = 1;
	GPIO_TypeDef * const USART2_PORT = GPIOA;
	//Alternate usart2
	const unsigned USART2_ALT_TX_BIT = 5;
	const unsigned USART2_ALT_RX_BIT = 6;
	GPIO_TypeDef * const USART2_ALT_PORT = GPIOD;
	//USART3 port
	const unsigned USART3_TX_BIT = 10;
	const unsigned USART3_RX_BIT = 11;
	const unsigned USART3_CTS_BIT = 13;
	const unsigned USART3_RTS_BIT = 14;
	GPIO_TypeDef * const USART3_PORT = GPIOB;
	const unsigned USART3_ALT1_TX_BIT = 10;
	const unsigned USART3_ALT1_RX_BIT = 11;
	GPIO_TypeDef * const USART3_ALT1_PORT = GPIOC;
	const unsigned USART3_ALT3_TX_BIT = 8;
	const unsigned USART3_ALT3_RX_BIT = 9;
	GPIO_TypeDef * const USART3_ALT3_PORT = GPIOD;
	//USART4 port
	const unsigned USART4_TX_BIT = 10;
	const unsigned USART4_RX_BIT = 11;
	GPIO_TypeDef * const USART4_PORT = GPIOC;
	//USART5 port
	const unsigned USART5_TX_BIT = 12;
	const unsigned USART5_RX_BIT = 2;
	GPIO_TypeDef * const USART5_PORT_TX = GPIOC;
	GPIO_TypeDef * const USART5_PORT_RX = GPIOD;

}


void usart_buffered::flow_gpio_config( const USART_TypeDef* usart, altgpio_mode )
{
	if( usart == USART1 ) 
	{
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
		gpio_config(USART1_PORT, USART1_RTS_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		gpio_config(USART1_PORT, USART1_CTS_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		gpio_pin_AF_config( USART1_PORT, USART1_RTS_BIT, GPIO_AF_USART1 );
		gpio_pin_AF_config( USART1_PORT, USART1_CTS_BIT, GPIO_AF_USART1 );
#else
		gpio_abstract_config(USART1_PORT,USART1_RTS_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
		gpio_abstract_config(USART1_PORT,USART1_CTS_BIT, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
#endif
	}
	else if( usart == USART2 ) 
	{
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
		gpio_config(USART2_PORT, USART2_RTS_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		gpio_config(USART2_PORT, USART2_CTS_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		gpio_pin_AF_config( USART2_PORT, USART2_RTS_BIT, GPIO_AF_USART2 );
		gpio_pin_AF_config( USART2_PORT, USART2_CTS_BIT, GPIO_AF_USART2 );
#else
		gpio_abstract_config(USART2_PORT,USART2_RTS_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
		gpio_abstract_config(USART2_PORT,USART2_CTS_BIT, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
#endif
	}
	else if( usart == USART3 ) 
	{
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
		gpio_config(USART3_PORT, USART3_RTS_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		gpio_config(USART3_PORT, USART3_CTS_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		gpio_pin_AF_config( USART3_PORT, USART3_RTS_BIT, GPIO_AF_USART3 );
		gpio_pin_AF_config( USART3_PORT, USART3_CTS_BIT, GPIO_AF_USART3 );
#else
		gpio_abstract_config(USART3_PORT,USART3_RTS_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
		gpio_abstract_config(USART3_PORT,USART3_CTS_BIT, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
#endif
	}
}

void usart_buffered::periphcfg_usart1(altgpio_mode mode)
{
	using namespace stm32;
	RCC->APB2ENR |= RCC_APB2Periph_USART1;
	if( mode == altgpio_mode_0 )
	{
		RCC->APB2ENR |= RCC_APB2Periph_USART1;
		//Configure GPIO port TxD and RxD
		gpio_clock_enable( USART1_PORT, true );

#		ifdef _CONFIG_STM32_GPIO_V2_
		gpio_config(USART1_PORT, USART1_TX_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		gpio_config(USART1_PORT, USART1_RX_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		gpio_pin_AF_config( USART1_PORT, USART1_TX_BIT, GPIO_AF_7 );
		gpio_pin_AF_config( USART1_PORT, USART1_RX_BIT, GPIO_AF_7 );
#		else
		gpio_abstract_config(USART1_PORT,USART1_TX_BIT, AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF );
		gpio_abstract_config(USART1_PORT,USART1_RX_BIT, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
#		endif
	}
	else if( mode == altgpio_mode_1 )
	{
#		ifndef _CONFIG_STM32_GPIO_V2_
		 RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
#		endif
		 gpio_clock_enable( USART1_ALT1_PORT, true );
#		 if !defined(_CONFIG_STM32_GPIO_V2_)
		 AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;
		 gpio_abstract_config(USART1_ALT1_PORT,USART1_ALT1_TX_BIT, AGPIO_MODE_ALTERNATE_PP,  AGPIO_SPEED_HALF );
		 gpio_abstract_config(USART1_ALT1_PORT,USART1_ALT1_RX_BIT, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
#		 else
		 gpio_config(USART1_ALT1_PORT,USART1_ALT1_TX_BIT , GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		 gpio_config(USART1_ALT1_PORT,USART1_ALT1_RX_BIT , GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		 gpio_pin_AF_config( USART1_ALT1_PORT, USART1_ALT1_TX_BIT, GPIO_AF_7 );
		 gpio_pin_AF_config( USART1_ALT1_PORT, USART1_ALT1_RX_BIT, GPIO_AF_7 );
#		endif
	} else if( mode == altgpio_mode_2 ) {
#		 if defined(_CONFIG_STM32_GPIO_V2_)
		 gpio_clock_enable( USART1_ALT2_PORT, true );
		 gpio_config(USART1_ALT2_PORT,USART1_ALT2_TX_BIT , GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		 gpio_config(USART1_ALT2_PORT,USART1_ALT2_RX_BIT , GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		 gpio_pin_AF_config( USART1_ALT2_PORT, USART1_ALT2_TX_BIT, GPIO_AF_7 );
		 gpio_pin_AF_config( USART1_ALT2_PORT, USART1_ALT2_RX_BIT, GPIO_AF_7 );
#	 	 endif
	}
}


void usart_buffered::periphcfg_usart2(altgpio_mode mode)
{
	using namespace stm32;
	if( mode == altgpio_mode_0 )
	{
		gpio_clock_enable( USART2_PORT, true );
		RCC->APB1ENR |= RCC_APB1Periph_USART2;

		//Configure GPIO port TxD and RxD
#if defined(_CONFIG_STM32_GPIO_V2_)
		gpio_config(USART2_PORT, USART2_TX_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);
		gpio_config(USART2_PORT, USART2_RX_BIT, GPIO_MODE_ALTERNATE, GPIO_PUPD_NONE, AGPIO_SPEED_FULL, 0);

		gpio_pin_AF_config( USART2_PORT, USART2_TX_BIT, GPIO_AF_7 );
		gpio_pin_AF_config( USART2_PORT, USART2_RX_BIT, GPIO_AF_7 );
#else
		gpio_abstract_config( USART2_PORT, USART2_TX_BIT, AGPIO_MODE_ALTERNATE_PP,  AGPIO_SPEED_HALF );
		gpio_abstract_config( USART2_PORT, USART2_RX_BIT, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF );
#endif
	}
	else
	{
		 gpio_clock_enable( USART2_ALT_PORT, true );
		 RCC->APB1ENR |= RCC_APB1Periph_USART2;
		 gpio_abstract_config(USART2_ALT_PORT,USART2_ALT_TX_BIT,AGPIO_MODE_ALTERNATE_PP,  AGPIO_SPEED_HALF);
		 gpio_abstract_config(USART2_ALT_PORT,USART2_ALT_RX_BIT,AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF);
#ifndef _CONFIG_STM32_GPIO_V2_
		 RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
		 AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
#else
		 gpio_pin_AF_config( USART2_ALT_PORT,USART2_ALT_TX_BIT, GPIO_AF_7 );
		 gpio_pin_AF_config( USART2_ALT_PORT,USART2_ALT_RX_BIT, GPIO_AF_7 );
#endif
	}
}
#if	defined(USART3)
	void usart_buffered::periphcfg_usart3(altgpio_mode mode)
	{
		if( mode == altgpio_mode_0 )
		{
			RCC->APB1ENR |= RCC_APB1Periph_USART3;
			//Configure GPIO port TxD and RxD
			gpio_clock_enable( USART3_PORT, true );
			gpio_abstract_config(USART3_PORT,USART3_TX_BIT,AGPIO_MODE_ALTERNATE_PP,  AGPIO_SPEED_HALF);
			gpio_abstract_config(USART3_PORT,USART3_RX_BIT,AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF);
		}
		else if( mode == altgpio_mode_1 )
		{
			 RCC->APB1ENR |= RCC_APB1Periph_USART3;
			 //Configure GPIO port TxD and RxD
			 gpio_clock_enable( USART3_ALT1_PORT, true );
			 gpio_abstract_config(USART3_ALT1_PORT,USART3_ALT1_TX_BIT,AGPIO_MODE_ALTERNATE_PP,  AGPIO_SPEED_HALF);
			 gpio_abstract_config(USART3_ALT1_PORT,USART3_ALT1_RX_BIT,AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF);
#ifndef _CONFIG_STM32_GPIO_V2_
			 RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
			 AFIO->MAPR |= AFIO_MAPR_USART3_REMAP_PARTIALREMAP;
#else
			 gpio_pin_AF_config( USART3_ALT1_PORT,USART3_ALT1_TX_BIT, 7 );
			 gpio_pin_AF_config( USART3_ALT1_PORT,USART3_ALT1_RX_BIT, 7 );
#endif
		}
		else if( mode == altgpio_mode_3 )
		{
			RCC->APB1ENR |= RCC_APB1Periph_USART3;
			//Configure GPIO port TxD and RxD
			gpio_clock_enable( USART3_ALT3_PORT, true );
			gpio_abstract_config(USART3_ALT3_PORT,USART3_ALT3_TX_BIT,AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF);
			gpio_abstract_config(USART3_ALT3_PORT,USART3_ALT3_RX_BIT,AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF);
#ifndef _CONFIG_STM32_GPIO_V2_
			RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
			AFIO->MAPR |= AFIO_MAPR_USART3_REMAP_FULLREMAP;
#else
			gpio_pin_AF_config( USART3_ALT3_PORT,USART3_ALT3_TX_BIT, GPIO_AF_7 );
			gpio_pin_AF_config( USART3_ALT3_PORT,USART3_ALT3_RX_BIT, GPIO_AF_7 );
#endif
		}
	}
#endif
#ifdef USART4
	void usart_buffered::periphcfg_usart4(altgpio_mode /*mode*/)
	{
		//TODO: Add remapping
		RCC->APB1ENR |= RCC_APB1Periph_UART4;
		//Configure GPIO port TxD and RxD
		gpio_clock_enable( USART4_PORT, true );
		gpio_abstract_config(USART4_PORT,USART4_TX_BIT,AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF);
		gpio_abstract_config(USART4_PORT,USART4_RX_BIT,AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF);
	}
	void usart_buffered::periphcfg_usart5(altgpio_mode /*mode*/)
	{
		//TODO: Add remapping
		gpio_clock_enable( USART5_PORT_TX , true );
		gpio_clock_enable( USART5_PORT_RX , true );
		RCC->APB1ENR |= RCC_APB1Periph_UART5;
		//Configure GPIO port TxD and RxD
		gpio_abstract_config(USART5_PORT_TX,USART5_TX_BIT,AGPIO_MODE_ALTERNATE_PP, AGPIO_SPEED_HALF);
		gpio_abstract_config(USART5_PORT_RX,USART5_RX_BIT,AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_HALF);
	}
#endif

//! Constructor called for usart buffered
usart_buffered::usart_buffered(USART_TypeDef *_usart,
		unsigned _pclk1_hz, unsigned _pclk2_hz, unsigned cbaudrate,
		std::size_t queue_size ,unsigned config, unsigned _irq_prio, unsigned _irq_sub,
		altgpio_mode alternate_gpio_mode
) : usart(_usart), pclk1_hz(_pclk1_hz), pclk2_hz(_pclk2_hz),
	tx_queue(queue_size), rx_queue(queue_size),
	irq_prio(_irq_prio), irq_sub(_irq_sub), m_line_config( config )
{
	using namespace stm32;
	if(_usart == USART1) periphcfg_usart1(alternate_gpio_mode);
	else if(_usart == USART2) periphcfg_usart2(alternate_gpio_mode);
#ifdef USART3
	else if(_usart == USART3) periphcfg_usart3(alternate_gpio_mode);
#endif
#ifdef USART4
	else if(_usart == UART4) periphcfg_usart4(alternate_gpio_mode);
#endif
#ifdef USART5
	else if(_usart == UART5) periphcfg_usart5(alternate_gpio_mode);
#endif
	//Enable UART
	usart->CR1 = USART_CR1_UE;
	//Setup default baudrate
	set_baudrate( cbaudrate );
	set_parity( parity_mask(config) );

	//One stop bit
	usart->CR2 = USART_StopBits_1;

	//Enable receiver and transmitter and anable related interrupts
	usart->CR1 |=  USART_Mode_Rx | USART_Mode_Tx ;
	//Flush bufer
	while( usart_get_flag_status(usart,USART_FLAG_RXNE) ) {
		usart_receive_data( usart );
	}
	usart_it_config( usart, USART_IT_RXNE, true );
	IRQn irq_num = WWDG_IRQn;
	if( _usart == USART1 )
	{
		usart1_obj = this;
		irq_num = USART1_IRQn;
	}
	else if( _usart == USART2 )
	{
		usart2_obj = this;
		irq_num = USART2_IRQn;
	}
#ifdef USART3
	else if(  _usart == USART3 )
	{
		usart3_obj = this;
		irq_num = USART3_IRQn;
	}
#endif
#ifdef USART4
	else if( _usart == UART4 )
	{
		usart4_obj = this;
		irq_num = UART4_IRQn;
	}
#endif
#ifdef USART5
	else if( _usart == UART5 )
	{
		usart5_obj = this;
		irq_num = UART5_IRQn;
	}
#endif
	if(irq_num != WWDG_IRQn)
	{
		nvic_set_priority( irq_num, _irq_prio, _irq_sub );
		nvic_irq_enable( irq_num, true );
	}
}


int usart_buffered::set_baudrate(unsigned new_baudrate)
{
	wait_for_eot();
	stm32::usart_set_baudrate( usart, new_baudrate, pclk1_hz, pclk2_hz );
	return 0;
}

//! Set data bits
int usart_buffered::set_databits( data_bits db )
{
	databits_mask( m_line_config, db );
	return 0;
}

int usart_buffered::set_parity(parity new_parity)
{
	parity_mask( m_line_config, new_parity );

	//! Software parity 
	if( databits_mask(m_line_config) == data_7b ) {
		return 0;
	}
	wait_for_eot();
	//if usart parity has bit check
	if(new_parity == parity_none)
	{
		usart->CR1 &= ~(USART_CR1_M | USART_CR1_PCE);
	}
	else
	{
		usart->CR1 |= USART_CR1_M | USART_CR1_PCE;
	}

	if(new_parity == parity_odd)
	{
		usart->CR1 |= USART_CR1_PS;
	}
	else
	{
		usart->CR1 &= ~USART_CR1_PS;
	}
	return 0;
}

//! Usart start transmision called by usb endpoint
void usart_buffered::start_tx()
{
	if(tx_restart.exchange(false) ) {
		usart_it_config( usart, USART_IT_TXE, true );
	}
}

//! Wait for end of transmission
void usart_buffered::wait_for_eot()
{
	//TODO: better impl
	while( tx_queue.size() ) {
		isix::wait_ms(10);
	}
	isix::wait_ms(10);
}


//! Usart interrupt handler
void usart_buffered::isr()
{
	if( usart_get_flag_status(usart, USART_FLAG_RXNE ) )
	{
		//Received data interrupt
		value_type ch = usart_receive_data( usart );
		rx_queue.push_isr(ch);
	}
	if( usart_get_flag_status(usart, USART_FLAG_TXE) )
	{
		value_type ch;
		if( tx_queue.pop_isr(ch) == ISIX_EOK )
		{
			usart_send_data( usart, ch );
		}
		else
		{
			usart_it_config( usart, USART_IT_TXE, false );
			tx_restart = true;
		}
	}
}

int usart_buffered::getchar(value_type &c, int timeout)
{
	auto ret = rx_queue.pop( c, timeout );
	if( databits_mask(m_line_config)==data_7b) c &= 0x7f;
	return ret==ISIX_EOK?(1):(ret==ISIX_ETIMEOUT?0:ret);
}

//Put string
int usart_buffered::puts(const value_type *str)
{
	int r;
	auto ptr = str;
	while(*ptr)
	{
		r = putchar(*ptr++, ISIX_TIME_INFINITE );
		if( r < 1 ) return r;
	}
	return ptr-str;
}

int usart_buffered::putchar(value_type c, int timeout)
{
	if( databits_mask(m_line_config)==data_7b) {
		c &= 0x7f;
		switch( parity_mask(m_line_config) ) {
		case parity_even: c|= __builtin_parity(c)<<7U; break;
		case parity_odd: c|= (__builtin_parity(c)<<7U)^0x80; break;
		default: break;
		}
	}
	int result = tx_queue.push( c, timeout );
	start_tx();
	return result==ISIX_EOK?(1):(result);
}

int usart_buffered::put(const void *buf, std::size_t buf_len)
{
	int r;
	const char *bb = static_cast<const char*>(buf);
	size_t s;
	for(s=0; s<buf_len; s++)
	{
		r = putchar(bb[s]);
		if( r < 1 ) return r;
	}
	return s;
}


int usart_buffered::gets(value_type *str, std::size_t max_len, int timeout)
{
	int res;
	std::size_t l;

	for(l=0; l<max_len-1; l++)
	{
		res = getchar( str[l], timeout );
		if(res>0)
		{
			if(str[l] == '\n') break;
		}
		else
		{
			str[l] = '\0';
		    return res==0?l:res;
		}
	}
	str[l] = '\0';
	return l;
}

int usart_buffered::get(void *buf, std::size_t max_len, 
		int timeout, std::size_t min_len )
{
	int res = 0;
	char *fbuf = static_cast<char*>(buf);
    std::size_t l;
	for(l=0; l<max_len; )
	{
		res = getchar ((unsigned char &) fbuf[l], timeout );
		if( res == 1 ) 
		{
			++l;
			if( l >= min_len && !rx_avail() ) return l;
		} 
		else
		{
			return res;
		}
	}
	return l;
}

//! Set flow control
int usart_buffered::set_flow( flow_control flow )
{
	wait_for_eot();
	if( usart==USART1 || usart==USART2 || usart==USART3 )
	{
		if( flow == flow_control::flow_rtscts ) {
			flow_gpio_config( usart, altgpio_mode_0 );
		}
		usart->CR3 |= USART_CR3_RTSE | USART_CR3_CTSE;
		return 0;
	}
	return ISIX_EINVARG;
}

int usart_buffered::set_ioreport( unsigned tio_report )
{
	(void)tio_report;
	return ISIX_EINVARG;
}

int usart_buffered::tiocm_get() const
{
	int flags {};
	if( m_ctl_map.ri_pin != usart_cfgctl_lines::NC ) {
		flags |= gpio_get( m_ctl_map.port, m_ctl_map.ri_pin )?tiocm_ri:0;
	}
	if( m_ctl_map.dsr_pin != usart_cfgctl_lines::NC ) {
		flags |= gpio_get( m_ctl_map.port, m_ctl_map.dsr_pin )?tiocm_dsr:0;
	}
	if( m_ctl_map.dcd_pin != usart_cfgctl_lines::NC ) {
		flags |= gpio_get( m_ctl_map.port, m_ctl_map.dcd_pin )?tiocm_dcd:0;
	}
	if( m_ctl_map.dtr_pin != usart_cfgctl_lines::NC ) {
		flags |=  gpio_get( m_ctl_map.port, m_ctl_map.dtr_pin )?tiocm_dtr:0;
	}
	return flags;
}

int usart_buffered::tiocm_flags( unsigned /*flags*/ ) const
{
	return ISIX_ENOTSUP;
}


int usart_buffered::tiocm_set( unsigned tiosigs )
{
	if( m_ctl_map.dtr_pin != usart_cfgctl_lines::NC &&
		m_ctl_map.port ) 
	{
		if( tiosigs & tiocm_dtr ) 
			gpio_set( m_ctl_map.port, m_ctl_map.dtr_pin );
		else 
			gpio_clr( m_ctl_map.port, m_ctl_map.dtr_pin );
	}
	return ISIX_ENOTSUP;
}

//! Map control lines
void usart_buffered::map_control_lines( const usart_cfgctl_lines& cfg )
{
	if( !cfg.port ) {
		return;
	}
	if( cfg.ri_pin != usart_cfgctl_lines::NC ) {
		gpio_abstract_config( cfg.port, cfg.ri_pin, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_LOW );
	}
	if( cfg.dsr_pin != usart_cfgctl_lines::NC ) {
		gpio_abstract_config( cfg.port, cfg.dsr_pin, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_LOW );
	}
	if( cfg.dcd_pin != usart_cfgctl_lines::NC ) {
		gpio_abstract_config( cfg.port, cfg.dcd_pin, AGPIO_MODE_INPUT_FLOATING, AGPIO_SPEED_LOW );
	}
	if( cfg.dtr_pin != usart_cfgctl_lines::NC ) {
		gpio_abstract_config( cfg.port, cfg.dtr_pin, AGPIO_MODE_OUTPUT_PP, AGPIO_SPEED_LOW );
	}
 	m_ctl_map = cfg;
}

//Serial interrupts handlers
extern "C"
{
	//Usart 1
	void usart1_isr_vector(void) __attribute__ ((interrupt));
	void usart1_isr_vector(void)
	{
		if(usart1_obj) usart1_obj->isr();
	}
	//Usart 2
	void usart2_isr_vector(void) __attribute__ ((interrupt));
	void usart2_isr_vector(void)
	{
		if(usart2_obj) usart2_obj->isr();
	}
#ifdef USART3
	//Usart 2
	void usart3_isr_vector(void) __attribute__ ((interrupt));
	void usart3_isr_vector(void)
	{
		if(usart3_obj) usart3_obj->isr();
	}
#endif
#ifdef USART4
	void usart4_isr_vector(void) __attribute__ ((interrupt));
	void usart4_isr_vector(void)
	{
		if(usart4_obj) usart4_obj->isr();
	}
#endif
#ifdef USART5
	void usart5_isr_vector(void) __attribute__ ((interrupt));
	void usart5_isr_vector(void)
	{
		if(usart5_obj) usart5_obj->isr();
	}
#endif 
}


}
}

