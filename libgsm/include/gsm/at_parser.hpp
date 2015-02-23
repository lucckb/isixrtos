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
/* ------------------------------------------------------------------ */ 
namespace fnd {

	class serial_port;
}
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
public:
	//! Constructor for serial port
	at_parser( fnd::serial_port& port );

	//! Chat with the modem
	char* chat( char inp[], const char at_cmd[]=nullptr, const char resp[]=nullptr,
			bool ignore_errors=false, bool empty_response=false );

	// Chat and get he vector of string
	resp_vec chatv( char inp[], const char at_cmd[]=nullptr, bool ignore_errors = false );

	//! Remove whitespace at begginning and end of string
	char* normalize( char* input );

	//! Get parser error
	int error() const {
		return m_error;
	}

private:
	fnd::serial_port& m_port;
	int m_error {};
};
/* ------------------------------------------------------------------ */
}
