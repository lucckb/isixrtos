/*
 * =====================================================================================
 *
 *       Filename:  gsm_event.hpp
 *
 *    Description:  GSM event dispatcher class
 *
 *        Version:  1.0
 *        Created:  25.02.2015 19:51:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
//TODO: Implement this one
namespace gsm_modem {
	class at_parser;
	class event {
	public:
		void dispatch( const char *, at_parser&  ) {}
	};
}
