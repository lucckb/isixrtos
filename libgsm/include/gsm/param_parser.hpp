#pragma once

#include <string>
#include <cstring>
#include <vector>


namespace gsmlib
{
	//Temporary
	struct error {
		enum error_ {
			parser_buf_overflow = -1,
			parser_unexpected_char = -2,
			parser_unexpected_quote = -3,
			parser_quote_not_found = -4,
			parser_expected_comma = -5,
			parser_expected_param = -6,
			parser_expected_number = -7,
			parser_unexpected_eof = -8,
			parser_expected_min_and_coma = -9,
			parser_range_abc_not_allowed = -10,
			parser_range_a_not_allowed = -11,
			parser_eol_excepted = -12,
			parser_range_error = -14,
		};
	};
  template<typename T>
  using vector = std::vector<T>;

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
    std::string _parameter;
    int_range _range;
  };

  class parser
  {
  private:
	  char * const m_buf {};
	  char *m_pos {};
	  const char* const m_eob {};
	  int m_error {};
	  char* m_comma_pos {};

	//Check if pointer is in range
	bool good() const {
		
		return ( (m_pos)>= m_buf && (m_pos) < m_eob );
	}
	bool bad() {
		if( !good() ) {
			m_error = error::parser_buf_overflow;
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
    char* do_parse_string(bool string_with_quotation_marks);

    // parse a int (like 1234)
    // throw an exception if not well-formed
    int do_parse_int( int& val );

  public:
	int error() const {
		return m_error;
	}

	//For test only
	parser( const std::string& s ) 
		: parser( new char[s.length()+1], s.length()+1 )
	{
		std::strcpy( m_buf, s.c_str() );
	}

	parser(char* s, size_t len )
	: m_buf(s), m_pos(s), m_eob( s+len )
	{
	}
    // the following functions skip white space
    // parse a character, if absent throw a GsmException
    // return false if allowNoChar == true and character not encountered
    int parse_char(char c, bool allow_no_char = false) ;

    // parse a list of the form "("ABC", DEF")"
    // the list can be empty (ie. == "" ) if allow_no_list == true
    int parse_string_list( vector<char*>& result, bool allow_no_list = false);

    // parse a list of the form "(12, 14)" or "(1-4, 10)"
    // the result is returned as a bit vector where for each integer
    // in the list and/or range(s) a bit is set
    // the list can be empty (ie. == "") if allow_no_list == true
    int parse_int_list( vector<bool>& result, bool allow_no_list = false);

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
    char* parse_string(bool allow_no_string = false,
                       bool string_with_quotation_marks = false);

    // parse a single ","
    // the comma may be absent if allow_no_comma == true
    // returns true if there was a comma
	int parse_comma(bool allow_no_comma = false);

    // parse till end of line, return result without whitespace
    char* parse_eol();

    // check that end of line is reached
    int check_eol();

    // return std::string till end of line without whitespace
    // (does not change internal state)
    char* get_eol();
  };
};

