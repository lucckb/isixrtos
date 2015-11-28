/*
 * =====================================================================================
 *
 *       Filename:  serial_port.hpp
 *
 *    Description:  Serial port abstract layer for foundation
 *
 *        Version:  1.0
 *        Created:  20.02.2015 22:35:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
/* ------------------------------------------------------------------ */
#include <cstddef>
#ifdef COMPILED_UNDER_ISIX
#include <isix.h>
#endif
/* ------------------------------------------------------------------ */
namespace fnd {
/* ------------------------------------------------------------------ */ 
class serial_port {
public:
#ifndef COMPILED_UNDER_ISIX
	static constexpr auto time_infinite=0;
#else
	static constexpr auto time_infinite=ISIX_TIME_INFINITE;
#endif
public:
	using value_type =  char;
	enum parity			//Baud enumeration
	{
		parity_none,
		parity_odd,
		parity_even
	};
	enum flow_control {
		flow_none,
		flow_rtscts
	};
	enum tiocm_signals {
		tiocm_dsr = 1,
		tiocm_dtr = 1<<1,
		tiocm_rts = 1<<2,
		tiocm_cts = 1<<3,
		tiocm_dcd = 1<<4,
		tiocm_ri  = 1<<5
	};
	//! Destructor
	virtual ~serial_port() {

	}
	//!Set baudrate
	virtual int set_baudrate(unsigned new_baudrate) = 0;
	//!Set parity
	virtual int set_parity(parity new_parity) = 0;
	/** Set special control 
	 * @param[in] flow Hardware flow control settings
	 */
	virtual int set_flow( flow_control flow ) = 0;
	 /*	Set io report
	  * @param[in] tio_report input line for state change monitoring
	  */
	virtual int set_ioreport( unsigned tio_report ) = 0;
	//!Putchar
	virtual int putchar( value_type c, int timeout = time_infinite ) = 0;
	//!Get char
	virtual int getchar( value_type& c, int timeout = time_infinite ) = 0;
	virtual int puts(const value_type *str) = 0;
	virtual int put(const void *buf, std::size_t buf_len) = 0;
	//Get string into the uart
	virtual int gets(value_type *str, std::size_t max_len, int timeout=time_infinite ) = 0;
	//! Get string 
	virtual int get(void *buf, std::size_t max_len, 
			int timeout=time_infinite, std::size_t min_len = 0 ) = 0;
	//! Get avail bytes
	virtual int rx_avail() const = 0;
	//Get status lines
	virtual int tiocm_get() const = 0;
	//Get tiocm event
	virtual int tiocm_flags( unsigned flags ) const = 0;
	//Set status line
	virtual int tiocm_set( unsigned tiosigs ) = 0;
	//Sleep for amount of time
	virtual void sleep( unsigned ms ) = 0;
	//Inject data into rx buffer
	virtual int push_rx_char( value_type )  {
		return ISIX_ENOTSUP;
	}
};
/* ------------------------------------------------------------------ */
}

