/*
 * =====================================================================================
 *
 *       Filename:  sio_ppp.h
 *
 *    Description:  SIO PPP porting layer for ISIX rtos
 *
 *        Version:  1.0
 *        Created:  24.09.2015 15:22:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

namespace fnd {
	class serial_port;
}

namespace tcpip {
namespace sio {

	//! Connect hardware usart to the SIO stuff
	void connect_usart( fnd::serial_port* ser );
	//! Disconnect hardware usart form the SIO stuff
	void disconnect_usart( fnd::serial_port* ser );
	//! Allow sio operation
	void allow( bool on );
}}
