/*
 * =====================================================================================
 *
 *       Filename:  at_parser.hpp
 *
 *    Description:  ATParser command implementation
 *
 *        Version:  1.0
 *        Created:  22.02.2015 19:40:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <array>
#include <foundation/fixed_vector.hpp>
#include <foundation/serial_port.hpp>
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */
//! Static vector of input string
using resp_vec = fnd::fixed_vector<char*,8>;
/* ------------------------------------------------------------------ */
//At parser command class
class at_parser 
{
	//! Make it noncopyable
	at_parser& operator=( at_parser& ) = delete;
	at_parser( at_parser& ) = delete;
	static constexpr auto cmd_buffer_len = 512U;
public:
	enum ecapab : unsigned {
		cap_ommits_colon = 1
	};
	//! Constructor for serial port
	at_parser( fnd::serial_port& port, unsigned capabilities );

	//! Chat with the modem
	char* chat( const char at_cmd[]=nullptr, const char resp[]=nullptr,
			bool ignore_errors=false, bool empty_response=false );

	// Chat and get he vector of string
	resp_vec chatv( const char at_cmd[]=nullptr, bool ignore_errors = false );

	//! Remove whitespace at begginning and end of string
	char* normalize( char* input );

	//! Get parser error
	int error() const {
		return m_error;
	}
	
	//! Put line to the serial interface
	int put_line( const char* line1, const char* line2=nullptr,
			bool carriage_return = true );
private:
	//! Require ommits colon
	bool is_ommit_colon() const {
		return m_capabilities & cap_ommits_colon;	
	}
	//Match the response string
	bool match_response( const char* answer, const char* response_to_match );
private:
	fnd::serial_port& m_port;	//Serial port reference
	int m_error {};				//Error code
	char m_cmd_buffer[ cmd_buffer_len ] {};	//Command buffer
	const unsigned m_capabilities {};	//Parser capabilities
};
/* ------------------------------------------------------------------ */
}
