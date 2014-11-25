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
#include <stm32system.h>
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
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	void usart3_isr_vector(void) __attribute__ ((interrupt));
#endif
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	void usart4_isr_vector(void) __attribute__ ((interrupt));
	void usart5_isr_vector(void) __attribute__ ((interrupt));
#endif
}


/*----------------------------------------------------------*/
class usart_buffered
{
	friend void usart1_isr_vector(void);
	friend void usart2_isr_vector(void);
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	friend void usart3_isr_vector(void);
#endif
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	friend void usart4_isr_vector(void);
	friend void usart5_isr_vector(void);
#endif
public:
	using value_type =  char;
private:
	using container_type = isix::fifo<value_type>;
public:
	enum parity			//Baud enumeration
	{
		parity_none,
		parity_odd,
		parity_even
	};
	enum altgpio_mode
	{
		altgpio_mode_0,
		altgpio_mode_1,
		altgpio_mode_2,
		altgpio_mode_3
	};
	//Constructor
	usart_buffered(
		USART_TypeDef *_usart, unsigned _pclk1_hz, unsigned _pclk2_hz,
		unsigned cbaudrate = 115200, std::size_t queue_size=192, parity cpar=parity_none,
		unsigned _irq_prio=1, unsigned _irq_sub=7, altgpio_mode alternate_gpio_mode=altgpio_mode_0
	);

	//Set baudrate
	void set_baudrate(unsigned new_baudrate);

	//Set parity
	void set_parity(parity new_parity);

	//Putchar
	int putchar(value_type c, int timeout=isix::ISIX_TIME_INFINITE)
	{
		int result = tx_queue.push( c, timeout );
		start_tx();
		return result;
	}

	//Getchar
	template <typename T>
	int getchar( T &c, isix::tick_t timeout=isix::ISIX_TIME_INFINITE )
	{
		value_type r;
		auto ret = rx_queue.pop( r, timeout );
		c = static_cast<T>(r);
		return ret;
	}
	//Put string into the uart
	int puts(const value_type *str);
	int put(const void *buf, std::size_t buf_len);
	//Get string into the uart
	int gets(value_type *str, std::size_t max_len, isix::tick_t timeout=isix::ISIX_TIME_INFINITE);
	int get(void *buf, std::size_t max_len, isix::tick_t timeout);
	int rx_avail() const { return rx_queue.size(); }
	const isix::fifo_base& get_rxfifo() const { return rx_queue; } 
protected:
	virtual void before_tx() {}
	virtual void after_tx() {}

private:
	void start_tx();
	void isr();
	void irq_mask() { stm32::irq_mask( irq_prio, irq_sub ); }
	void irq_umask() { stm32::irq_umask(); }
	void periphcfg_usart1(altgpio_mode mode);
	void periphcfg_usart2(altgpio_mode mode);
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	void periphcfg_usart3(altgpio_mode mode);
#endif
#if defined(STM32F10X_HD) || defined(STM32F10X_CL) || defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	void periphcfg_usart4(altgpio_mode mode);
	void periphcfg_usart5(altgpio_mode mode);
#endif
private:
	USART_TypeDef * const usart;
	const unsigned pclk1_hz;
	const unsigned pclk2_hz;
	container_type tx_queue;
	container_type rx_queue;
	const unsigned char irq_prio;
	const unsigned char irq_sub;
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
