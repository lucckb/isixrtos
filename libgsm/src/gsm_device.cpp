/*
 * =====================================================================================
 *
 *       Filename:  gsm_device.cpp
 *
 *    Description:  GSM device modem implementation class
 *
 *        Version:  1.0
 *        Created:  28.02.2015 16:42:17
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <gsm/gsm_device.hpp>
#include <foundation/dbglog.h>
#include <gsm/param_parser.hpp>
#include <cstring>
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */
namespace  {
	//Strncat utility
	inline void catcstr( char* str1, const char* str2, 
			const char* str3, const char* str4, size_t blen ) 
	{
		std::strncpy( str1, str2 , blen );
		std::strncat( str1, str3, blen-1 );
		std::strncat( str1 ,str4, blen-1 ); 
		str1[blen-1] = '\0';
	}
}
/* ------------------------------------------------------------------ */
//! GSM device constructor
device::device( fnd::serial_port& comm,  hw_control& hwctl )
	: m_at( comm ), m_hwctl( hwctl )
{

}
/* ------------------------------------------------------------------ */ 
//! Do enable device
int device::do_enable () {

	//Switch power on the device
 	m_hwctl.power_control( true );
	//Switch extended error codes
	auto resp = m_at.chat("CMEE=1", nullptr, true, true );
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	//Set charset to the GSM
	if( set_charset("GSM") ) {
		return m_at.error();
	}
	//Select Non pdu mode
	resp = m_at.chat( "+CMGF=1" );
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	//! Set event handler for the device
	m_at.set_event_handler( &m_event );
	return error::success;
}
/* ------------------------------------------------------------------ */ 
//Do single command
int device::send_command_noresp( const char *cmd, const char* arg )
{
	char buf[32];
	catcstr( buf, cmd, arg, "\"" , sizeof buf );
	auto resp = m_at.chat(buf);
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	return error::success;
}
/* ------------------------------------------------------------------ */ 
//! Set character set to the modem
int device::set_charset( const char* charset ) 
{
	return send_command_noresp( "+CSCS=\"", charset );
}
/* ------------------------------------------------------------------ */ 
//! Set pin
int device::set_pin( const char* pin )
{
	return send_command_noresp("+CPIN=\"", pin );
}
/* ------------------------------------------------------------------ */
//! Get pin
int device::get_pin_status( const char*& resp )
{
	resp = m_at.chat("+CPIN?", "+CPIN:");
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	return error::success;
}
/* ------------------------------------------------------------------ */ 
//! Register to GSM network 
int device::register_to_network( const char *pin )
{
	const char* resp;
	auto ret = get_pin_status( resp );
	dbprintf("pin stat %i %s", ret, resp );
}
/* ------------------------------------------------------------------ */ 
}
