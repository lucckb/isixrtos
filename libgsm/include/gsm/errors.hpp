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
			unsupported_operation = -16389,
			//Parserr errors
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
			//At command error codes
			at_cme_first = -16500,
			at_cme_phone_failure = -16500,
			at_cme_no_connection_to_phone = -16501,
			at_cme_phone_adapter_link_reserved = -16502,
			at_cme_operation_not_allowed = -16503,
			at_cme_operation_not_supported = -16504,
			at_cme_ph_sim_pin_required = -16505,
			at_cme_ph_fsim_pin_required = -16506,
			at_cme_ph_puk_pin_required = -16507,
			at_cme_sim_not_inserted = -16510,
			at_cme_sim_pin_required = -16511,
			at_cme_sim_puk_required = -16512,
			at_cme_sim_failure = -16513,
			at_cme_sim_busy = -16514,
			at_cme_sim_wrong = -16515,
			at_cme_incorrect_password = -16516,
		};
	};
}
