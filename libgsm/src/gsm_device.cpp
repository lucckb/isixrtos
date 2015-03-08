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

#include <gsm/device.hpp>
#include <foundation/dbglog.h>
#include <gsm/param_parser.hpp>
#include <foundation/tiny_printf.h>
#include <cstring>
#include <thread>
#include <chrono>
/* ------------------------------------------------------------------ */ 
//! TODO: PDU mode text messages
namespace gsm_modem {
/* ------------------------------------------------------------------ */
//! GSM device constructor
device::device( fnd::serial_port& comm,  hw_control& hwctl, unsigned cap )
	: m_at( comm ), m_hwctl( hwctl ), m_phonebook( *this ), 
	  m_sms_store( *this ), m_capabilities( cap ), m_event(m_sms_store)
{

}
/* ------------------------------------------------------------------ */ 
//! Do enable device
int device::do_enable () {

	//Switch power on the device
 	m_hwctl.power_control( true );
	m_at.discard_data( 500 );
	//Switch extended error codes
	auto resp = m_at.chat("+CMEE=1", nullptr, true, true );
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	//Set charset to the GSM
	if( set_charset("GSM") ) {
		return m_at.error();
	}
	//!TODO: Enable GPRS modem RS232 hardware pins control
	if( m_capabilities.has_hw_flow() ) 
	{
		dbprintf("Trying to set hardware flow control");
		int ret;
		do {
			resp = m_at.chat("+IFC=2,2");
			if(!resp) { ret = m_at.error(); break; }
			if( (ret=m_at.flow_control(true)<0) ) break;
		} while(0);
		if( ret ) {
			dbprintf("Unable to setup hardware flow %i", ret );
			return ret;
		}
	}
	//Set CFUN command
	resp = m_at.chat("+CFUN=1");
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	//Select Non pdu mode
	for( int retry=0; retry<10; ++retry ) {
		resp = m_at.chat( m_capabilities.has_sms_pdu()?"+CMGF=0":"+CMGF=1" );
		if( !resp )
		{
			if( (m_at.error() == error::at_cme_sim_busy ||
				m_at.error() == error::at_cme_sim_not_inserted))
			{
				dbprintf("Parser code %i", m_at.error() );
				std::this_thread::sleep_for( std::chrono::seconds(2) );
				continue;
			}
			dbprintf( "Modem error response %i", m_at.error() );	
			return m_at.error();
		}
		else break;
	}
	if( !m_capabilities.has_sms_pdu() ) 
	{
		//! Set extended headers text format
		resp = m_at.chat("+CSDH=1");
		if( !resp ) {
			dbprintf( "Modem error response %i", m_at.error() );	
			return m_at.error();
		}
	}
	//! Set event handler for the device
	m_at.set_event_handler( &m_event );
	return error::success;
}
/* ------------------------------------------------------------------ */ 
//Do single command
int device::send_command_noresp( const char *cmd, const char* arg )
{
	char buf[32]; buf[sizeof(buf)-1] = '\0';
	fnd::tiny_snprintf(buf, sizeof(buf)-1,"%s\"%s\"", cmd, arg );
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
	return send_command_noresp( "+CSCS=", charset );
}
/* ------------------------------------------------------------------ */ 
//! Set pin
int device::set_pin( const char* pin )
{
	return send_command_noresp("+CPIN=", pin );
}
/* ------------------------------------------------------------------ */
//! Get pin
int device::get_pin_status()
{
	auto respn = m_at.chat("+CPIN?", "+CPIN:");
	if( !respn ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	param_parser p( respn, m_at.bufsize() );
	auto resp = p.parse_string();
	if( p.error() ) 
		return p.error();
	if( !std::strcmp("READY", resp) ) {
		return sim_req::ready;	
	} else if( !std::strcmp("SIM PIN", resp) ) {
		return sim_req::pin;
	} else if( !std::strcmp("SIM PUK", resp) ) {
		return sim_req::puk;
	} else if( !std::strcmp("SIM PIN2", resp) ) {
		return sim_req::pin2;
	} else if( !std::strcmp("SIM PUK2", resp) ) {
		return sim_req::pin2;
	}
	return error::unexpected_resp;
}
/* ------------------------------------------------------------------ */ 
//! Register to GSM network 
int device::register_to_network( const char *pin )
{
	//Set Cops to automatic registration
	auto ret = get_pin_status( );
	if( ret < 0 ) {
		dbprintf( "Modem error response %i", errno );	
		return m_at.error();
	}
	if( ret == sim_req::pin  ) {
		//! Waiting for sim pin send it
		dbprintf("SIM pin is required" );
		ret = set_pin( pin );
		if( ret ) return ret;
	}
	else if( ret == sim_req::ready ) {
		dbprintf("SIM pin not required" );
	} else {
		if(ret>0) ret = error::unsupported_operation;
	}
	//! Disable notify messages to the device
	auto resp = m_at.chat("+CREG=0");
	if( !resp ) {
		dbprintf( "Unable to net notifier %i", m_at.error() );	
		return m_at.error();
	}
	return ret;
}
/* ------------------------------------------------------------------ */ 
/** Get current operator information 
	* @param[out] Information structure
	* @return success or error code if failure
	*/
int device::get_current_op_info( oper_info& info )
{
	char buf[] { "+COPS=3,0" };
	int val;
	for( int fmt=0; fmt<3; ++fmt ) 
	{
		buf[8] = '0' + fmt;
		auto resp = m_at.chat(buf);
		if( !resp ) {
			dbprintf( "Modem error response %i", m_at.error() );	
			return m_at.error();
		}
		resp = m_at.chat("+COPS?", "+COPS:");
		if( !resp ) {
			dbprintf( "Modem error response %i", m_at.error() );	
			return m_at.error();
		}
		param_parser p( resp, m_at.bufsize() );
		if( p.parse_int(val) < 0 ) {
			dbprintf("Parse int failed at #1 %i", p.error() );
			return p.error();	
		}
		info.mode = op_modes( val );
		if( p.parse_comma(true)>0 ) 
		{
			if( p.parse_int(val) ) {
				dbprintf("Parse int failed at #2 %i", p.error() );
				return p.error();	
			}
			if( val != fmt ) {
				dbprintf("Unexpected response");
				return error::unexpected_resp;
			}
			if( p.parse_comma() < 0 ) {
				dbprintf("Parse int failed at #3 %i", p.error() );
				return p.error();	
			}
			if( fmt != 2 ) 
			{
				auto ps = p.parse_string();
				if(!ps) {
					dbprintf("Parse string failed at #2 %i", p.error() );
					return p.error();	
				}
				if( fmt == 0 ) 
					std::strncpy( info.desc_long, ps, sizeof info.desc_long -1 );
				if( fmt == 1 )
					std::strncpy( info.desc_short, ps, sizeof info.desc_short -1 );
			} else 
			{
				if( p.parse_int(val) < 0 ) {
					if( p.error() == error::param_parser_expected_number ) {
						//Some phones return int as a normal string
						auto pi = p.parse_string();
						if( pi ) val = std::atoi( pi );
						else val = 0;
					} else {
						dbprintf("Parse int failed at #3 %i", p.error() );
						return p.error();	
					}
				}
				info.numeric_name = val;
			}
		} else break;
	}
	return error::success;
}
/* ------------------------------------------------------------------ */ 
/** Print registration status
* @return registration code or failed if fatal
*/
int device::get_registration_status()
{
	auto resp = m_at.chat("+CREG?", "+CREG:");
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	param_parser p( resp, m_at.bufsize() );
	int val;
	if( p.parse_int(val) < 0 ) {
		dbprintf("Parse int1 failed %i", p.error() );
		return p.error();
	}
	if( p.parse_comma() < 0 ) {
		dbprintf("Parse comma failed %i", p.error() );
		return p.error();
	}
	if( p.parse_int(val) < 0 ) {
		dbprintf("Unable to get registration status err %i", p.error() );	
		return p.error();
	}
	return val;
}
/* ------------------------------------------------------------------ */ 
/** Get signal strength information
* @return signal stength or error code
*/
int device::get_signal_strength()
{
	auto resp = m_at.chat("+CSQ?", "+CSQ:");
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	param_parser p( resp, m_at.bufsize() );
	int val;
	if( p.parse_int(val) < 0 ) {
		dbprintf("Parse int1 failed %i", p.error() );
		return p.error();
	}
	return val;
}
/* ------------------------------------------------------------------ */
//! Get text mode sms parameter
int device::get_text_mode_param_config( sms_text_params& param )
{
	auto resp = m_at.chat("+CSMP?", "+CSMP:");
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	param_parser p( resp, m_at.bufsize() );
	int val;
	if( p.parse_int(val) < 0 ) {
		return p.error();
	}
	param.first_octet = val;
	if( p.parse_comma() < 0 ) {
		return p.error();
	}
	if( p.parse_int(val) < 0 ) {
		return p.error();
	}
	param.validity_period = val;
	if( p.parse_comma() < 0 ) {
		return p.error();
	}
	if( p.parse_int(val) < 0 ) {
		return p.error();
	}
	param.protocol_identifier = val;
	if( p.parse_comma() < 0 ) {
		return p.error();
	}
	if( p.parse_int(val) < 0 ) {
		return p.error();
	}
	param.data_coding_scheme = val;
	return error::success;
}
/* ------------------------------------------------------------------ */
//! Set text mode parameters
int device::set_text_mode_param_config( const sms_text_params& param )
{
	char buf[32]; buf[sizeof(buf)-1] = '\0';	
	fnd::tiny_snprintf(buf, sizeof(buf)-1, "+CSMP=%i,%i,%i,%i", param.first_octet, 
			param.validity_period, param.protocol_identifier, param.data_coding_scheme );
	auto resp = m_at.chat( buf );
	if( !resp ) {
		return m_at.error();
	}
	return error::success;
}
/* ------------------------------------------------------------------ */
/** Send sms message and return
	* error code if it is required
	*/
int device::send_sms( const sms_submit& sms )
{
	sms_text_params old_txt_param;
	int ret;
	bool textmp_changed {};
	if( !m_capabilities.has_sms_pdu() ) {
		if( (ret=get_text_mode_param_config(old_txt_param)) < 0 ) {
			return ret;
		}
		auto new_txt_param = old_txt_param;
		new_txt_param.validity_period = sms.validity_period();
		if( sms.flash_message() ) {
			dbprintf("Flash message param config");
			unsigned char dcs = sms.get_dcs();
			dcs |=  dcsc::FLASH_MESSAGE;
			new_txt_param.data_coding_scheme = dcs;
		}
		if( sms.report_request() ) {
			dbprintf("Report request");
				new_txt_param.first_octet |= foctet::REPORT_REQUEST;
 		}
		if( new_txt_param != old_txt_param ) {
			if( (ret=set_text_mode_param_config(new_txt_param)) < 0 ) {
				return ret;
			}
			textmp_changed = true;
		}
	}
	int mref;
	do {
			
		char buf[32]; buf[ sizeof(buf)-1 ] = '\0';
		fnd::tiny_snprintf(buf, sizeof(buf)-1,"+CMGS=\"%s\"", sms.dest_address() );
		//NOTE: Now it is not real pdu but text behaves like pdu
		auto result = m_at.send_pdu(buf,"+CMGS:", sms.message() );
		if( !result ) {
			dbprintf("Unable to send pdu %i", m_at.error() );
			mref = m_at.error();
			break;
		}
		param_parser p( result, m_at.bufsize() );
		if( p.parse_int(mref) < 0 ) {
			mref = p.error();
			break;
		}
	}  while(0);
	if( textmp_changed )
	if( (ret=set_text_mode_param_config(old_txt_param)) < 0 ) {
		return ret;
	}
	return mref;
}

/* ------------------------------------------------------------------ */
//Set sms routing to TA
int device::set_sms_routing_to_ta( bool en_sms, bool en_cbs, 
		bool en_stat_report, bool only_reception_indication )
{
	bool sms_mode_set {};
	bool cbs_mode_set {};
	bool stat_mode_set {};
	bool buffered_mode_set {};
	bit_range modes;
	bit_range sms_modes;
	bit_range cbs_modes;
	bit_range stat_modes;
	bit_range buffer_modes;
	//Find out capabilities
	auto resp = m_at.chat("+CNMI=?", "+CNMI:");
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	param_parser p( resp, m_at.bufsize() );
	
	if( p.parse_int_list(modes)<0 ) return p.error();
	if( p.parse_comma(true)>0 ) {
		if( p.parse_int_list(sms_modes)<0 ) return p.error();
		sms_mode_set = true;
		if( p.parse_comma(true)>0 ) {
			if( p.parse_int_list(cbs_modes)<0 ) return p.error();
			cbs_mode_set = true;
			if(p.parse_comma(true)>0) {
				if( p.parse_int_list(stat_modes)<0 ) return p.error();
				stat_mode_set = true;
				if( p.parse_comma(true)>0) {
					if( p.parse_int_list(buffer_modes)<0 ) return p.error();
					buffered_mode_set = true;
				}
			}
		}
	}
	if( !sms_mode_set ) sms_modes[0] = true;
	if( !cbs_mode_set ) cbs_modes[0] = true;
	if( !stat_mode_set ) stat_modes[0] = true;
	if( !buffered_mode_set ) buffer_modes[0] = true;
	char chat_string[48] { "+CNMI=" };
	//Handle modes
	if( modes[2] ) std::strncat(chat_string,"2",sizeof(chat_string)-1);
	else if( modes[1] ) std::strncat(chat_string,"1",sizeof(chat_string)-1);
	else if( modes[0] ) std::strncat(chat_string,"0",sizeof(chat_string)-1);
	else if( modes[3] ) std::strncat(chat_string,"3",sizeof(chat_string)-1);
	if( only_reception_indication ) 
	{
		if( en_sms ) 
		{
			if( sms_modes[1] ) {
				std::strncat(chat_string,",1",sizeof(chat_string)-1);
			} else {
				dbprintf("Cant route SMS to TE");
				return error::cant_route_sms_to_te;
			}
		} else {
			std::strncat(chat_string,",0",sizeof(chat_string)-1);
		}
		if( en_cbs ) 
		{
			if( cbs_modes[1] ) {
				std::strncat(chat_string,",1",sizeof(chat_string)-1);
			} else if( cbs_modes[2] ) {
				std::strncat(chat_string,",2",sizeof(chat_string)-1);
			} else {
				dbprintf("Cant route CBS to TE");
				return error::cant_route_cb_to_te;
			}
		} else {
			std::strncat(chat_string,",0",sizeof(chat_string)-1);
		}
		if( en_stat_report ) 
		{
			if( stat_modes[1] ) {
				std::strncat(chat_string,",1",sizeof(chat_string)-1);
			} else if( stat_modes[2] ) {
				std::strncat(chat_string,",2",sizeof(chat_string)-1);
			} else {
				dbprintf("Cant route StatusReports to TE");	
				return error::cant_route_sr_to_te;
			}
		} else {
			std::strncat(chat_string,",0",sizeof(chat_string)-1);
		}
	}
	else /* only_reception_indication */
	{
		if( en_sms )
		{
			if( sms_modes[2] ) {
				std::strncat(chat_string,",2",sizeof(chat_string)-1);
			}
			else if( sms_modes[3] ) {
				std::strncat(chat_string,",3",sizeof(chat_string)-1);
			}
			else {
				dbprintf("Cannot route SMS to TE");
				return error::cant_route_sms_to_te;
			}
		} 
		else 
		{
			std::strncat(chat_string,",0",sizeof(chat_string)-1);
		}
		if( en_cbs ) 
		{
			if( cbs_modes[2] ) {
				std::strncat(chat_string,",2",sizeof(chat_string)-1);
			} else if( cbs_modes[3] ) {
				std::strncat(chat_string,",3",sizeof(chat_string)-1);
			} else {
				dbprintf("Cannot route CB to TE");
				return error::cant_route_cb_to_te;
			}
		} else {
			std::strncat(chat_string,",0",sizeof(chat_string)-1);
		}
		if( en_stat_report ) 
		{
			if( stat_modes[1] ) {
				std::strncat(chat_string,",1",sizeof(chat_string)-1);
			}
			else if( stat_modes[2] ) {
				std::strncat(chat_string,",2",sizeof(chat_string)-1);
			} else {
				dbprintf("Cannot route StatusReports to TE");
				return error::cant_route_sr_to_te;
			}
		} else {
			std::strncat(chat_string,",0",sizeof(chat_string)-1);
		}
	}
	if( buffered_mode_set ) 
	{
		if( buffer_modes[1] ) {
			std::strncat(chat_string,",1",sizeof(chat_string)-1);
		} else {
			std::strncat(chat_string,",0",sizeof(chat_string)-1);
		}
	}
	//Set capabilities accept empty response
	resp = m_at.chat(chat_string, "+CNMI:", false, true );
	if( !resp ) {
		dbprintf( "Modem error response %i", m_at.error() );	
		return m_at.error();
	}
	return error::success;
}
/* ------------------------------------------------------------------ */
}
