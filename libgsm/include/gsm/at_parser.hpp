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
/* ------------------------------------------------------------------ */ 
namespace fnd {

	class serial_port;
}
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */
//At p arser command class
class at_parser 
{
	//! Make it noncopyable
	at_parser& operator=( at_parser& ) = delete;
	at_parser( at_parser& ) = delete;
public:
	//! Constructor for serial port
	at_parser( fnd::serial_port& port );
private:
	fnd::serial_port& m_port;
};
/* ------------------------------------------------------------------ */
}
