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
//TODO: Add other alphabet encoding
// detect curent alphabet and convert it to phone alphabet
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
	int index, country;
	if( p.parse_int( index ) < 0 ) return p.error();
	if( p.parse_comma() < 0 ) return p.error();
	const auto tel = p.parse_string();
	if( !tel ) return p.error();
	if( p.parse_comma() < 0 ) return p.error();
	if( p.parse_int( country ) < 0 ) return p.error(); //Number format unused
	if( p.parse_comma() < 0 ) return p.error();
	const auto text = p.parse_string();
	if( !text ) return p.error();
	std::strncpy( entry.phone, tel, sizeof entry.phone - 1 );
	std::strncpy( entry.name, text, sizeof entry.name - 1 );
	return index;
}
/* ------------------------------------------------------------------ */
/** Find phonebook entry by name return number 
	* @param[in,out] Entry for find
	* @return index or error code
	*/
int phonebook::find_entry( phbook_entry& entry )
{
	char buf[64];
	//! IF empty request return invalid argument
	if( entry.name[0] == '\0' ) {
		return error::invalid_argument;	
	}
	detail::catcstr( buf, "+CPBF=\"", entry.name, "\"", sizeof buf );
	//! Accept empty response but not ignore errors
	auto resp = at().chat(buf, "+CPBF:", false, true );
	if( !resp ) {
		dbprintf( "Modem error response %i", at().error() );	
		return at().error();
	}
	if( resp[0] == '\0' ) {
		entry.phone[0] = '\0';
	}
	else {
		return parse_phonebook_entry( resp, entry );	
	}
	return error::lib_bug;
}
/* ------------------------------------------------------------------ */
//! Internal version write or delete entry
int phonebook::write_or_delete_entry( int index, const phbook_entry* phb )
{
	char buf[ sizeof(phbook_entry) + 24 ];
	detail::catcstrint( buf, "+CPBW=", index, sizeof buf );
	if( phb )	//!Also entry to write
	{
		int type { phbook_format::unknown };
		if( std::strchr(phb->phone, '+') ) {
			type = phbook_format::international;
		}
		if( detail::catparam( buf, phb->phone, type, phb->name, sizeof buf ) ) {
			return error::query_format_error;
		}
	}
	if( !at().chat(buf) ) {
		dbprintf( "Modem error response %i", at().error() );	
		return at().error();
	}
	return error::success;
}
/* ------------------------------------------------------------------ */ 
}
/* ------------------------------------------------------------------ */ 

