/*
 * =====================================================================================
 *
 *       Filename:  at_parser.cpp
 *
 *    Description:  AT parser for gsm library
 *
 *        Version:  1.0
 *        Created:  22.02.2015 22:31:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gsm/at_parser.hpp>
#include <cstring>
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */ 
//! Put line to the serial interface
int at_parser::put_line( const char* line1, const char* line2, bool carriage_return )
{
	m_error = m_port.puts( line1 );
	if( m_error ) return m_error;
	if( line2 ) {
		m_error = m_port.puts( line2 );
		if( m_error ) return m_error;
	}
	if( carriage_return )
		m_error = m_port.puts("\r\n");
	else
		m_error = m_port.puts("\n");
	return m_error;
}
/* ------------------------------------------------------------------ */
//Match the response string
bool at_parser::match_response( const char* answer, const char* response_to_match )
{
	const auto rtm_len = std::strlen(response_to_match);
	if( !std::strncmp( answer, response_to_match, rtm_len ) ) 
	{
		return true;
	}
	else if( is_ommit_colon() &&
		response_to_match[ rtm_len-1] == ':' &&
		!std::strncmp( answer, response_to_match, rtm_len-1) ) 
	{
		return true;	
	}
	return false;
}
/* ------------------------------------------------------------------ */ 
//! Chat with the modem
char* at_parser::chat( const char at_cmd[], const char resp[],
	bool ignore_errors, bool empty_response )
{
	m_error = put_line( "AT", at_cmd );
	if( m_error ) return nullptr;
}
/* ------------------------------------------------------------------ */ 
}

