/*
 * =====================================================================================
 *
 *       Filename:  sms_message.cpp
 *
 *    Description:  SMS message class implementation
 *
 *        Version:  1.0
 *        Created:  07.03.2015 20:18:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gsm/sms_message.hpp>
#include <gsm/param_parser.hpp>
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */ 
//! Construct SMS deliver message from  txtdata
sms_deliver::sms_deliver( param_parser& p, const char* msg )
{
	int val;
	//Origin address
	if( p.parse_comma()<0 ) return ;
	auto str = p.parse_string();
	if( !str ) return ;
	origin_address( str );
	//Alpha not used
	if( p.parse_comma()<0 ) return;
	if( !p.parse_string() ) return; 
	//Service centre timestamp
	if( p.parse_comma()<0 ) return; 
	str = p.parse_string();
	if( !str ) return; 
	service_tstamp( str );
	//! TooA Skip it
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	//! First octet skip it
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	report_indication( val & foctet::REPORT_REQUEST );
	//Parse PID
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	pid( val );
	//Parse DCS
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	set_dcs( val );
	//SCA service centre address dont used by us
	if( p.parse_comma()<0 ) return ;
	if( !p.parse_string() ) return ;
	//Type of sca skip
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	//Length (for verify only)
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	if( val < int(std::strlen(msg)) ) 
	{	//Check the len
		p.error( error::sms_length_mismatch );
		return;
	}
	message( msg );
}
/* ------------------------------------------------------------------ */ 
//! Construct sumit message from deliver data
sms_submit::sms_submit( param_parser &p, const char* msg )
{
	int val;
	//Destinaton address
	if( p.parse_comma()<0 ) return ;
	auto str = p.parse_string();
	dest_address( str );
	//Alpha not used
	if( p.parse_comma()<0 ) return ;
	if( !p.parse_string() ) return ;
	//Type of destination address
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	//! First octet skip it
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	report_request( val & foctet::REPORT_REQUEST );
	//Parse PID
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	pid( val );
	//Parse DCS
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	set_dcs( val );
	// Validity period
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	validity_period(val);
	//SCA service centre address dont used by us
	if( p.parse_comma()<0 ) return ;
	if( !p.parse_string() ) return ;
	//Type of sca skip
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	//Length (for verify only)
	if( p.parse_comma()<0 ) return ;
	if( p.parse_int(val) ) return ;
	if( val < int(std::strlen(msg)) ) {	//Check the len
		p.error( error::sms_length_mismatch );
		return ;
	}
	message( msg );
}
/* ------------------------------------------------------------------ */ 
}

