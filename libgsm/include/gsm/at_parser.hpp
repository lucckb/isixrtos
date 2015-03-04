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
#include "errors.hpp"
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */
//! Static vector of input string
using resp_vec = fnd::fixed_vector<char*,8>;
//Fwd decl
class event;
/* ------------------------------------------------------------------ */
//At parser command class
class at_parser 
{
	//! Make it noncopyable
	at_parser& operator=( at_parser& ) = delete;
	at_parser( at_parser& ) = delete;
	static constexpr auto cmd_buffer_len = 512U;
	static constexpr auto atcmd_maxlen = 64U;
	static constexpr auto def_timeout = 5000;
	static constexpr auto time_infinite = fnd::serial_port::time_infinite;
	static constexpr char CR = 13;             // ASCII carriage return
	static constexpr char LF = 10;             // ASCII line feed
public:
	enum ecapab : unsigned {
		cap_ommits_colon = 1
	};
	//! Constructor for serial port
	explicit at_parser( fnd::serial_port& port, unsigned capabilities = 0 )
		: m_port( port ), m_capabilities( capabilities )
	{}

	//! Chat with the modem
	char* chat( const char at_cmd[]=nullptr, const char resp[]=nullptr,
			bool ignore_errors=false, bool empty_response=false, char** pdu = nullptr );

	// Chat and get he vector of string
	int chatv( resp_vec& ans_vec, const char at_cmd[]=nullptr, 
			const char response[]=nullptr, bool ignore_errors = false );

	//! Discard bytes and don't wait for line response
	int discard_data( int timeout = time_infinite );

	 // send pdu (wait for <CR><LF><greater_than><space> and send <CTRL-Z>
    // at the end
    // return text after response
	char* send_pdu( const char at_cmd[], const char resp[],
			const char pdu[], bool accept_empty_response=false );

	//! Get parser error
	int error() const {
		return m_error;
	}
	//! Get buffer size 
	size_t bufsize() const {
		return cmd_buffer_len;
	}
	//! Put line to the serial interface
	int put_line( const char* line1, const char* line2=nullptr,
			bool carriage_return = true );
	//! Set event handler for the parser
	void set_event_handler( event* ev ) {
		m_event = ev;
	}
private:
	//! Remove whitespace at begginning and end of string
	char* normalize( char* input );

	//! Require ommits colon
	bool is_ommit_colon() const {
		return m_capabilities & cap_ommits_colon;	
	}
	//Match the response string
	bool match_response( const char* answer, const char* response_to_match );
	// Report and decode error
	void report_error( char* inp );
	// Cut the response
	char* cut_response( char* answer, const char* response_to_match );
	// Get line and handle events
	char* getline( size_t pos_from = 0U);
	// Get some data and don't wait for cr/lf
	int getsome( size_t from_pos );
private:
	fnd::serial_port& m_port;	//Serial port reference
	int m_error {};				//Error code
	char m_cmd_buffer[ cmd_buffer_len ] {};	//Command buffer
	const unsigned m_capabilities {};	//Parser capabilities
	event* m_event {};
};
/* ------------------------------------------------------------------ */
}
