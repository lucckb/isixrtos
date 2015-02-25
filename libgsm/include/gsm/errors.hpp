/*
 * =====================================================================================
 *
 *       Filename:  errors.hpp
 *
 *    Description:  Errors definition for at parser
 *
 *        Version:  1.0
 *        Created:  24.02.2015 21:19:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

namespace gsm_modem {
	struct error {
		enum error_ {
			success = 0,
			buffer_overflow = -16384, 
			aterr_unspecified =-16385,
			lib_bug = -16386,
			at_error_first = 16387,
		};
	};
}
