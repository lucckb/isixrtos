/*
 * =====================================================================================
 *
 *       Filename:  serialport_unix.hpp
 *
 *    Description:  Serial port unix executable test
 *
 *        Version:  1.0
 *        Created:  20.02.2015 23:32:50
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <foundation/serial_port.hpp>
#include <memory>
#include <thread>
#include <atomic>

struct termios;

namespace fnd {

class serialport_unix : public serial_port {
public:
	serialport_unix( const char *port, unsigned baud, parity par = parity_none );
	virtual ~serialport_unix();
	//!Set baudrate
	virtual int set_baudrate(unsigned new_baudrate);
	//!Set parity
	virtual void set_parity(parity new_parity);
	/** Set special control 
	 * @param[in] flow Hardware flow control settings
	 * @param[in] tio_report input line for state change monitoring
	 */
	virtual int set_control( flow_control flow, unsigned tio_report = 0 );
	//!Putchar
	virtual int putchar( value_type c, int timeout = time_infinite );
	//!Get char
	virtual int getchar( value_type& c, int timeout = time_infinite );
	virtual int puts(const value_type *str);
	virtual int put(const void *buf, std::size_t buf_len);
	//Get string from usart
	virtual int gets(value_type *str, std::size_t max_len, int timeout=time_infinite ) ;
	//! Get data without delimiters
	virtual int get(void *buf, std::size_t max_len, 
			int timeout=time_infinite, std::size_t min_len = 0 );
	//! Get avail bytes
	virtual int rx_avail() ;
	//Get status lines
	virtual int tiocm_get();
	//Get tiocm event
	virtual int tiocm_flags( unsigned flags ) ;
	//Set status line
	virtual int tiocm_set( unsigned tiosigs );
private:
	void io_watcher_thread();
	int read_timeout( void* buf, size_t len, int timeout, size_t min_len=0 );
	int write_all( const void* buf, size_t len );
private:
	int m_fd {-1};
	std::unique_ptr<termios> m_oldcfg;
	std::unique_ptr<std::thread> m_thr_watch;
	std::atomic<unsigned> m_flags;
	std::atomic<unsigned> m_req_flags;
};

}

