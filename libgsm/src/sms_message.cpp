/*
 * =====================================================================================
 *
 *       Filename:  sms_message.cpp
 *
 *    Description:  SMS message implementation classes
 *
 *        Version:  1.0
 *        Created:  03.03.2015 19:08:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <gsm/sms_message.hpp>
#include <gsm/errors.hpp>
#include <gsm/at_parser.hpp>
#include <gsm/param_parser.hpp>
#include <foundation/tiny_printf.h>
#include <foundation/dbglog.h>
#include <cstring>
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */
//TODO: Other encoding schemes long message tests etc
//Encode sms submit message
int sms_submit::encode( at_parser& at, bool pdu ) const
{
	char buf[32]; buf[ sizeof(buf)-1 ] = '\0';
	if( pdu ) return error::unsupported_operation;
	fnd::tiny_snprintf(buf, sizeof(buf)-1,"+CMGS=\"%s\"", m_dest_addr );
	//NOTE: Now it is not real pdu but text behaves like pdu
	auto result = at.send_pdu(buf,"+CMGS:", m_message );
	if( !result ) {
		dbprintf("Unable to send pdu %i", at.error() );
		return at.error();
	}
	param_parser p( result, at.bufsize() );
	int ret;
	if( p.parse_int(ret) < 0 ) {
		return p.error();
	}
	return ret;
}
/* ------------------------------------------------------------------ */ 
//Decode sms submit message
int sms_submit::decode( at_parser& , bool )
{
	return error::unsupported_operation;
}
/* ------------------------------------------------------------------ */
}
/* ------------------------------------------------------------------ */
