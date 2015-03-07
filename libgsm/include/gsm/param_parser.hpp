#pragma once

#include <cstddef>
#include <bitset>
#include <gsm/errors.hpp>
#include <gsm/containers.hpp>

namespace gsm_modem
{
	
	using bit_range = std::bitset<256>;
	
	// indicate that a value is not set
	static constexpr int NOT_SET = -1;

	// An integer range
	struct int_range
	{
		int high, low;
		int_range() : high(NOT_SET), low(NOT_SET) {}
	};

	// A valid integer range for a given parameter
	struct parameter_range
	{
		const char* parameter;
		int_range range;
	};

	class param_parser
	{
		public:
			using ret_str_t = const char*;
		private:
			char * const m_buf {};
			char *m_pos {};
			const char* const m_eob {};
			int m_error {};
			const char* m_comma_pos {};

			//Check if pointer is in range
			bool good() const {

				return ( (m_pos)>= m_buf && (m_pos) < m_eob );
			}
			bool bad() {
				if( !good() ) {
					m_error = error::param_parser_buf_overflow;
					return true;
				}
				return false;
			}
			//Skip space
			void seek_eol() {
				for( ;good() && *m_pos; ++m_pos );
			}
			void skip_space() {
				if( good() && *m_pos && *m_pos==' ' ) {
					++m_pos;	
				}
			}

			void skip_all_spaces() {
				if( good() && *m_pos && *m_pos==' ' ) {
					++m_pos;	
				}
			}

			// check for empty parameter (ie. "," or end of string)
			// skips white space
			// returns true if no parameter
			// or throw an GsmException if allow_no_parameter == false
			int check_empty_parameter(bool allow_no_parameter); 

			// parse a std::string (like "string")
			// throw an exception if not well-formed
			ret_str_t do_parse_string(bool string_with_quotation_marks);

			// parse a int (like 1234)
			// throw an exception if not well-formed
			int do_parse_int( int& val );

		public:

			param_parser(char* s, size_t len )
				: m_buf(s), m_pos(s), m_eob( s+len )
			{
			}
			// the following functions skip white space
			// parse a character, if absent throw a GsmException
			// return false if allowNoChar == true and character not encountered
			int parse_char(char c, bool allow_no_char = false) ;

			// parse a list of the form "("ABC", DEF")"
			// the list can be empty (ie. == "" ) if allow_no_list == true
			int parse_string_list( vector<ret_str_t>& result, bool allow_no_list = false);

			// parse a list of the form "(12, 14)" or "(1-4, 10)"
			// the result is returned as a bit vector where for each integer
			// in the list and/or range(s) a bit is set
			// the list can be empty (ie. == "") if allow_no_list == true
			int parse_int_list( bit_range& result, bool allow_no_list = false);

			// parse a list of parameter ranges (see below)
			// the list can be empty (ie. == "" ) if allow_no_list == true
			int parse_parameter_range_list(vector<parameter_range>& result, bool allow_no_list = false);

			// parse a std::string plus its valid integer range of the
			// form "("string",(1-125))"
			// the parameter range may be absent if allowNoparameter_range == true
			int  parse_parameter_range(parameter_range& result, bool allow_no_parameter_range = false);

			// parse an integer range of the form "(1-125)"
			// the range may be absent if allow_no_range == true
			// then int_range::_high and _low are set to NOT_SET
			// the range may be short if allow_non_range == true
			// then int_range::_high is set to NOT_SET
			int parse_range(int_range& result, bool allow_no_range = false, bool allow_non_range = false);

			// parse an integer of the form "1234"
			// allow absent int if allow_no_int == true
			// then it returns NOT_SET
			int parse_int(int& val, bool allow_no_int = false) ;

			// parse a std::string of the form ""string""
			// allow absent std::string if allow_no_string == true
			// then it returns ""
			// if string_with_quotation_marks == true the std::string may contain """
			// the std::string is then parsed till the end of the line
			ret_str_t parse_string(bool allow_no_string = false,
					bool string_with_quotation_marks = false);

			// parse a single ","
			// the comma may be absent if allow_no_comma == true
			// returns true if there was a comma
			int parse_comma(bool allow_no_comma = false);

			// parse till end of line, return result without whitespace
			ret_str_t parse_eol();

			// check that end of line is reached
			int check_eol();

			// return std::string till end of line without whitespace
			// (does not change internal state)
			ret_str_t get_eol();

			//Get the constant pointer for error pos
			std::size_t pos() const {
				return m_pos - m_buf;
			}
			//! Return the object error state
			int error() const {
				return m_error;
			}
			//! Set error status
			void error( int err ) {
				m_error = err;
			}
	};
};

