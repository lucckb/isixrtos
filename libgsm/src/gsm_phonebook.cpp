/*
 * =====================================================================================
 *
 *       Filename:  gsm_phonebook.cpp
 *
 *    Description:  Phonebook implementation
 *
 *        Version:  1.0
 *        Created:  01.03.2015 20:12:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gsm/phonebook.hpp>
#include <gsm/gsm_device.hpp>
#include <gsm/at_parser.hpp>
#include <gsm/param_parser.hpp>
#include <foundation/dbglog.h>
#include <cstring>
#include <gsm/utility.hpp>
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */
//! Get AT parser helper function
at_parser& phonebook::at() {
	return m_dev.get_at();
}
/* ------------------------------------------------------------------ */ 
/** Select phonebook return error code or
	*  phonebook identifer
	*  @param[in] Phonebook id
	*  @param[out] Error code
	*/
int phonebook::select_book( const phbook_id& id )
{
	const auto pbname =  id.get_name();
	if( !pbname ) {
		dbprintf("Unable to get phonebook name" );
		return error::invalid_argument;
	}
	//Set phonebook
	char buf[32];
	detail::catcstr( buf, "+CPBS=\"", pbname, "\"" , sizeof buf );
	auto resp = at().chat( buf );
	if( !resp ) {
		dbprintf( "Modem error response %i", at().error() );	
		return at().error();
	}
	return error::success;
}
/* ------------------------------------------------------------------ */ 
/** Get phonebook indentifiers and return its representation
	* @param[out] ids Output identifer
	*/
int phonebook::get_phonebooks_identifiers( phbook_id& ids )
{
	auto resp = at().chat("+CPBS=?", "+CPBS:");
	if( !resp ) {
		dbprintf( "Modem error response %i", at().error() );	
		return at().error();
	}
	param_parser p( resp, at().bufsize() );
	vector<param_parser::ret_str_t> res;
	if( p.parse_string_list( res ) < 0 ) {
		dbprintf("Unable to parse phonebook entries");
		return at().error();
	}
	ids.clear();
	for( const auto e : res ) 
	{
		ids.set_bit( e );
	}
	return error::success;
}
/* ------------------------------------------------------------------ */ 
	/** Read entry from the phone book
	* @param[in] input index
	* @param[out] filled structure
	* @return Error code */
int phonebook::read_entry( int index, phbook_entry& entry )
{
	char buf[32];
	if( detail::catcstrint( buf, "+CPBR=", index, sizeof buf ) ) {
		return error::invalid_argument;
	}
	auto resp = at().chat(buf, "+CPBR:", false, true );
	if( !resp ) {
		dbprintf( "Modem error response %i", at().error() );	
		return at().error();
	}
	if( *resp == '\0' ) {
		return error::entry_not_found;
	}
	return parse_phonebook_entry( resp, entry );
}
/* ------------------------------------------------------------------ */ 
//! Parse phonebook entry
int phonebook::parse_phonebook_entry( char* buf, phbook_entry& entry )
{
	param_parser p( buf, at().bufsize() );
	int val;
	if( p.parse_int( val ) < 0 ) return p.error();
	if( p.parse_comma() < 0 ) return p.error();
	const auto tel = p.parse_string();
	if( !tel ) return p.error();
	if( p.parse_comma() < 0 ) return p.error();
	if( p.parse_int( val ) < 0 ) return p.error(); //Number format unused
	if( p.parse_comma() < 0 ) return p.error();
	const auto text = p.parse_string();
	if( !text ) return p.error();
	std::strncpy( entry.phone, tel, sizeof entry.phone - 1 );
	std::strncpy( entry.name, text, sizeof entry.name - 1 );
	return error::success;
}
/* ------------------------------------------------------------------ */ 
}
/* ------------------------------------------------------------------ */ 

