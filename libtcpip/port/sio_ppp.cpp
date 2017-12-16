/*
 * =====================================================================================
 *
 *       Filename:  sio_ppp.cpp
 *
 *    Description:  SIO ppp internal implementation
 *
 *        Version:  1.0
 *        Created:  24.09.2015 17:00:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include "sio_ppp.h"
#include <foundation/sys/dbglog.h>
#include <foundation/serial_port.hpp>
extern "C" {
#include <netif/ppp/ppp.h>
}


namespace {
	fnd::serial_port* g_serial;
	volatile bool g_intr;
	volatile bool g_sio_allow;
}


namespace {
	bool in_data_mode() 
	{
		 if( g_serial ) 
			return g_sio_allow;
		 else 
			return false;
	}
}

namespace tcpip {
namespace sio {

//! Connect hardware usart to the SIO stuff
void connect_usart( fnd::serial_port* ser )
{
	g_serial = ser;
}

//! Disconnect hardware usart form the SIO stuff
void disconnect_usart( fnd::serial_port* )
{
	g_serial = nullptr;
}

void allow( bool en ) 
{
	g_sio_allow =  en;
}


}}	//Namespace end




/**
 * Reads from the serial device.
 * 
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 * 
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t , u8_t *data, u32_t len)
{
	int ret = 0;
	if( in_data_mode() ) 
	{
		ret =  g_serial->get( data, len );
		if( ret < 0 ) ret = 0;
		if( g_intr ) {
			ret = 0;
			g_intr = false;
		}
	} else {
		dbg_warn("wrong_read In cmd mode %p", g_serial );
		g_serial->sleep( 500 );
	}
	return ret;
}


/**
 * Writes to the serial device.
 * 
 * @param fd serial device handle
 * @param data pointer to data to send
 * @param len length (in bytes) of data to send
 * @return number of bytes actually sent
 * 
 * @note This function will block until all data can be sent.
 */
u32_t sio_write(sio_fd_t , u8_t *data, u32_t len)
{
	int ret = 0 ;
	if( in_data_mode() )
	{
		dbg_info("sio_wr(%u)", len );
		ret = g_serial->put(data,len);
		if( ret < 0 ) ret = 0;
	} else {
		dbg_warn("wrong_write In cmd mode %p", g_serial );
		g_serial->sleep( 500 );
	}
	return ret;
}

/**
 * Aborts a blocking sio_read() call.
 * 
 * @param fd serial device handle
 */
void sio_read_abort(sio_fd_t )
{
	g_intr = true;
	g_serial->putchar( 0 );
}
