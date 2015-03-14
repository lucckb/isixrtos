#include <gsm/param_parser.hpp>
#include <cctype>
#include <cstdlib>

namespace gsm_modem {

int param_parser::parse_char(char c, bool allow_no_char) 
{
	skip_space();
	if( bad() ) return m_error;
	if( *m_pos++ != c )
	{
		if (allow_no_char)
		{
			--m_pos;
			return false;
		}
		else
		{
			m_error = error::param_parser_unexpected_char;
			return m_error;
		}
	}
	return true;
}

int param_parser::check_empty_parameter(bool allow_no_parameter) 
{
	skip_space();
	if( bad() ) return m_error;
	if (*m_pos == ',' || *m_pos == '\0' )
	{
		if (allow_no_parameter)
		{
			return true;
		}
		else {
			m_error = error::param_parser_expected_param;
			return m_error;
		}

	}
	return false;
}

param_parser::ret_str_t param_parser::do_parse_string(bool string_with_quotation_marks)
{

	char* beg {};
	int ret = parse_char('"', true);
	if( ret < 0 ) return nullptr;
	if ( ret )  // OK, std::string starts and ends with quotation mark
		if (string_with_quotation_marks)
		{
			if( *m_pos ) {
				beg = m_pos;
			}
			seek_eol();

			// check for """ at end of line
			if ( beg==m_pos || *(m_pos-1)  != '"')
			{
				m_error = error::param_parser_unexpected_quote;
				return nullptr;
			}

			// remove """ at the end
			//result.resize(result.length() - 1);
			*(--m_pos) = '\0';
		}
		else
		{
			// read till next """
			beg = m_pos;
			for( ; good() && *m_pos && *m_pos!='"'; ++m_pos ) {}
			if( bad() ) return nullptr;
			if( *m_pos != '"') {
				m_error = error::param_parser_quote_not_found;	
				return nullptr;
			} else {
				*m_pos++ = '\0';
			}
		}
	else                          // std::string ends with "," or EOL
	{
		beg = m_pos;
		for( ;good() && *m_pos && *m_pos!=','; ++m_pos ) {}
		if( bad() ) return nullptr;
		if( *m_pos == ',' ) {
			m_comma_pos = m_pos;
			*m_pos = '\0';
		}
	}
	return beg;
}

param_parser::ret_str_t param_parser::parse_string(bool allow_no_string,
		bool string_with_quotation_marks)

{
	// handle case of empty parameter
	auto ret = check_empty_parameter(allow_no_string);
	if (ret == true ) {
		*m_pos = '\0';
		m_comma_pos = m_pos;
		return m_pos;
	} else if( ret < 0 ) {
		return nullptr;
	}
	auto result = do_parse_string(string_with_quotation_marks);

	return result;
}

int param_parser::do_parse_int( int &val )
{
	skip_space();
	if( bad() ) return m_error;
	char *eptr;
	val = std::strtol( m_pos, &eptr, 10 );
	if( eptr == m_pos ) {
		m_error = error::param_parser_expected_number;	
		return m_error;
	} else {
		//! Temporary to last char
		if( eptr >= m_eob ) m_pos = const_cast<char*>(m_eob)-1;
		else m_pos = eptr;
	}
	return bad()?m_error:0;
}




int param_parser::parse_string_list(vector<param_parser::ret_str_t>& result, bool allow_no_list)

{
	// handle case of empty parameter
	{
		auto ret = check_empty_parameter(allow_no_list);
		if (ret==true) return error::success;
		else if(ret<0) return m_error;
	}

	if( parse_char('(') < 0 ) {
		m_error = error::param_parser_unexpected_char;
		return m_error;
	}
	if(bad()) return m_error;
	if (*m_pos && *m_pos != ')')
	{
		//putBackChar();
		while (good())
		{
			result.push_back(parse_string());
			int c = *m_pos++;
			if (c == ')')
				break;
			if (c == 0) {
				m_error = error::param_parser_unexpected_eof;
				return m_error;
			}
			if (c != ',') {
				m_error = error::param_parser_expected_comma;
				return m_error;
			}
		}
	}
	return bad()?m_error:0;
}

int param_parser::parse_int_list(bit_range& result, bool allow_no_list)

{
	// handle case of empty parameter
	bool isRange = false;
	int resultCapacity = 0;
	const auto saveI = m_pos;
	if(bad()) { 
		return m_error;
	}
	{
		auto ret = check_empty_parameter(allow_no_list);
		if (ret==true) return error::success;
		else if(ret<0) return m_error;
	}
	// check for the case of a integer list consisting of only one parameter
	// some TAs omit the parentheses in this case
	skip_space();
	if(bad()) return m_error;
	if (isdigit(*m_pos))
	{
		int num; 
		if( parse_int(num) ) {
			return m_error;
		}
		//result.resize(num + 1, false);
		if( num > int(result.size()) ) {
			m_error = error::param_parser_container_overflow;
			return m_error;
}
		result[num] = true;
		return 0;
	}

	// run in two passes
	// pass 0: find capacity needed for result
	// pass 1: resize result and fill it in
	for (int pass = 0; pass < 2; ++pass)
	{
		if (pass == 1)
		{
			m_pos = saveI;
			//result.resize(resultCapacity + 1, false);
			if( resultCapacity > int(result.size()) ) {
				m_error = error::param_parser_container_overflow;
				return m_error;
			}
		}

		if( parse_char('(') < 0 ) 
		{
			return m_error;	//Got err
		}
		if(bad()) return m_error;
		if ( *m_pos != ')')
		{
			//putBackChar();
			int lastInt = -1;
			while (good())
			{
				int thisInt;
				if( parse_int(thisInt) < 0 ) {
					return m_error;
				}
				if (isRange)
				{
					if (lastInt <= thisInt)
						for (int i = lastInt; i < thisInt; ++i)
						{
							if (i > resultCapacity)
								resultCapacity = i;
							if (pass == 1)
								result[i] = true;
						}
					else
						for (int i = thisInt; i < lastInt; ++i)
						{
							if (i > resultCapacity)
								resultCapacity = i;
							if (pass == 1)
								result[i] = true;
						}
					isRange = false;
				}

				if (thisInt > resultCapacity)
					resultCapacity = thisInt;
				if (pass == 1)
					result[thisInt] = true;
				lastInt = thisInt;

				skip_space();
				int c = *m_pos++;
				if (c == ')')
					break;

				if (c == 0) {
					m_error = error::param_parser_unexpected_eof;
					return m_error;
				}

				if (c != ',' && c != '-') {
					m_error = error::param_parser_expected_min_and_coma;
					return m_error;
				}

				if (c == ',')
					isRange = false;
				else                      // is '-'
					if (isRange) {
						m_error = error::param_parser_range_abc_not_allowed;
						return m_error;
					}
					else
						isRange = true;
			}
		}
	}
	if (isRange) {
		m_error = error::param_parser_range_a_not_allowed;
		return m_error;
	}
	return bad()?m_error:0;
}

int param_parser::parse_parameter_range_list(vector<parameter_range>& result, bool allow_no_list)

{
	// handle case of empty parameter
	{
		auto ret = check_empty_parameter(allow_no_list);
		if (ret==true) return error::success;
		else if(ret<0) return m_error;
    }	
	parameter_range item;
	if( parse_parameter_range(item) < 0 ) {
		return m_error;
	}
	result.push_back(item);
	while (parse_comma(true)>0)
	{
		if( parse_parameter_range(item) < 0 ) {
			return m_error;
		}
		result.push_back(item);
	}

	return m_error;
}

int param_parser::parse_parameter_range(parameter_range& result, bool allow_no_parameter_range)

{
	// handle case of empty parameter
	{
		auto ret = check_empty_parameter(allow_no_parameter_range);
		if (ret==true) return error::success;
		else if(ret<0) return m_error;
	}
	if( parse_char('(') < 0 ) {
		return m_error;
	}
	result.parameter = parse_string();
	parse_comma();
	//result._range = parse_range(false, true);
	if( parse_range(result.range, false, true) < 0 ) {
		return m_error;	
	}
	if( parse_char(')') < 0 ) {
		return m_error;
	}

	return m_error;
}

int param_parser::parse_range(int_range& result, bool allow_no_range, bool allow_non_range)

{
	// handle case of empty parameter
	{
		auto ret = check_empty_parameter(allow_no_range);
		if (ret==true) return error::success;
		else if(ret<0) return m_error;
	}
	if( parse_char('(') < 0 ) {
		return m_error;
	}
	if( parse_int( result.low ) < 0 ) {
		return m_error;
	}
	// allow non-ranges is allow_non_range == true
	auto ret = parse_char('-', allow_non_range);
	if(ret<0) return ret;
	if (ret)
	{
		if( parse_int(result.high) < 0 ) {
			return m_error;
		}
	}
	if( parse_char(')') < 0 ) {
		m_error = error::param_parser_range_error;
		return m_error;
	}
	return m_error;
}

int param_parser::parse_int(int& result, bool allow_no_int) 
{
	// handle case of empty parameter
	result = NOT_SET;
	auto ret = check_empty_parameter(allow_no_int);
	if (ret==true) return error::success;
	else if(ret<0) return m_error;
	return do_parse_int(result);
}


int param_parser::parse_comma(bool allow_no_comma) 
{
	if( m_comma_pos ) {
		m_comma_pos = nullptr;
		++m_pos;
		if(allow_no_comma) {
			return false;
		} else {
			return true;
		}
	}
	skip_space();
	if(bad()) return m_error;
	if (*m_pos++ != ',')
	{
		if(allow_no_comma)
		{
			//putBackChar();
			return false;
		}
		else 
		{
			m_error = error::param_parser_expected_comma;
			return m_error;
		}
	}
	return true;
}

//TODO: It is inplace operation so internal spaces are not removed
param_parser::ret_str_t param_parser::parse_eol() 
{
	skip_all_spaces();
	if(bad()) return nullptr;
	const auto beg = m_pos;
	seek_eol();
	if(bad()) return nullptr;
	*m_pos = '\0';
	return beg;
}

int param_parser::check_eol() 
{
	if( *m_pos == '\0' ) {
		return 0;
	} else {
		m_error = error::param_parser_eol_excepted;
		return m_error;
	}
	return m_error;
}

param_parser::ret_str_t param_parser::get_eol()
{
	skip_all_spaces();
	if( m_pos >= m_eob ) {
		m_error = error::param_parser_buf_overflow;
		return nullptr;
	}
	return m_pos;
}

//Namespace end
}
