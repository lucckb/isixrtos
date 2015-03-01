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
#include <foundation/dbglog.h>
#include <gsm/at_parser.hpp>
#include <gsm/gsm_event.hpp>
#include <cstring>
#include <cstddef>
#include <algorithm>
#include <cstdlib>
#include <algorithm>
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */
//! Put line to the serial interface
int at_parser::put_line( const char* line1, const char* line2, bool carriage_return )
{
	dbprintf("tx>[%s%s]", line1, line2);
	auto ret = m_port.puts( line1 );
	if( ret < 0 ) { m_error = ret; return m_error; }
	if( line2 ) {
		ret = m_port.puts( line2 );
		if( ret < 0 ) { m_error = ret; return m_error; }
	}
	if( carriage_return ) {
		ret = m_port.puts("\r\n");
		if( ret < 0 ) { m_error = ret; return m_error; }
	} else {
		ret = m_port.puts("\n");
		if( ret < 0 ) { m_error = ret; return m_error; }
	}
	return 0;
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
	if( !input ) return nullptr;
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
		m_error = error::at_cme_first - std::atoi( inp );
}
/* ------------------------------------------------------------------ */
// Get line and handle events
char* at_parser::getline( size_t pos_from )
{
	const auto begin_ptr = m_cmd_buffer+pos_from;
	int ret {};
	do {
		ret = m_port.gets( begin_ptr, sizeof(m_cmd_buffer)-pos_from, def_timeout );
		if( ret <= 0 ) 
		{ 
			m_error = !ret?error::receive_timeout:ret; 
			return nullptr; 
		}
		//! Ignored events
		if( std::strstr(begin_ptr,"^STN:") ||
			std::strstr(begin_ptr,"+SIM READY") ) 
		{
			dbprintf("ign>[%s]", begin_ptr );
			continue;
		}
		auto s = normalize(begin_ptr);
		if( match_response(s,"+CMT:") ||
			match_response(s,"+CBM:") ||
			match_response(s,"+CDS:") ||
			match_response(s,"+CMTI:") ||
			match_response(s,"+CBMI:") ||
			match_response(s,"+CDSI:") ||
			//match_response(s,"+CREG:") ||
			match_response(s,"RING") ||
			match_response(s,"NO CARRIER") ||
			(match_response(s,"+CLIP:") && std::strlen(s)>10 )) 
		{
			if( m_event )
				m_event->dispatch( s, *this );		
			continue;
		}
		break;
	} while(true);
	dbprintf("rx>[%s]->%i", begin_ptr, ret );
	return begin_ptr;
}
/* ------------------------------------------------------------------ */ 
//! Chat with the modem
char* at_parser::chat( const char at_cmd[], const char resp[],
	bool ignore_errors, bool empty_response, char **pdu  )
{
	char expect[ atcmd_maxlen ];
	if( put_line( "AT", at_cmd ) < 0 ) return nullptr;
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
		inp = normalize( getline() );
		if( !inp ) return nullptr;	
	} while( inp[0]=='\0' || (inp[0]=='A'&&inp[1]=='T'&&!std::strcmp(inp+2,at_cmd)) ||
		    ((!resp||!match_response(inp,resp)) && 
			( expect[0] && match_response(inp,expect))));
	//! Handle errors
	if( match_response(inp,"+CME ERROR:") || match_response(inp,"+CMS ERROR:") ) {
		if( ignore_errors ) {
			m_cmd_buffer[0] = '\0';
			return m_cmd_buffer;	
		} else {
			report_error( inp );
			return nullptr;
		}
	}
	if( match_response( inp, "ERROR" ) ) {
		if( ignore_errors ) {
			m_cmd_buffer[0] = '\0';
			return m_cmd_buffer;	
		} else {
			m_error = error::aterr_unspecified;
			return nullptr;
		}
	}
	//Return empty string if response is ok and caller says ok
	if( empty_response && !std::strcmp(inp,"OK") ) {
		m_cmd_buffer[0] = '\0';
		return m_cmd_buffer;	
	}
	//Empty sms handling
	bool got_ok {};
	//Handle PDU if it is expected
	if( pdu ) {
		char* ps {};	
		do {
			ps = normalize( getline() );
			if(!ps) return nullptr;
		} while( ps[0]=='\0' && !std::strcmp(ps,"OK") );
		if( !std::strcmp(ps,"OK") ) {
			got_ok = true;	
		} else {
		  *pdu = ps;
		}
	}
	if( !resp || resp[0] == '\0' ) { 	// No resp expected
		if( !std::strcmp(inp,"OK") ) {
			m_cmd_buffer[0] = '\0';
			return m_cmd_buffer;	
		}
		//else jump to error
	} else {
		char *result {};
		if( match_response( inp, resp ) ) {
			result = cut_response( inp, resp );	
		} else {
			result = inp;
		}
		if( got_ok ) {
			return result;
		} else {
			//Calculate the position for extra response just outside the buffer
			const auto result_pos = std::strlen(result)+(result-m_cmd_buffer)+sizeof('\0');
			do {
				inp = normalize( getline(result_pos) );
				if(!inp) return nullptr;
			} while( inp[0] == '\0');
			if( !strcmp(inp,"OK") ) {
				return result;
			}
		}
	}
	m_error = error::unexpected_resp;
	return nullptr;
}
/* ------------------------------------------------------------------ */ 
// Chat and get he vector of string
int at_parser::chatv( resp_vec& ans_vec, const char at_cmd[], const char response[],
		bool ignore_errors )
{
	if( put_line( "AT", at_cmd ) < 0 ) return m_error;
	char* inp;
	do {
		inp = normalize( getline() );	
		if( !inp ) return m_error;	
	} while(  inp[0]=='\0' || (inp[0]=='A'&&inp[1]=='T'&&!std::strcmp(inp+2,at_cmd)) );
	//! Handle errors
	if( match_response(inp,"+CME ERROR:") || match_response(inp,"+CMS ERROR:") ) {
		if( ignore_errors ) {
			return 0;
		} else {
			report_error( inp );
			return m_error;
		}
	}
	if( match_response( inp, "ERROR" ) ) {
		if( ignore_errors ) {
			return 0;	
		} else {
			m_error = error::aterr_unspecified;
			return m_error;
		}
	}
	size_t getln_pos {};
	while(true) {
		if( !std::strcmp(inp,"OK") ) {
			return 0;
		}
		if( inp[0] != '\0' ) {
			if( response && *response && match_response(inp,response) ) {
				const auto resp = cut_response( inp, response );
				ans_vec.push_back( resp );
				//Calculate the position for extra response just outside the buffer
				getln_pos = std::strlen(resp)+(resp-m_cmd_buffer)+sizeof('\0');
			} else {
				ans_vec.push_back( inp );
				//Calculate the position for extra response just outside the buffer
				getln_pos = std::strlen(inp)+(inp-m_cmd_buffer)+sizeof('\0');
			}
		}
		do {
			inp = normalize( getline(getln_pos) );
			if(!inp) return m_error;
		} while( inp[0] == '0' );
	}
	m_error = error::lib_bug;
	return m_error;
}
/* ------------------------------------------------------------------ */
//! Discard bytes and don't wait for line response
int at_parser::discard_data( int timeout )
{
	int ret {};
	if( timeout != time_infinite ) {
		ret = m_port.get( m_cmd_buffer, std::min<size_t>(sizeof m_cmd_buffer,ret),1000 );
		if( ret <=0 ) return ret;
	}
	while( true ) 
	{
		ret = m_port.rx_avail();
		if( ret <= 0 ) break;
		ret = m_port.get( m_cmd_buffer, std::min<size_t>(sizeof m_cmd_buffer,ret),1000 );
		if( ret < 0 ) break;
	}
	m_cmd_buffer[0] = '\0';
	return ret;
}
/* ------------------------------------------------------------------ */ 
}

