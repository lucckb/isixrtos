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
#include <cstddef>
#include <algorithm>
#include <cstdlib>
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
// Cut the response
char* at_parser::cut_response( char* answer, const char* response_to_match )
{
	const auto rtm_len = std::strlen( response_to_match );
	if( !std::strncmp(answer,response_to_match,rtm_len) ) {
		return normalize( answer+rtm_len );
	} else {
		if( is_ommit_colon() &&
			response_to_match[rtm_len-1] == ':' &&
			!std::strncmp(answer,response_to_match,rtm_len-1) ) {
			return normalize( answer+rtm_len );
		}
	}
	return nullptr;
}
/* ------------------------------------------------------------------ */
//! Remove whitespace at begginning and end of string
char* at_parser::normalize( char* input )
{
	size_t start = 0, end = std::strlen(input);
	bool changed { true };
	while (start < end && changed)
    {
      changed = false;
      if (isspace(input[start]))
	{
	  ++start;
	  changed = true;
	}
      else
	if (isspace(input[end - 1]))
	  {
	    --end;
	    changed = true;
	  }
    }
	input[end] = '\0';
	return &input[start];
}
/* ------------------------------------------------------------------ */
// Report and decode error
void at_parser::report_error( char* inp )
{
	if( match_response(inp, "ERROR") ){
		m_error = error::aterr_unspecified;
		return;
	}
	auto me_error = match_response( inp, "+CME ERROR:" );
	if( me_error ) 
		inp = cut_response( inp,  "+CME ERROR:" );
	else
		inp = cut_response( inp,  "+CMS ERROR:" );
	if( !inp )
		m_error = error::lib_bug;
	else
		m_error = error::at_error_first - std::atoi( inp );
}
/* ------------------------------------------------------------------ */ 
//! Chat with the modem
char* at_parser::chat( const char at_cmd[], const char resp[],
	bool ignore_errors, bool empty_response, char **pdu  )
{
	char expect[ atcmd_maxlen ];
	m_error = put_line( "AT", at_cmd );
	if( m_error ) return nullptr;
	if( std::strlen(at_cmd) >= sizeof(expect) ) {
		m_error = error::buffer_overflow;
		return nullptr;
	}
	auto pos = std::strstr( at_cmd, "=" );
	if( pos ) 
	{
		size_t rpos = pos-at_cmd;
		std::strncpy( expect, at_cmd, rpos );
		expect[rpos++] = ':';
		expect[rpos++] = ' ';
		std::strcpy( expect+rpos, pos+1 );
	} else {
		expect[0] = '\0';
	}
	char* inp {};
	do {
		m_error = m_port.gets( m_cmd_buffer, sizeof m_cmd_buffer, def_timeout );
		if( m_error ) break;
		inp = normalize( m_cmd_buffer );
	} while( std::strlen(inp)==0 || (inp[0]=='A'&&inp[1]=='T'&&!std::strcmp(inp+2,at_cmd)) ||
		    ((!resp||!match_response(inp,resp)) && 
			( expect[0] && match_response(inp,expect))));
	//! Handle errors
	if( match_response(inp,"+CME ERROR:") || match_response(inp,"+CMS ERROR:") ) {
		if( ignore_errors ) {
			m_cmd_buffer[0] = '\0';
			m_error = error::success;
			return m_cmd_buffer;	
		} else {
			report_error( inp );
			return nullptr;
		}
	}
	if( match_response( inp, "ERROR" ) ) {
		if( ignore_errors ) {
			m_cmd_buffer[0] = '\0';
			m_error = error::success;
			return m_cmd_buffer;	
		} else {
			m_error = error::aterr_unspecified;
		}
	}
	//Return empty string if response is ok and caller says ok
	if( empty_response && std::strcmp(inp,"OK") ) {
		m_cmd_buffer[0] = '\0';
		m_error = error::success;
		return m_cmd_buffer;	
	}
	//Empty sms handling
	bool got_ok {};
	//Handle PDU if it is expected
	if( pdu ) {
		char* ps {};	
		do {
			m_error = m_port.gets( m_cmd_buffer, sizeof m_cmd_buffer, def_timeout );
			if( m_error ) break;
			ps = normalize( m_cmd_buffer );
		} while( ps[0]=='\0' && std::strcmp(ps,"OK") );
		if( !std::strcmp(ps,"OK") ) {
			got_ok = true;	
		} else {
		  *pdu = ps;
		}
	}
	if( !resp ) { 	// No resp expected
		if( !std::strcmp(inp,"OK") ) {
			m_cmd_buffer[0] = '\0';
			m_error = error::success;
			return m_cmd_buffer;	
		}
		//else jump to error
	} else {
		//get final OK
		do {
			m_error = m_port.gets( m_cmd_buffer, sizeof m_cmd_buffer, def_timeout );
			if( m_error ) break;
			inp = normalize( m_cmd_buffer );

		} while( inp[0]=='\0' );
	}
}
/* ------------------------------------------------------------------ */ 
}

