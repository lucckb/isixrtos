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
#include <cstdlib>
#include <gsm/event.hpp>
#include <gsm/device.hpp>
#include <gsm/param_parser.hpp>
#include <foundation/sys/dbglog.h>
  
namespace gsm_modem {
  
//! TODO: Dispatch event add support for cell broadcast
void event::dispatch( at_parser& at , char* str ) 
{
	sms::mtype msg_type {};
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
		//TODO Broadcast not supported yet
	} else if( !std::strncmp(str,"+CREG:",6) ) {
		param_parser p(str+6, at.bufsize()-6);
		int regs, lac {}, ci {};
		if( p.parse_int(regs) < 0 ) {
			dbg_warn("Unable to parse reg status");
			return;
		}
		if( regs > int(reg_status::registered_roaming) ) {
			dbg_warn("Value too big");
			return;
		}
		//Optional LAC and CI parsing
		if( p.parse_comma() > 0 ) {
			const auto slac = p.parse_string();
			if( !slac ) {
				dbg_warn("Unable to parse lac");
				return;
			}
			lac = std::strtol( slac, nullptr, 16 );
			if( p.parse_comma() < 0 ) {
				dbg_warn("No comma after lac");
				return;
			}
			const auto sci = p.parse_string();
			if( !sci ) {
				dbg_warn("Unable to parse CI");
				return;
			}
			ci = std::strtol( sci, nullptr, 16 );
		}
		net_registration( reg_status(regs), lac, ci );
		return;
	}
	else if( !std::strncmp(str,"+CDSI:",6) ) {
		indication = true;
		msg_type = sms::t_status_report;
	} else if( !std::strncmp(str,"RING",4) ) {
		ring_indication();
		return;

	} else if( !std::strncmp(str,"NO CARRIER",10) ) {
		no_answer();
		return;
	} else if( !std::strncmp(str,"+CLIP:",6) ) {
		param_parser p(str+6, at.bufsize()-6);
		auto num = p.parse_string();
		char cnum[16] { "+" };
		if(p.parse_comma(true)>0) 
		{
			int number_format;
			if( p.parse_int(number_format)<0) {
				dbg_warn("Parse format fail %i", p.error() );
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
				return;
			}
			if( p.parse_comma()<0 ) {
				return;
			}
			int dummy;
			if( (dummy=p.parse_int(dummy,true))<0 ) {
				return;
			}
			if( p.parse_comma(true)>0 ) {
				alpha = p.parse_string();
			}
		}
		caller_line_id( num, alpha );
		return;
	}
	if( indication )
	{
		param_parser p(str+6, at.bufsize()-6);
		const auto store_name = p.parse_string();
		if( !store_name ) {
			return;
		}
		if( p.parse_comma()<0 ) {
			return;
		}
		int index;
		if( p.parse_int(index)<0 ) {
			return;
		}
		smsmem_id mem;
		mem.set_bit( store_name );
		sms_reception_indication( mem, index );
	} 
	else 
	{
		param_parser p(str+5, at.bufsize()-5);
		//! Normal deliver SMS routed to TA
		if( msg_type == sms::t_deliver ) 
		{
			dbg_debug("Before PDU %s", str );
			sms_deliver dmsg(p);
			if( !p.error() ) {
				const auto pdu = at.get_second_line(str);
				if( pdu ) {
					dmsg.message(pdu);
					sms_reception( dmsg );
				} else {
					dbg_err("Unable to get pdu %i", at.error() );
				}
			} else {
				dbg_err("Unable to parse message %i", p.error() );
			}
		} 
		//Normal status report
		else if( msg_type == sms::t_status_report ) 
		{
			sms_status_report rmsg(p);
			if( !p.error() ) {
				sms_reception( rmsg );
			} else{
				dbg_err( "Unable to handle message %i", p.error() );
			}
		}
		//Notify excepted ACKnowledgement
		at.ack_excepted();
	}
}
 
//Callback functions
void event::sms_reception( sms& sms ) 
{
#ifdef PDEBUG
	dbg_info("Unhandled sms_reception");
	if( sms.type() == sms::t_status_report ) {
		const auto msg = static_cast<const sms_status_report&>(sms);
		dbg_debug("RecAddr %s SCTS %s DiscTime %s status %i msgreg %i",
				msg.receimpent_address(), msg.scs_timestamp(), msg.discharge_time(),
				msg.status(),  msg.msg_ref() );
	} else if( sms.type() == sms::t_deliver ) {
			const auto it = static_cast<const gsm_modem::sms_deliver&>( sms );
			dbg_debug("TSTAMP %s ORIGIN_ADDR %s PID %i REPORT_INDIC %i",
				it.service_tstamp(), it.origin_address(), it.pid(), it.report_indication() );
			dbg_debug("Content %s", it.message() );
	}
#else
	static_cast<void>(sms);
#endif
}
  
//SMS reception indication
void event::sms_reception_indication(const smsmem_id& storage ,int index)
{
	dbg_warn("Unhandled sms_reception_indication store %08x index %i", storage.bits(), index );
#ifndef PDEBUG
	static_cast<void>(storage);
	static_cast<void>(index);
#endif
}
  
// number, subaddr, alpha
void event::caller_line_id( const char* number, const char* alpha) 
{
	dbg_warn("Unhandled caller_line_id(num=%s, alpha=%s)", number, alpha );
#ifndef PDEBUG
	static_cast<void>(number);
	static_cast<void>(alpha);
#endif
}
  
}

