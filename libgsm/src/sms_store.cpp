/*
 * =====================================================================================
 *
 *       Filename:  sms_store.cpp
 *
 *    Description:  SMS storage class implementation
 *
 *        Version:  1.0
 *        Created:  05.03.2015 18:18:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gsm/sms_store.hpp>
#include <gsm/at_parser.hpp>
#include <gsm/param_parser.hpp>
#include <gsm/device.hpp>
#include <gsm/sms_codec.hpp>
#include <foundation/tiny_printf.h>
#include <foundation/sys/dbglog.h>
  
namespace gsm_modem {
  
//! Sms store names defs
constexpr const char* const smsmem_id::smsstorenames[];
  
//Get at parser
at_parser& sms_store::at() 
{
	return m_dev.get_at();
}
 
/** Set sms store phonebook name
	* @param[in] id SMS store name 
	* @return error code
	*/
int sms_store::select_store( const smsmem_id& id )
{
	char buf[48]; buf[ sizeof(buf)-1 ] = '\0';
	const auto id1 = id.get_name();
	const auto id2 = id.get_name(true);
	if(!id1 || !id2 ) {
		return error::invalid_argument;
	}
	fnd::tiny_snprintf(buf, sizeof(buf)-1, "+CPMS=\"%s\",\"%s\",\"%s\"",
			id1, id2, id2 );
	auto resp = at().chat( buf,"+CPMS:" );
	if( !resp ) {
		dbg_err("Unable to execute command %i", at().error() );
		return at().error();
	}
	m_store_flags = id.bits();
	//! Parse of used entries in core memory
	int value;
	param_parser p( resp, at().bufsize() );
	if( !p.parse_string() ) return p.error();
	if(p.parse_comma() < 0 ) return p.error();
	if( p.parse_int(value) < 0 ) return p.error();
	if(p.parse_comma() < 0 ) return p.error();
	if( p.parse_int(value) < 0 ) return p.error();
	if(p.parse_comma() < 0 ) return p.error();
	if( !p.parse_string() ) return p.error();
	if(p.parse_comma() < 0 ) return p.error();
	//Interesting entries from second memory type
	int n_entries;
	if( p.parse_int(n_entries) < 0 ) return p.error();
	if(p.parse_comma() < 0 ) return p.error();
	if( p.parse_int(value) < 0 ) return p.error();
	m_total_entries = value;
	return n_entries;
}
  
/** Get sms store indentifiers 
	@param[in] id Store identifiers bitflag class
	@return error code for storage
*/
int sms_store::get_store_identifiers( smsmem_id& id )
{
	auto resp = at().chat( "+CPMS=?","+CPMS:" );
	if( !resp ) {
		dbg_err("Unable to execute command %i", at().error() );
		return at().error();
	}
	param_parser p( resp, at().bufsize() );
	vector<param_parser::ret_str_t> result;
	if( p.parse_string_list(result) < 0 ) {
		return p.error();	
	}
	for( auto e: result ) {
		id.set_bit( e );
	}
	return error::success;
}
 
/** Read message from phonebook */
sms_store_result_t sms_store::read_entry( int index ) 
{
	if( !m_store_flags ) {
		return sms_store_result_t(error::sms_store_not_selected, nullptr );
	}
	char buf[32]; buf[sizeof(buf)-1] = '\0';
	fnd::tiny_snprintf( buf, sizeof(buf)-1, "+CMGR=%i", index );
	char* pdu {};
	auto resp = at().chat( buf,"+CMGR:", false, true, &pdu );
	if( !resp ) {
		dbg_err("Unable to execute command %i", at().error() );
		return sms_store_result_t( at().error(), nullptr );
	}
	param_parser p( resp, at().bufsize() );
	auto str = p.parse_string();
	if( !str ) return sms_store_result_t( at().error(), nullptr );
	int msgtype; 
	if( !std::strcmp(str,"REC READ") ) {
		msgtype = smsstore_message_type::rec_read;
	} else if( !std::strcmp(str,"REC UNREAD") ) {
		msgtype = smsstore_message_type::rec_unread;
	} else if( !std::strcmp(str,"STO SENT") ) {
		msgtype = smsstore_message_type::sto_sent;	
	} else if( !std::strcmp(str,"STO UNSENT") ) {
		msgtype = smsstore_message_type::sto_unsent;	
	} else {
		return sms_store_result_t(error::sms_type_unsupported,nullptr);
	}
	if( p.parse_comma()<0 ) return sms_store_result_t( p.error(), nullptr );
	//deliver, status_report, submit_report
	if( msgtype==smsstore_message_type::rec_read||msgtype==smsstore_message_type::rec_unread ) 
	{
		auto msg = create_message<sms_deliver>( p, pdu );
		if( p.error() ) {
			return sms_store_result_t(p.error(), nullptr );
		} else {
			return sms_store_result_t( msgtype, msg );
		}
	} 
	//SMS submit, deliver_report, command
	else if( msgtype==smsstore_message_type::sto_sent||msgtype==smsstore_message_type::sto_unsent )
	{
		auto msg = create_message<sms_submit>( p, pdu );
		if( p.error() ) {
			return sms_store_result_t(p.error(), nullptr );
		} else {
			return sms_store_result_t( msgtype, msg );
		}
	}
	return sms_store_result_t(error::sms_type_unsupported,nullptr);
}
  
//! Erase of the message from the store book return error code
int sms_store::erase_entry( int index, del flags )
{
	if( !m_store_flags ) {
		return error::sms_store_not_selected;
	}
	char buf[32]; buf[sizeof(buf)-1] = '\0';
	fnd::tiny_snprintf( buf, sizeof(buf)-1, "+CMGD=%i,%i", index, int(flags) );
	auto resp = at().chat( buf, "+CMGD:", false, true );
	if( !resp ) {
		dbg_err("Unable to execute command %i", at().error() );
		return at().error();
	}
	return error::success;
}
 
}

