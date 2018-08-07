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

namespace  periph::drivers::uart_early {


namespace {
	USART_TypeDef* usart;
};

//! Initialize the usart
int open( const char *name, unsigned baudrate )
{
	int ret;
	do {
		dt::clk_periph pclk;
		ret = dt::get_periph_clock( name, pclk );
		if( ret < 0 ) break;
		ret = clock::device_enable(pclk);
		if( ret < 0 ) break;
		int mux = dt::get_periph_pin_mux( name );
		if( mux < 0 ) { ret = mux; break; }
		ret = dt::get_periph_pin( name, dt::pinfunc::rxd );
		if( ret < 0 && ret != error::nopin ) break;
		auto dir = LL_USART_DIRECTION_TX;
		if( ret == error::success ) {
			gpio::setup( ret, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::medium} );
			dir |= LL_USART_DIRECTION_RX;
		}
		ret = dt::get_periph_pin( name, dt::pinfunc::txd );
		if( ret < 0 ) break;
		gpio::setup( ret, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::medium} );
		uintptr_t addr = dt::get_periph_base_address( name );
		if( ret <= 0 ) return addr;
		usart = reinterpret_cast<USART_TypeDef*>(addr);
		LL_USART_InitTypeDef ucfg {
			baudrate,
			LL_USART_DATAWIDTH_8B,
			LL_USART_STOPBITS_1,
			LL_USART_PARITY_NONE,
			dir,
			LL_USART_HWCONTROL_NONE,
			LL_USART_OVERSAMPLING_16
		};
	   ret = LL_USART_Init( usart, &ucfg );
	   if( ret != SUCCESS ) {
		   ret = error::init;
		   break;
	   } else {
		   ret = error::success;
	   }
	   LL_USART_Enable(usart);
	} while(0);

	return ret;
}

//! Put char
int putc( int ch )
{
	if( !usart ) return error::noinit;
	while( !LL_USART_IsActiveFlag_TXE(usart) );
	LL_USART_TransmitData8(usart, ch);
	return ch;
}

//! Get char
int getc()
{
	if( !usart ) return error::noinit;
	if( LL_USART_IsActiveFlag_RXNE(usart) ) {
		return LL_USART_ReceiveData8(usart);
	} else {
		return error::again;
	}
}

//! Check for new chars
int isc()
{
	if( !usart ) return error::noinit;
	return LL_USART_IsActiveFlag_RXNE(usart);
}


//! Put string
int puts( const char str[] )
{
	int ret {};
	while(*str) {
		ret = putc(*str++);
		if( ret < 0 ) break;
	}
	return ret;
}


}

