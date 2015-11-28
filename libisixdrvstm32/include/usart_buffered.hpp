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
#include <foundation/serial_port.hpp>
#include <atomic>
/*----------------------------------------------------------*/
namespace stm32 {
namespace dev {
/*----------------------------------------------------------*/
extern "C"
{
	void usart1_isr_vector(void) __attribute__ ((interrupt));
	void usart2_isr_vector(void) __attribute__ ((interrupt));
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || \
	defined(STM32F10X_CL) || defined(STM32MCU_MAJOR_TYPE_F4) || \
	defined(STM32MCU_MAJOR_TYPE_F2)
	void usart3_isr_vector(void) __attribute__ ((interrupt));
#endif
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL) || \
	defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	void usart4_isr_vector(void) __attribute__ ((interrupt));
	void usart5_isr_vector(void) __attribute__ ((interrupt));
#endif
}

/*----------------------------------------------------------*/
//! Usart control lines config
struct usart_cfgctl_lines {
	static constexpr char NC=-1;
	GPIO_TypeDef * port ;
	char ri_pin ;	// Input
	char dsr_pin ;	// Input
	char dtr_pin ;	// Output
	char dcd_pin ;   // Input
};
/*----------------------------------------------------------*/
class usart_buffered : public fnd::serial_port 
{
	friend void usart1_isr_vector(void);
	friend void usart2_isr_vector(void);
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || \
	defined(STM32F10X_CL) || defined(STM32MCU_MAJOR_TYPE_F4) \
	|| defined(STM32MCU_MAJOR_TYPE_F2)
	friend void usart3_isr_vector(void);
#endif
#if	defined(STM32F10X_HD) || defined(STM32F10X_CL) || \
	defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
	friend void usart4_isr_vector(void);
	friend void usart5_isr_vector(void);
#endif
public:
	using value_type =  fnd::serial_port::value_type;
private:
	using container_type = isix::fifo<value_type>;
	static constexpr auto tinf = ISIX_TIME_INFINITE;
public:
	using parity = fnd::serial_port::parity;
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

	//! Virtual destructor
	virtual ~usart_buffered() {

	}
	//! Set baudrate
	virtual int set_baudrate(unsigned new_baudrate);

	//! Set parity
	virtual int set_parity(parity new_parity);

	//! Set flow control
	virtual int set_flow( flow_control flow );

	//! Map control lines
	void map_control_lines( const usart_cfgctl_lines& cfg );

	//! Put char
	virtual int putchar(value_type c, int timeout=tinf);
	
	//! Get char
	int getchar(value_type &c, int timeout=tinf);

	template <typename T> 
	int getchar(T &c, int timeout=tinf) {
		value_type oc;
		const auto ret = getchar( oc, timeout );
		c = static_cast<T>(oc);
		return ret;
	}

	//! Put string
	virtual int puts(const value_type *str);
	
	//! Put some data
	virtual int put(const void *buf, size_t buf_len);
	
	//! Read string
	virtual int gets(value_type *str, size_t max_len, int timeout=tinf);
	
	//! Read some data
	virtual
	int get(void *buf, size_t max_len, int timeout, size_t min_len=0 );

	//! Set IO reporting flags
	virtual int set_ioreport( unsigned tio_report );
	
	//! Get tiomcm statate
	virtual int tiocm_get() const;
	
	//! Set event interrupt
	virtual int tiocm_flags( unsigned flags ) const;
	
	//!Set event
	virtual int tiocm_set( unsigned tiosigs );

	//! Set amount of time
	virtual void sleep( unsigned ms ) {
		isix_wait_ms( ms );
	}
	
	//! Inject character into rx queue
	virtual int push_rx_char( value_type ch )  {
		auto res = rx_queue.push( ch );
		return res==ISIX_EOK?1:res;
	}

	//! Number of bytes avail
	virtual int rx_avail() const { 
		return rx_queue.size(); 
	}

	//! Get internal RX fifo
	const isix::fifo_base& get_rxfifo() const {
		return rx_queue; 
	} 

private:
	void start_tx();
	void isr();
	void irq_mask() { stm32::irq_mask( irq_prio, irq_sub ); }
	void irq_umask() { stm32::irq_umask(); }
	void periphcfg_usart1(altgpio_mode mode);
	void periphcfg_usart2(altgpio_mode mode);
	void flow_gpio_config( const USART_TypeDef* usart, altgpio_mode );
#if	defined(STM32F10X_MD) || defined(STM32F10X_HD) || \
	defined(STM32F10X_CL) || defined(STM32MCU_MAJOR_TYPE_F4) \
	|| defined(STM32MCU_MAJOR_TYPE_F2)
	void periphcfg_usart3(altgpio_mode mode);
#endif
#if defined(STM32F10X_HD) || defined(STM32F10X_CL) \
	|| defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
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
	std::atomic<bool> tx_restart {true};
	usart_cfgctl_lines m_ctl_map;
private: 	//Noncopyable
	usart_buffered(usart_buffered &);
	usart_buffered& operator=(const usart_buffered&);
};

/*----------------------------------------------------------*/
}
}
/*----------------------------------------------------------*/

#endif /* USART_BUFFERED_HPP_ */
