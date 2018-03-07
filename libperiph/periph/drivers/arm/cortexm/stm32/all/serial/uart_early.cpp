/*
 * =====================================================================================
 *
 *       Filename:  uart_early.cpp
 *
 *    Description:  Uart early driver for STM32
 *
 *        Version:  1.0
 *        Created:  03.03.2018 21:07:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <periph/drivers/serial/uart_early.hpp>
#include <periph/dt/dts.hpp>
#include <periph/clock/clocks.hpp>
#include <periph/gpio/gpio.hpp>
#include <stm32_ll_usart.h>

namespace  periph {
namespace drivers {
namespace uart_early {


//! Initialize the usart
int init( const char *name, unsigned baudrate )
{
	int ret;
	do {
		dt::clk_periph pclk;
		ret = dt::get_periph_clock( name, pclk );
		if( ret < 0 ) break;
		ret = clock::device_enable( pclk );
		if( ret < 0 ) break;
		int mux = dt::get_periph_pin_mux( name );
		if( mux < 0 ) { ret = mux; break; }
		ret = dt::get_periph_pin( name, dt::pinfunc::rxd );
		if( ret < 0 || ret != error::nopin ) break;
		if( ret == error::success ) {
			gpio::setup( ret, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::medium} );
		}
		ret = dt::get_periph_pin( name, dt::pinfunc::txd );
		if( ret < 0 ) break;
		gpio::setup( ret, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::medium} );
	} while(0);
	LL_USART_InitTypeDef ucfg;
	ucfg.BaudRate = baudrate;
	ucfg.DataWidth = LL_USART_DATAWIDTH_8B;
	ucfg.StopBits = LL_USART_STOPBITS_1;
	ucfg.Parity = LL_USART_PARITY_NONE;    /* When using Parity the word length must be configured to 9 bits */
	ucfg.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	ucfg.TransferDirection = LL_USART_DIRECTION_RX | LL_USART_DIRECTION_TX;
	ucfg.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART1, &ucfg);
	return ret;
}

//! Put char
int putc( int ch )
{
	return ch;
}

//! Get char
int getc()
{
	return 0;
}

//! Check for new chars
int isc()
{
	return 0;
}


//! Put string
int puts( const char str[] )
{
	return str[0];
}


}}}

