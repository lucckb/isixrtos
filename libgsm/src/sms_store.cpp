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
#include <gsm/gsm_device.hpp>
#include <foundation/tiny_printf.h>
#include <foundation/dbglog.h>
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */ 
//! Sms store names defs
constexpr const char* const smsmem_id::smsstorenames[];
/* ------------------------------------------------------------------ */ 
//Get at parser
at_parser& sms_store::at()
{
	return m_dev.get_at();
}
/* ------------------------------------------------------------------ */
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
		dbprintf("Unable to execute command %i", at().error() );
		return at().error();
	}
	m_store_flags = id.bits();
	return error::success;
}
/* ------------------------------------------------------------------ */ 
/** Get sms store indentifiers 
	@param[in] id Store identifiers bitflag class
	@return error code for storage
*/
int sms_store::get_store_identifiers( smsmem_id& id )
{
	auto resp = at().chat( "+CPMS=?","+CPMS:" );
	if( !resp ) {
		dbprintf("Unable to execute command %i", at().error() );
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
/* ------------------------------------------------------------------ */
/** Read message from phonebook */
sms_store_result_t sms_store::read_entry( int index )
{
	char buf[32]; buf[sizeof(buf)-1] = '\0';
	fnd::tiny_snprintf( buf, sizeof(buf)-1, "+CMGR=%i", index );
	char* pdu {};
	auto resp = at().chat( buf,"+CMGR:", false, true, &pdu );
	if( !resp ) {
		dbprintf("Unable to execute command %i", at().error() );
		return sms_store_result_t( at().error(), nullptr );
	}
	dbprintf("Resp %s", resp );
	dbprintf("PDU %s", pdu );
}
/* ------------------------------------------------------------------ */
}

