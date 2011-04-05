/*----------------------------------------------------------*/
/*
 * usart_buffered.cpp
 *
 *  Created on: 2009-12-09
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#include "usart_buffered.hpp"
#include <system.h>

/*----------------------------------------------------------*/
namespace stm32
{
namespace dev
{
/*----------------------------------------------------------*/
namespace	//Object pointers for interrupt
{
	usart_buffered *usart1_obj;
	usart_buffered *usart2_obj;
#if defined(STM32F10X_LD)
#elif defined(STM32F10X_MD)
	usart_buffered *usart3_obj;
#elif defined(STM32F10X_HD) || defined(STM32F10X_CL)
	usart_buffered *usart3_obj;
	usart_buffered *usart4_obj;
	usart_buffered *usart5_obj;
#else
#error STM32 devtype not defined
#endif
}

/*----------------------------------------------------------*/
namespace
{
	//USART1 port
	const unsigned USART1_TX_BIT = 9;
	const unsigned USART1_RX_BIT = 10;
	GPIO_TypeDef * const USART1_PORT = GPIOA;
	//Alternate usart1
	const unsigned USART1_ALT_TX_BIT = 6;
	const unsigned USART1_ALT_RX_BIT = 7;
	GPIO_TypeDef * const USART1_ALT_PORT = GPIOB;
	//USART2 port
	const unsigned USART2_TX_BIT = 2;
	const unsigned USART2_RX_BIT = 3;
	GPIO_TypeDef * const USART2_PORT = GPIOA;
	//Alternate usart2
	const unsigned USART2_ALT_TX_BIT = 5;
	const unsigned USART2_ALT_RX_BIT = 6;
	GPIO_TypeDef * const USART2_ALT_PORT = GPIOD;
	//USART3 port
	const unsigned USART3_TX_BIT = 10;
	const unsigned USART3_RX_BIT = 11;
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



	const unsigned CR1_UE_SET = 0x2000;
	const unsigned CCR_ENABLE_SET = 0x00000001;
	const unsigned USART_M_BIT = (1<<12);
	const unsigned USART_PCE_BIT = (1<<10);
	const unsigned USART_PS_BIT = (1<<9);
	const unsigned USART_TXEIE = (1<<7);
	const unsigned USART_TCIE = (1<<6);
	const unsigned USART_RXNEIE = (1<<5);
	const unsigned USART_RXNE = (1<<5);
	const unsigned USART_TXE = (1<<7);
	const unsigned USART_ORE = (1<<3);
	const unsigned USART_TC = (1<<6);
	const unsigned USART_FE = (1<<1);
	const unsigned USART_PE = (1<<0);
}

/*----------------------------------------------------------*/
void usart_buffered::periphcfg_usart1(altgpio_mode mode)
{
	using namespace stm32;
	if( mode == altgpio_mode_0 )
	{
		RCC->APB2ENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1;
		//Configure GPIO port TxD and RxD
		io_config(USART1_PORT,USART1_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
		io_config(USART1_PORT,USART1_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
	}
	else
	{
		 RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1;
		 io_config(USART1_ALT_PORT,USART1_ALT_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
		 io_config(USART1_ALT_PORT,USART1_ALT_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
		 AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;
	}
}
/*----------------------------------------------------------*/
void usart_buffered::periphcfg_usart2(altgpio_mode mode)
{
	using namespace stm32;
	if( mode == altgpio_mode_0 )
	{
		RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
		RCC->APB1ENR |= RCC_APB1Periph_USART2;
		//Configure GPIO port TxD and RxD
		io_config(USART2_PORT,USART2_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
		io_config(USART2_PORT,USART2_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
	}
	else
	{
		 RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |RCC_APB2Periph_GPIOD;
		 RCC->APB1ENR |= RCC_APB1Periph_USART2;
		 io_config(USART2_ALT_PORT,USART2_ALT_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
		 io_config(USART2_ALT_PORT,USART2_ALT_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
		 AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
	}
}
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_CL)
	void usart_buffered::periphcfg_usart3(altgpio_mode mode)
	{
		if( mode == altgpio_mode_0 )
		{
			RCC->APB2ENR |= RCC_APB2Periph_GPIOB;
			RCC->APB1ENR |= RCC_APB1Periph_USART3;
			//Configure GPIO port TxD and RxD
			io_config(USART3_PORT,USART3_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
			io_config(USART3_PORT,USART3_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
		}
		else if( mode == altgpio_mode_1 )
		{
			 RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |RCC_APB2Periph_GPIOC;
			 RCC->APB1ENR |= RCC_APB1Periph_USART3;
			 //Configure GPIO port TxD and RxD
			 io_config(USART3_ALT1_PORT,USART3_ALT1_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
			 io_config(USART3_ALT1_PORT,USART3_ALT1_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
			 AFIO->MAPR |= AFIO_MAPR_USART3_REMAP_PARTIALREMAP;
		}
		else if( mode == altgpio_mode_3 )
		{
			RCC->APB2ENR |= RCC_APB2ENR_AFIOEN |RCC_APB2Periph_GPIOD;
			RCC->APB1ENR |= RCC_APB1Periph_USART3;
			//Configure GPIO port TxD and RxD
			io_config(USART3_ALT3_PORT,USART3_ALT3_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
			io_config(USART3_ALT3_PORT,USART3_ALT3_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
			AFIO->MAPR |= AFIO_MAPR_USART3_REMAP_FULLREMAP;
		}
	}
#endif
#if defined(STM32F10X_HD) || defined(STM32F10X_CL)
	void usart_buffered::periphcfg_usart4(altgpio_mode /*mode*/)
	{
		//TODO: Add remapping
		RCC->APB2ENR |= RCC_APB2Periph_GPIOC;
		RCC->APB1ENR |= RCC_APB1Periph_UART4;
		//Configure GPIO port TxD and RxD
		io_config(USART4_PORT,USART4_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
		io_config(USART4_PORT,USART4_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
	}
	void usart_buffered::periphcfg_usart5(altgpio_mode /*mode*/)
	{
		//TODO: Add remapping
		RCC->APB2ENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
		RCC->APB1ENR |= RCC_APB1Periph_UART5;
		//Configure GPIO port TxD and RxD
		io_config(USART5_PORT_TX,USART5_TX_BIT,GPIO_MODE_10MHZ,GPIO_CNF_ALT_PP);
		io_config(USART5_PORT_RX,USART5_RX_BIT,GPIO_MODE_INPUT,GPIO_CNF_IN_FLOAT);
	}
#endif
/*----------------------------------------------------------*/
//! Constructor called for usart buffered
usart_buffered::usart_buffered(USART_TypeDef *_usart,
		unsigned _pclk1_hz, unsigned _pclk2_hz, unsigned cbaudrate,
		std::size_t queue_size ,parity cpar, unsigned _irq_prio, unsigned _irq_sub,
		altgpio_mode alternate_gpio_mode
) : usart(_usart), pclk1_hz(_pclk1_hz), pclk2_hz(_pclk2_hz),
	tx_queue(queue_size), rx_queue(queue_size),
	irq_prio(_irq_prio), irq_sub(_irq_sub) ,tx_en( false )
{
	using namespace stm32;
	if(_usart == USART1) periphcfg_usart1(alternate_gpio_mode);
	else if(_usart == USART2) periphcfg_usart2(alternate_gpio_mode);
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_CL)
	else if(_usart == USART3) periphcfg_usart3(alternate_gpio_mode);
#endif
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL)
	else if(_usart == UART4) periphcfg_usart4(alternate_gpio_mode);
	else if(_usart == UART5) periphcfg_usart5(alternate_gpio_mode);
#endif
	//Enable UART
	usart->CR1 = CR1_UE_SET;
	//Setup default baudrate
	set_baudrate( cbaudrate );
	set_parity( cpar );

	//One stop bit
	usart->CR2 = USART_StopBits_1;

	//Enable receiver and transmitter and anable related interrupts
	usart->CR1 |=  USART_Mode_Rx |USART_RXNEIE | USART_Mode_Tx ;
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
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_CL)
	else if(  _usart == USART3 )
	{
		usart3_obj = this;
		irq_num = USART3_IRQn;
	}
#endif
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL)
	else if(  _usart == UART4 )
	{
		usart4_obj = this;
		irq_num = UART4_IRQn;
	}
	else if(  _usart == UART5 )
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

/*----------------------------------------------------------*/
void usart_buffered::set_baudrate(unsigned new_baudrate)
{
	unsigned hz = usart==USART1?pclk2_hz:pclk1_hz;
	//TODO: Sem wait not busy waiting
	while(!(usart->SR & USART_TC)) isix::isix_wait(10);
	//Calculate baud rate
	uint32_t int_part = ((0x19 * hz) / (0x04 * new_baudrate));
	uint32_t tmp = (int_part / 0x64) << 0x04;
	uint32_t fract_part = int_part - (0x64 * (tmp >> 0x04));
	tmp |= ((((fract_part * 0x10) + 0x32) / 0x64)) & ((u8)0x0F);
	usart->BRR = tmp;
}

/*----------------------------------------------------------*/
void usart_buffered::set_parity(parity new_parity)
{
	//TODO: Sem wait not busy waiting
	while(!(usart->SR & USART_TC)) isix::isix_wait(10);

	//if usart parity has bit check
	if(new_parity == parity_none)
	{
		usart->CR1 &= ~(USART_M_BIT | USART_PCE_BIT);
	}
	else
	{
		usart->CR1 |= USART_M_BIT | USART_PCE_BIT;
	}

	if(new_parity == parity_odd)
	{
		usart->CR1 |= USART_PS_BIT;
	}
	else
	{
		usart->CR1 &= ~USART_PS_BIT;
	}
}

/*----------------------------------------------------------*/
//! Usart start transmision called by usb endpoint
void usart_buffered::start_tx()
{
	irq_mask();
	before_tx();
	if(!tx_en)
	{
    	tx_en = true;
		usart->CR1 |= USART_TXEIE;
	}
	irq_umask();
}

/*----------------------------------------------------------*/
//! Usart interrupt handler
void usart_buffered::isr()
{
	uint16_t usart_sr = usart->SR;
	if( usart_sr & USART_RXNE  )
	{
		//Received data interrupt
		unsigned char ch = usart->DR;
		//fifo_put(&hwnd->rx_fifo,ch);
		rx_queue.push_isr(ch);
	}
	if(tx_en && (usart_sr&USART_TXE) )
	{
		char ch;
		if( tx_queue.pop_isr(ch) == isix::ISIX_EOK )
		{
			usart->DR = ch;
		}
		else
		{
			usart->CR1 &= ~USART_TXEIE;
			usart->CR1 |= USART_TCIE;
			tx_en = false;
		}
	}
	if( (usart_sr & USART_TC) && !tx_en )
	{
		usart->CR1 &= ~USART_TCIE;
		after_tx();
	}
}


/*----------------------------------------------------------*/
//Put string
int usart_buffered::puts(const char *str)
{
	int r = isix::ISIX_EOK;
	while(*str)
		if( (r=putchar(*str)) == isix::ISIX_EOK ) str++;
		else return r;
	return r;
}

/*----------------------------------------------------------*/
int usart_buffered::put(const void *buf, std::size_t buf_len)
{
	int r = isix::ISIX_EOK;
	const char *bb = static_cast<const char*>(buf);
	for(std::size_t s=0; s<buf_len; s++)
	{
		r = putchar(bb[s]);
		if( r != isix::ISIX_EOK ) return r;
	}
	return r;
}

/*----------------------------------------------------------*/
int usart_buffered::gets(char *str, std::size_t max_len, isix::tick_t timeout)
{
	int res = isix::ISIX_EOK;
	std::size_t l;
	for(l=0; l<max_len; l++)
	{
		res= getchar( str[l],timeout );
		if(res==isix::ISIX_EOK )
		{
			if(str[l]=='\r') str[l] = '\0';
			else if(str[l]=='\n') { str[l] = '\0'; break; }
		}
		else
		{
		    str[l] = '\0';
		    return res;
		}
	}
	if(l>=max_len) str[l] = '\0';
	return l;
}
/*----------------------------------------------------------*/
int usart_buffered::get(void *buf, std::size_t max_len, isix::tick_t timeout)
{
	int res =0;
	char *fbuf = static_cast<char*>(buf);
	for(std::size_t l=0; l<max_len; l++)
	{
		res= getchar ( fbuf[l],timeout );
		if( res == isix::ISIX_ETIMEOUT )
			return l;
		else if(res<0)
			return res;
	}
	return res;
}
/*----------------------------------------------------------*/
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
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_CL)
	//Usart 2
	void usart3_isr_vector(void) __attribute__ ((interrupt));
	void usart3_isr_vector(void)
	{
		if(usart3_obj) usart3_obj->isr();
	}
#endif /*defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_CL) */
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL)
	void usart4_isr_vector(void) __attribute__ ((interrupt));
	void usart4_isr_vector(void)
	{
		if(usart4_obj) usart4_obj->isr();
	}
	void usart5_isr_vector(void) __attribute__ ((interrupt));
	void usart5_isr_vector(void)
	{
		if(usart5_obj) usart5_obj->isr();
	}
#endif /* #if defined(STM32F10X_HD) || defined(STM32F10X_CL) */
}

/*----------------------------------------------------------*/
}
}
/*----------------------------------------------------------*/
