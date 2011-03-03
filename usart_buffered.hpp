/*----------------------------------------------------------*/
/*
 * usart_buffered.hpp
 *
 *  Created on: 2009-12-09
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef USART_BUFFERED_HPP_
#define USART_BUFFERED_HPP_
/*----------------------------------------------------------*/
#include <isix.h>
#include <system.h>
/*----------------------------------------------------------*/
namespace stm32
{
namespace dev
{
/*----------------------------------------------------------*/
extern "C"
{
	void usart1_isr_vector(void) __attribute__ ((interrupt));
	void usart2_isr_vector(void) __attribute__ ((interrupt));
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_CL)
	void usart3_isr_vector(void) __attribute__ ((interrupt));
#endif
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL)
	void usart4_isr_vector(void) __attribute__ ((interrupt));
	void usart5_isr_vector(void) __attribute__ ((interrupt));
#endif
}

/*----------------------------------------------------------*/
class usart_buffered
{
	friend void usart1_isr_vector(void);
	friend void usart2_isr_vector(void);
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL)
	friend void usart3_isr_vector(void);
#endif
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL)
	friend void usart4_isr_vector(void);
	friend void usart5_isr_vector(void);
#endif
public:

	enum parity			//Baud enumeration
	{
		parity_none,
		parity_odd,
		parity_even
	};

	//Constructor
	explicit usart_buffered(
		USART_TypeDef *_usart, unsigned _pclk1_hz,
		unsigned _pclk2_hz, unsigned cbaudrate = 115200,
		std::size_t queue_size=192, parity cpar=parity_none
	);

	//Set baudrate
	void set_baudrate(unsigned new_baudrate);

	//Set parity
	void set_parity(parity new_parity);

	//Putchar
	int putchar(char c, int timeout=isix::ISIX_TIME_INFINITE)
	{
		int result = tx_queue.push( c, timeout );
		start_tx();
		return result;
	}

	//Getchar
	int getchar(char &c, isix::tick_t timeout=isix::ISIX_TIME_INFINITE)
	{
		return rx_queue.pop(c, timeout );
	}

	//Put string into the uart
	int puts(const char *str);
	int put(const void *buf, std::size_t buf_len);
	//Get string into the uart
	int gets(char *str, std::size_t max_len, isix::tick_t timeout=isix::ISIX_TIME_INFINITE);
	int get(void *buf, std::size_t max_len, isix::tick_t timeout);
protected:
	virtual void before_tx() {}
	virtual void after_tx() {}
private:
	static const unsigned USART2_IRQ_PRIO = 0;
	static const unsigned USART2_IRQ_SUB = 7;
	static const unsigned USART1_IRQ_PRIO = 1;
	static const unsigned USART1_IRQ_SUB = 6;
private:
	void start_tx();
	void isr();
	void irq_mask();
	void irq_umask() { stm32::irq_umask(); }
	void periphcfg_usart1(bool is_alternate);
	void periphcfg_usart2(bool is_alternate);
private:
	USART_TypeDef * const usart;
	const unsigned pclk1_hz;
	const unsigned pclk2_hz;
	isix::fifo<char> tx_queue;
	isix::fifo<char> rx_queue;
	volatile bool tx_en;

private: 	//Noncopyable
	usart_buffered(usart_buffered &);
	usart_buffered& operator=(const usart_buffered&);
};

/*----------------------------------------------------------*/
}
}
/*----------------------------------------------------------*/

#endif /* USART_BUFFERED_HPP_ */
