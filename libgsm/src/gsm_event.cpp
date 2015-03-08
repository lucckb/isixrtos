/*
 * =====================================================================================
 *
 *       Filename:  gsm_event.cpp
 *
 *    Description:  GSM event implementation class
 *
 *        Version:  1.0
 *        Created:  07.03.2015 21:38:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gsm/event.hpp>
#include <gsm/device.hpp>
#include <gsm/param_parser.hpp>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */ 
//Default constructor
event::event( sms_store& dev )
	: m_store( dev )
{
}
/* ------------------------------------------------------------------ */ 
//! TODO: Dispatch event add support for cell broadcast
void event::dispatch( at_parser& at , char* str ) 
{
	dbprintf("TODO: impl. Dispatch event str %s", str );
	sms::mtype msg_type;
	bool indication {};
	if( !std::strncmp(str,"+CMT:",5) ) {
		msg_type = sms::t_deliver;
	} else if( !std::strncmp(str,"+CBM",5) ) {
		//TODO Broadcast not supported yet
	} else if( !std::strncmp(str,"+CDS:",5) ) {
		msg_type = sms::t_status_report;
	} else if( !std::strncmp(str,"+CMTI:",6) ) {
		indication = true;
		msg_type = sms::t_submit;
	} else if( !std::strncmp(str,"+CBMI:",6) ) {
		//TODO Broadcast not supported yey
	}
	else if( !std::strncmp(str,"+CDSI:",6) ) {
		indication = true;
		msg_type = sms::t_status_report;
	} else if( !std::strncmp(str,"RING",4) ) {
		ring_indication();

	} else if( !std::strncmp(str,"NO CARRIER",10) ) {
		no_answer();
	} else if( !std::strncmp(str,"+CLIP:",6) ) {
		param_parser p(str+6, at.bufsize()-6);
		auto num = p.parse_string();
		char cnum[16] { "+" };
		if(p.parse_comma(true)>0) 
		{
			int number_format;
			if( p.parse_int(number_format)<0) {
				dbprintf("Parse format fail %i", p.error() );
				return;
			}
			if( number_format == number_format::international ) {
				std::strncat(cnum,num,sizeof(cnum)-1);
				cnum[sizeof(cnum)-1] =  '\0';
				num = cnum;
			}
		}
		const char* alpha {};
		if(p.parse_comma(true)>0) 
		{
			if( !p.parse_string(true) ) {
				dbprintf("Unable to get SA");
				return;
			}
			if( p.parse_comma()<0 ) {
				dbprintf("Pcoma1");
				return;
			}
			int dummy;
			if( (dummy=p.parse_int(dummy,true))<0 ) {
				dbprintf("PINT2");
				return;
			}
			if( p.parse_comma(true)>0 ) {
				alpha = p.parse_string();
			}
		}
		caller_line_id( num, alpha );
	}
}
/* ------------------------------------------------------------------ */
//Callback functions
void event::sms_reception( const sms_type_ptr_t /*sms*/) 
{
	dbprintf("Unhandled sms_reception");
}
/* ------------------------------------------------------------------ */ 
//SMS reception indication
void event::sms_reception_indication(const smsmem_id& /*storage */,int index)
{
	dbprintf("Unhandled sms_reception_indication %i", index );
}
/* ------------------------------------------------------------------ */ 
// number, subaddr, alpha
void event::caller_line_id( const char* number, const char* alpha) 
{
	dbprintf("Unhandled caller_line_id(num=%s, alpha=%s)", number, alpha );
}
/* ------------------------------------------------------------------ */ 
}

