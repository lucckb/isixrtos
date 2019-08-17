/*
 * =====================================================================================
 *
 *       Filename:  serialport_unix.cpp
 *
 *    Description: Serial port unix implementation  debug purposes only
 *
 *        Version:  1.0
 *        Created:  20.02.2015 23:31:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <serialport_unix.hpp>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <system_error>
#include <cstring>
#include <sys/ioctl.h>
#include <poll.h>

namespace fnd {
 
namespace {
int nbaud_to_unix_baud( int baudrate ) {
	int baudr {};
	switch(baudrate) {
    case      50 : baudr = B50; break;
    case      75 : baudr = B75; break;
    case     110 : baudr = B110; break;
    case     134 : baudr = B134; break;
    case     150 : baudr = B150; break;
    case     200 : baudr = B200; break;
    case     300 : baudr = B300; break;
    case     600 : baudr = B600; break;
    case    1200 : baudr = B1200; break;
    case    1800 : baudr = B1800; break;
    case    2400 : baudr = B2400; break;
    case    4800 : baudr = B4800; break;
    case    9600 : baudr = B9600; break;
    case   19200 : baudr = B19200; break;
    case   38400 : baudr = B38400; break;
    case   57600 : baudr = B57600; break;
    case  115200 : baudr = B115200; break;
    case  230400 : baudr = B230400; break;
    case  460800 : baudr = B460800; break;
    case  500000 : baudr = B500000; break;
    case  576000 : baudr = B576000; break;
    case  921600 : baudr = B921600; break;
    case 1000000 : baudr = B1000000; break;
    case 1152000 : baudr = B1152000; break;
    case 1500000 : baudr = B1500000; break;
    case 2000000 : baudr = B2000000; break;
    case 2500000 : baudr = B2500000; break;
    case 3000000 : baudr = B3000000; break;
    case 3500000 : baudr = B3500000; break;
    case 4000000 : baudr = B4000000; break;
    default      : 
				   throw std::invalid_argument("Unable to find baudrate");
                   break;
  }
  return baudr;	
}
 
void parity_to_unix_parity( int& cpar, int& ipar, serial_port::parity par ) 
{
	switch( par ) {
	case serial_port::parity_none:
		cpar = 0;
		ipar = IGNPAR;
		break;
	case serial_port::parity_odd:
		cpar = PARENB|PARODD;
		ipar = INPCK;
		break;
	case serial_port::parity_even:
		cpar = PARENB;
		ipar = INPCK;
		break;
	}
}
 
int tiocm_bits_to_unix( unsigned bits ) 
{
	int ret {};
	if( bits & serial_port::tiocm_dsr ) {
		ret |= TIOCM_DSR;
	}
	if( bits & serial_port::tiocm_dtr ) {
		ret |= TIOCM_DTR;
	}
	if( bits & serial_port::tiocm_rts ) {
		ret |= TIOCM_RTS;
	}
	if( bits & serial_port::tiocm_cts ) {
		ret |= TIOCM_CTS;
	}
	if( bits & serial_port::tiocm_dcd ) {
		ret |= TIOCM_CAR;
	}
	if( bits & serial_port::tiocm_ri ) {
		ret |= TIOCM_RI;
	}
	return ret;
}
 
unsigned unix_to_tiocm_bits( int bits ) 
{
	unsigned ret {};
	if( bits & TIOCM_DSR ) {
		ret |= serial_port::tiocm_dsr;
	}
	if( bits & TIOCM_DTR ) {
		ret |= serial_port::tiocm_dtr;
	}
	if( bits & TIOCM_RTS ) {
		ret |= serial_port::tiocm_rts;
	}
	if( bits & TIOCM_CTS ) {
		ret |= serial_port::tiocm_cts;
	}
	if( bits & TIOCM_CAR ) {
		ret |= serial_port::tiocm_dcd;
	}
	if( bits & TIOCM_RI ) {
		ret |= serial_port::tiocm_ri;
	}
	return ret;
}
 
}	//! Unnamed namespace end
 
serialport_unix::serialport_unix( const char *port , unsigned baud, parity par )
	: m_oldcfg( new termios )
{
	m_fd = ::open( port, O_RDWR | O_NOCTTY );
	if (m_fd == -1) {
		throw std::system_error( errno, std::system_category() );
	}
	if( ::tcgetattr( m_fd, m_oldcfg.get() ) ) {
		throw std::system_error( errno, std::system_category() );
	}
	termios tios;
	std::memset( &tios, 0, sizeof tios );
	int cpar, ipar;
	parity_to_unix_parity( cpar, ipar, par );
	tios.c_cflag = CS8|CLOCAL|CREAD|cpar; //RTS cts flow
	tios.c_iflag = ipar;
	tios.c_oflag = 0;
	tios.c_lflag = 0;
	tios.c_cc[VMIN] = 0;	//block until n bytes recv
	tios.c_cc[VTIME] = 10;	//Block when vtime
	const auto bbaud = nbaud_to_unix_baud( baud );
	cfsetispeed( &tios, bbaud );
	cfsetospeed( &tios, bbaud );
	if( tcsetattr( m_fd, TCSANOW, &tios ) ) {
		throw std::system_error( errno, std::system_category() );
	}
}
 
//Destructor
serialport_unix::~serialport_unix() 
{
	m_thr_watch.reset();
	tcsetattr( m_fd, TCSANOW, m_oldcfg.get() );
	close(m_fd);
}
  
//!Set baudrate
int serialport_unix::set_baudrate(unsigned baud )
{
	termios tios;
	if( ::tcgetattr( m_fd, &tios ) ) {
		throw std::system_error( errno, std::system_category() );
	}
	const auto bbaud = nbaud_to_unix_baud( baud );
	cfsetispeed( &tios, bbaud );
	cfsetospeed( &tios, bbaud );
	if( tcsetattr( m_fd, TCSANOW, &tios ) ) {
		throw std::system_error( errno, std::system_category() );
	}
	return 0;
}
  
//!Set parity
int serialport_unix::set_parity( parity cparity )
{
	termios tios;
	if( ::tcgetattr( m_fd, &tios ) ) {
		throw std::system_error( errno, std::system_category() );
	}
	tios.c_cflag &= ~(PARENB|PARODD);
	tios.c_cflag &= ~(INPCK|IGNPAR);
	int cpar, ipar;
	parity_to_unix_parity( cpar, ipar, cparity );
	tios.c_cflag |= cpar;
	tios.c_iflag |= ipar;
	if( tcsetattr( m_fd, TCSANOW, &tios ) ) {
		throw std::system_error( errno, std::system_category() );
	}
	return 0;
}
 
/** Set special control 
	* @param[in] flow Hardware flow control settings
	* @param[in] tio_report input line for state change monitoring
	*/
int serialport_unix::set_flow( flow_control flow )
{
	
	termios tios;
	if( ::tcgetattr( m_fd, &tios ) ) {
		throw std::system_error( errno, std::system_category() );
	}
	if( flow == flow_none ) {
		tios.c_cflag &= ~CRTSCTS;
	} else if( flow == flow_rtscts ) {
		tios.c_cflag |= CRTSCTS;
	}
	if( tcsetattr( m_fd, TCSANOW, &tios ) ) {
		throw std::system_error( errno, std::system_category() );
	}
	return 0;
}
  
/*	Set io report
* @param[in] tio_report input line for state change monitoring
*/
int serialport_unix::set_ioreport( unsigned tio_report )
{
	if( tio_report && !m_thr_watch ) {
		m_req_flags = tio_report;
		m_thr_watch.reset( 
			new std::thread( std::bind( &serialport_unix::io_watcher_thread, std::ref(*this)) )
		);
	} else if( !tio_report && m_thr_watch ) {
		m_thr_watch.reset();
	} else {
		if( m_req_flags != tio_report ) {
			m_req_flags = tio_report;
			m_thr_watch.reset( 
				new std::thread( std::bind( &serialport_unix::io_watcher_thread, std::ref(*this)) )
			);
		}
	}
	return 0;
}
 
//!Putchar
int serialport_unix::putchar( value_type c, int )
{
	return write_all( &c, sizeof c );
}
  
//!Get char
int serialport_unix::getchar( value_type& c, int timeout )
{
	return read_timeout( &c, sizeof c, timeout );
}
 
//! Putstring
int serialport_unix::puts(const value_type *str)
{
	return write_all( str, std::strlen(str) );
}
  
//Put data
int serialport_unix::put(const void *buf, std::size_t buf_len)
{
	return write_all( buf, buf_len );
}
  
//Get string into the uart
int serialport_unix::gets(value_type *str, std::size_t max_len, int timeout ) 
{
	int res {};
	std::size_t l;
	
	for(l=0; l<max_len; l++)
	{
		res = getchar(str[l], timeout );
		if( res == 1 )
		{
			if(str[l] == '\r') str[l] = '\0';
			else if(str[l] == '\n') { str[l] = '\0'; break; }
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
 
//! Get data
int serialport_unix::get(void *buf, std::size_t max_len, int timeout, 
		std::size_t min_len )
{
	return read_timeout( buf, max_len, timeout, min_len );
}
 
//! Get avail bytes
int serialport_unix::rx_avail() const
{
	int result {};
	if( ::ioctl( m_fd, FIONREAD, &result ) == 0 ) {
		return result;	
	}
	return -1;
}
 
//!Get status lines
int serialport_unix::tiocm_get() const
{
	int status {};
	if( ::ioctl( m_fd, TIOCMGET, &status ) ) {
		status = -1;
	}
	return unix_to_tiocm_bits( status );
}
 
//!Get tiocm event
int serialport_unix::tiocm_flags( unsigned flags ) const
{
	return m_flags.fetch_and( ~flags );
}
 
//!Set status line
int serialport_unix::tiocm_set( unsigned tiosigs )
{
	auto flags = tiocm_bits_to_unix( tiosigs );
	return ::ioctl( m_fd, TIOCMSET, &flags );
}
 
//Watcher thread
void serialport_unix::io_watcher_thread() 
{
	int flags {}; int stat {};
	flags = tiocm_bits_to_unix( m_req_flags );
	for(;;) {
		if( ioctl( m_fd, TIOCMIWAIT, &flags ) ) {
			return;
		}
		if( ioctl( m_fd, TIOCMGET, &stat ) ) {
			return;
		}
		unsigned out {};
		if( stat & TIOCM_CAR ) {
			out |= tiocm_dcd;	
		}
		if( stat & TIOCM_DSR ) {
			out |= tiocm_dsr;	
		}
		if( stat & TIOCM_RI ) {
			out |= tiocm_ri;	
		}
		m_flags = out;
	}
}
  
int serialport_unix::read_timeout( void* buf, size_t len, int timeout, size_t min_len )
{
	pollfd pfd[1];
	pfd[0].fd = m_fd;
	pfd[0].events = POLLIN;
	if( min_len > len ) {
		return -1;
	}
	int ret;
	size_t pos {};
	do {
		ret = ::poll( pfd, 1, timeout ); 
		if( ret > 0 ) {	
			ret = read(m_fd,reinterpret_cast<char*>(buf)+pos, len-pos );
			if( ret < 0 ) break;
			pos += ret;	
			if( min_len == 0 ) {
				break;
			}
		} else if( ret == 0 ) {
			break;	//Timeout
		} else {
			break; //Fatal error
		}
	} while( pos < min_len );
	if( ret >= 0 ) ret = pos;
	return ret;
}
 
int serialport_unix::write_all( const void* buf, size_t len )
{
	char *d = (char*)buf;
	size_t writeb = 0;
	int res;
	do
	{
		res = ::write(m_fd,d+writeb,len-writeb);
		if(res<0) return -1;
		writeb+= len;
	}
	while(len-writeb>0);
	return len;

}
 
void serialport_unix::sleep( unsigned ms ) 
{
	usleep(1000*ms);
}
  
}


