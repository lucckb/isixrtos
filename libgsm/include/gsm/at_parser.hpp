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
  
namespace gsm_modem {
 
//! Static vector of input string
using resp_vec = fnd::fixed_vector<char*,8>;
//Fwd decl
class event;
 
//At parser command class
class at_parser 
{
	//! Make it noncopyable
	at_parser& operator=( at_parser& ) = delete;
	at_parser( at_parser& ) = delete;
	static constexpr auto cmd_buffer_len = 1024U;
	static constexpr auto atcmd_maxlen = 64U;
	static constexpr auto def_timeout = 15000;
	static constexpr char CR = 13;             // ASCII carriage return
	static constexpr char LF = 10;             // ASCII line feed
public:
	static constexpr auto time_infinite = fnd::serial_port::time_infinite;
	enum ecapab : unsigned {
		cap_ommits_colon = 1
	};
	//! Constructor for serial port
	explicit at_parser( fnd::serial_port& port, unsigned capabilities = 0 )
		: m_port( port ), m_capabilities( capabilities )
	{}

	//! Chat with the modem
	char* chat( const char at_cmd[]="", const char resp[]=nullptr,
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

	//! Set event handler for the parser
	void set_event_handler( event* ev ) {
		m_event = ev;
	}
	//! Wait for an event
	int wait( int timeout );

	//! Get second line just after the response
	char* get_second_line( const char* first_ln, int timeout = def_timeout );

	//Get serial port device
	int flow_control( bool en ) {
		return m_port.set_flow( en?fnd::serial_port::flow_rtscts:fnd::serial_port::flow_none );
	}

	//Sleep for amount of time
	void sleep( unsigned msec ) {
		m_port.sleep( msec );
	}
	
	//! If acknowledge is excepted
	void ack_excepted() {
		++m_ack_excepted_count;
	}

	//! Enable or disable unsolicited creg
	void unsolicited_creg( bool en ) {
		m_unsolicited_creg = en;
	}

	//! Get unsolicited creg status
	bool unsolicited_creg() const {
		return m_unsolicited_creg;
	}

	//! Switch to command mode if DSR/DTR not set ignore
	int hw_command_mode();
	
	//! Return true if in data mode
	bool in_data_mode() const {
		 return !(m_port.tiocm_get()&fnd::serial_port::tiocm_dcd) ;
	}

private:
	//! Put line to the serial interface
	int put_line( const char* line1, const char* line2=nullptr,
			bool carriage_return = true );
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
	char* getline( size_t pos_from = 0U, int timeout = def_timeout );
	// Get single character
	int getchar( char& ch, int timeout = def_timeout );
	// Handle unsolicited
	bool handle_unsolicited( char* begin_ptr );
private:
	fnd::serial_port& m_port;	//Serial port reference
	int m_error {};				//Error code
	char m_cmd_buffer[ cmd_buffer_len ] {};	//Command buffer
	short m_ack_excepted_count {};			//ACK excepted count
	bool m_unsolicited_creg {};				//Enable creg parsing
	const unsigned m_capabilities {};	    //Parser capabilities
	event* m_event {};
};
 
}
