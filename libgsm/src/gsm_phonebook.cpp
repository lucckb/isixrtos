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
#include <gsm/device.hpp>
#include <gsm/at_parser.hpp>
#include <gsm/param_parser.hpp>
#include <foundation/sys/dbglog.h>
#include <foundation/tiny_printf.h>
#include <cstring>
  
//TODO: Add other alphabet encoding
// detect curent alphabet and convert it to phone alphabet
// TODO: Long timeout for searching in address book (need at parser suport )

  
namespace gsm_modem {
  
// Constexr static def
constexpr const char* const phbook_id::bookids[];
 
//! Get AT parser helper function
at_parser& phonebook::at() {
	return m_dev.get_at();
}
  
/** Select phonebook return error code or
	*  phonebook identifer
	*  @param[in] Phonebook id
	*  @param[out] Error code
	*/
int phonebook::select_book( const phbook_id& id )
{
	const auto pbname =  id.get_name();
	if( !pbname ) {
		dbg_err("Unable to get phonebook name" );
		return error::invalid_argument;
	}
	//Set phonebook
	char buf[32]; buf[sizeof(buf)-1] = '\0';
	fnd::tiny_snprintf(buf, sizeof(buf)-1, "+CPBS=\"%s\"", pbname );
	auto resp = at().chat( buf );
	if( !resp ) {
		dbg_err( "Modem error response %i", at().error() );	
		return at().error();
	}
	m_curr_book =  id.bits();
	return error::success;
}
  
/** Get phonebook indentifiers and return its representation
	* @param[out] ids Output identifer
	*/
int phonebook::get_phonebooks_identifiers( phbook_id& ids )
{
	auto resp = at().chat("+CPBS=?", "+CPBS:");
	if( !resp ) {
		dbg_err( "Modem error response %i", at().error() );	
		return at().error();
	}
	param_parser p( resp, at().bufsize() );
	vector<param_parser::ret_str_t> res;
	if( p.parse_string_list( res ) < 0 ) {
		dbg_err("Unable to parse phonebook entries");
		return at().error();
	}
	ids.clear();
	for( const auto e : res ) 
	{
		ids.set_bit( e );
	}
	return error::success;
}
  
	/** Read entry from the phone book
	* @param[in] input index
	* @param[out] filled structure
	* @return Error code */
int phonebook::read_entry( int index, phbook_entry& entry )
{
	if( !m_curr_book ) {
		return error::phonebook_not_selected;
	}
	char buf[32]; buf[sizeof(buf)-1] = '\0';
	fnd::tiny_snprintf( buf, sizeof(buf)-1, "+CPBR=%i", index );

	auto resp = at().chat(buf, "+CPBR:", false, true );
	if( !resp ) {
		dbg_err( "Modem error response %i", at().error() );	
		return at().error();
	}
	if( *resp == '\0' ) {
		return error::entry_not_found;
	}
	return parse_phonebook_entry( resp, entry );
}
  
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
 
/** Find phonebook entry by name return number 
	* @param[in,out] Entry for find
	* @return index or error code
	*/
int phonebook::find_entry( phbook_entry& entry )
{
	if( !m_curr_book ) {
		return error::phonebook_not_selected;
	}
	char buf[64]; buf[sizeof(buf)-1] = '\0';
	//! IF empty request return invalid argument
	if( entry.name[0] == '\0' ) {
		return error::invalid_argument;	
	}
	fnd::tiny_snprintf( buf, sizeof(buf)-1, "+CPBF=\"%s\"", entry.name );
	//! Accept empty response but not ignore errors
	auto resp = at().chat(buf, "+CPBF:", false, true );
	if( !resp ) {
		dbg_err( "Modem error response %i", at().error() );	
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
 
//! Internal version write or delete entry
int phonebook::write_or_delete_entry( int index, const phbook_entry* phb )
{
	if( !m_curr_book ) {
		return error::phonebook_not_selected;
	}
	char buf[ sizeof(phbook_entry) + 24 ]; buf[sizeof(buf)-1] = '\0';
	auto tlen = fnd::tiny_snprintf(buf, sizeof(buf)-1, "+CPBW=%i", index );
	if( phb )	//!Also entry to write
	{
		int type { number_format::unknown };
		if( std::strchr(phb->phone, '+') ) {
			type = number_format::international;
		}
		fnd::tiny_snprintf(buf+tlen, sizeof(buf)-tlen-1, ",\"%s\",%i,\"%s\"", 
				phb->phone, type, phb->name );
	}
	if( !at().chat(buf) ) {
		dbg_err( "Modem error response %i", at().error() );	
		return at().error();
	}
	return error::success;
}
 
/** Find first phonebook empty entry 
	@return error or index found
*/
int phonebook::find_empty_entry()
{
	auto resp = at().chat("+CPBS?", "+CPBS:" );
	if( !resp ) {
		dbg_err( "Modem error response %i", at().error() );	
		return at().error();
	}
	int used_entries, total_entries;
	param_parser p( resp, at().bufsize() );
	if( !p.parse_string() ) return p.error();
	if( p.parse_comma() < 0 ) return p.error();
	if( p.parse_int( used_entries ) < 0 ) return p.error();
	if( p.parse_comma() < 0 ) return p.error();
	if( p.parse_int( total_entries ) < 0 ) return p.error();
	if( used_entries == total_entries ) {
		return error::phonebook_full;
	}
	//Try to find first empty entry
	phbook_entry entry;
	for( int idx=1; idx<=total_entries; ++idx ) {
		auto ret = read_entry( idx, entry );
		if( ret == error::entry_not_found ) {
			return idx;
		} else if( ret < 0 ) {
			return ret;
		}
	}
	return error::phonebook_full;
}
  
}
  

