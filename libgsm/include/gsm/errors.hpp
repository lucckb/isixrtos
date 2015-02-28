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
			unexpected_resp = -16387,
			receive_timeout = -16388,
			param_parser_buf_overflow = -16400,
			param_parser_unexpected_char = -16401,
			param_parser_unexpected_quote = -16402,
			param_parser_quote_not_found = -16403,
			param_parser_expected_comma = -16404,
			param_parser_expected_param = -16405,
			param_parser_expected_number = -16406,
			param_parser_unexpected_eof = -16507,
			param_parser_expected_min_and_coma = -16408,
			param_parser_range_abc_not_allowed = -16409,
			param_parser_range_a_not_allowed = -16410,
			param_parser_eol_excepted = -16411,
			param_parser_range_error = -16412,
			param_parser_container_overflow = -16413,
			at_error_first = -16500,
		};
	};
}
