
/*
 * usart_buffered.hpp
 *
 *  Created on: 2009-12-09
 *      Author: lucck
 */

#pragma once

#include <isix.h>
#include <stm32system.h>
#include <foundation/drv/bus/serial_port.hpp>
#include <atomic>

namespace stm32 {
namespace dev {

extern "C"
{
	void usart1_isr_vector(void) __attribute__ ((interrupt));
	void usart2_isr_vector(void) __attribute__ ((interrupt));
#ifdef USART3
	void usart3_isr_vector(void) __attribute__ ((interrupt));
#endif
#ifdef USART4
	void usart4_isr_vector(void) __attribute__ ((interrupt));
#endif
#ifdef USART5
	void usart5_isr_vector(void) __attribute__ ((interrupt));
#endif
}


//! Usart control lines config
struct usart_cfgctl_lines {
	static constexpr char NC=-1;
	GPIO_TypeDef * port ;
	char ri_pin ;	// Input
	char dsr_pin ;	// Input
	char dtr_pin ;	// Output
	char dcd_pin ;   // Input
};

class usart_buffered : public fnd::serial_port 
{
	friend void usart1_isr_vector(void);
	friend void usart2_isr_vector(void);
#ifdef USART3
	friend void usart3_isr_vector(void);
#endif
#ifdef USART4
	friend void usart4_isr_vector(void);
#endif
#ifdef USART5
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
		unsigned cbaudrate = 115200, std::size_t queue_size=192, unsigned config=parity_none,
		unsigned _irq_prio=1, unsigned _irq_sub=7, altgpio_mode alternate_gpio_mode=altgpio_mode_0
	);

	//! Virtual destructor
	virtual ~usart_buffered() {

	}
	//! Set baudrate
	int set_baudrate(unsigned new_baudrate) override;

	//! Set parity
	int set_parity(parity new_parity) override;

	//! Set flow control
	int set_flow( flow_control flow ) override;

	//! Set data bits
	int set_databits( data_bits db ) override;

	//! Map control lines
	void map_control_lines( const usart_cfgctl_lines& cfg );

	//! Put char
	int putchar(value_type c, int timeout=tinf) override;

	//! Get char
	int getchar(value_type &c, int timeout=tinf) override;

	template <typename T> 
	int getchar(T &c, int timeout=tinf) {
		value_type oc;
		const auto ret = getchar( oc, timeout );
		c = static_cast<T>(oc);
		return ret;
	}

	//! Put string
	int puts(const value_type *str) override;

	//! Put some data
	int put(const void *buf, size_t buf_len) override;

	//! Read string
	int gets(value_type *str, size_t max_len, int timeout=tinf) override;

	//! Read some data
	int get(void *buf, size_t max_len, int timeout, size_t min_len=0 )
		override;

	//! Set IO reporting flags
	int set_ioreport( unsigned tio_report ) override;

	//! Get tiomcm statate
	int tiocm_get() const override;

	//! Set event interrupt
	int tiocm_flags( unsigned flags ) const override;

	//!Set event
	int tiocm_set( unsigned tiosigs ) override;

	//! Set amount of time
	void sleep( unsigned ms ) override {
		isix_wait_ms( ms );
	}

	//! Inject character into rx queue
	int push_rx_char( value_type ch ) override {
		auto res = rx_queue.push( ch );
		return res==ISIX_EOK?1:res;
	}

	//! Number of bytes avail
	int rx_avail() const override {
		return rx_queue.size();
	}

	//! Number of bytes avail in tx fifo
	int tx_remain() const {
		return tx_queue.size();
	}

	//! Get internal RX fifo
	const isix::fifo_base& get_rxfifo() const {
		return rx_queue;
	}

private:
	void wait_for_eot();
	void start_tx();
	void isr();
	void irq_mask() { stm32::irq_mask( irq_prio, irq_sub ); }
	void irq_umask() { stm32::irq_umask(); }
	void periphcfg_usart1(altgpio_mode mode);
	void periphcfg_usart2(altgpio_mode mode);
	void flow_gpio_config( const USART_TypeDef* usart, altgpio_mode );
#ifdef USART3
	void periphcfg_usart3(altgpio_mode mode);
#endif
#ifdef USART4
	void periphcfg_usart4(altgpio_mode mode);
#endif
#ifdef USART5
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
	unsigned m_line_config;
private:	//Noncopyable
	usart_buffered(usart_buffered &);
	usart_buffered& operator=(const usart_buffered&);
};

}
}

