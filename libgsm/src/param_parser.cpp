
#include "gsm_parser.h"
#include <cctype>
#include <cassert>
#include <stdexcept>

using namespace gsmlib;

#define stringPrintf(...) 
#define throwParseException(x)  throw std::logic_error( x )
//#define throwParseException(x) assert(0)
#define _(x) x


int parser::parse_char(char c, bool allow_no_char) 
{
	skip_space();
	if( m_pos >= m_eob ) {
		m_error = error::parser_buf_overflow;
		return m_error;
	}
	if( *m_pos++ != c )
	{
		if (allow_no_char)
		{
			--m_pos;
			return false;
		}
		else
		{
			m_error = error::parser_unexpected_char;
			//throwParseException(stringPrintf(_("expected '%c'"), c));
			throwParseException( "excepted");
			return m_error;
		}
	}
	return true;
}

int parser::check_empty_parameter(bool allow_no_parameter) 
{
	skip_space();
	if (*m_pos == ',' || *m_pos == '\0' )
	{
		if (allow_no_parameter)
		{
			//putBackChar();
			return true;
		}
		else {
			m_error = error::parser_expected_param;
			throwParseException(_("expected parameter"));
			return m_error;
		}

	}
	//putBackChar();
	return false;
}

char* parser::do_parse_string(bool string_with_quotation_marks)
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
				m_error = error::parser_unexpected_quote;
				throwParseException(_("expected '\"'"));
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
			for( ;m_pos<m_eob && *m_pos && *m_pos!='"'; ++m_pos ) {}
			if( *m_pos != '"') {
				m_error = error::parser_quote_not_found;	
				throwParseException( "Quote not found" );
				return nullptr;
			} else {
				*m_pos++ = '\0';
			}
		}
	else                          // std::string ends with "," or EOL
	{
		beg = m_pos;
		for( ;m_pos<m_eob && *m_pos && *m_pos!=','; ++m_pos );
		if( *m_pos == ',' ) {
			m_comma_pos = m_pos;
			*m_pos = '\0';
		}
	}
	return beg;
}

char* parser::parse_string(bool allow_no_string,
		bool string_with_quotation_marks)

{
	// handle case of empty parameter
	if (check_empty_parameter(allow_no_string)) return m_pos;

	auto result = do_parse_string(string_with_quotation_marks);

	return result;
}

int parser::do_parse_int( int &val )
{
	skip_space();
	if( m_pos >= m_eob ) {
		m_error = error::parser_buf_overflow;
		return m_error;
	}
	char *eptr;
	val = std::strtol( m_pos, &eptr, 10 );
	if( eptr == m_pos ) {
		m_error = error::parser_expected_number;	
		throwParseException(_("expected number"));
		return m_error;
	} else {
		m_pos = eptr;
	}
	return 0;
}


parser::parser(char* s, size_t len )
: m_buf(s), m_pos(s), m_eob( s+len )
{
}


int parser::parse_string_list(vector<char*>& result, bool allow_no_list)

{
	// handle case of empty parameter
	if (check_empty_parameter(allow_no_list)) return m_error;

	if( parse_char('(') < 0 ) {
		m_error = error::parser_unexpected_char;
		return m_error;
	}
	if (*m_pos && *m_pos != ')')
	{
		//putBackChar();
		while (1)
		{
			result.push_back(parse_string());
			int c = *m_pos++;
			if (c == ')')
				break;
			if (c == 0)
				throwParseException("c=-1");
			if (c != ',')
				throwParseException(_("expected ')' or ','"));
		}
	}
	return 0;
}

//TODO error value
int parser::parse_int_list(vector<bool>& result, bool allow_no_list)

{
	// handle case of empty parameter
	bool isRange = false;
	int resultCapacity = 0;
	const auto saveI = m_pos;

	if( m_pos >= m_eob ) {
		m_error = error::parser_buf_overflow;
		throwParseException("Test 1");
		return m_error;
	}
	if (check_empty_parameter(allow_no_list)) return m_error;

	// check for the case of a integer list consisting of only one parameter
	// some TAs omit the parentheses in this case
	skip_space();
	if (isdigit(*m_pos))
	{
		int num; 
		if( parse_int(num) ) {
			throwParseException("ex-dupa");
			return m_error;
		}
		result.resize(num + 1, false);
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
			result.resize(resultCapacity + 1, false);
		}

		if( parse_char('(') < 0 ) 
		{
			throwParseException("test2");
			return m_error;	//Got err
		}
		if ( *m_pos != ')')
		{
			//putBackChar();
			int lastInt = -1;
			while (1)
			{
				int thisInt;
				if( parse_int(thisInt) < 0 ) {
					throwParseException("dupa2");
					return m_error;
				}
				if (isRange)
				{
					assert(lastInt != -1);
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
					m_error = error::parser_unexpected_eof;
					throwParseException("c=-1");
					return m_error;
				}

				if (c != ',' && c != '-') {
					m_error = error::parser_expected_min_and_coma;
					throwParseException(_("expected ')', ',' or '-'"));
					return m_error;
				}

				if (c == ',')
					isRange = false;
				else                      // is '-'
					if (isRange) {
						m_error = error::parser_range_abc_not_allowed;
						throwParseException(_("range of the form a-b-c not allowed"));
						return m_error;
					}
					else
						isRange = true;
			}
		}
	}
	if (isRange) {
		m_error = error::parser_range_a_not_allowed;
		throwParseException(_("range of the form a- no allowed"));
		return m_error;
	}
	return 0;
}

int parser::parse_parameter_range_list(vector<parameter_range>& result, bool allow_no_list)

{
	// handle case of empty parameter
	if (check_empty_parameter(allow_no_list)) return m_error;
	
	parameter_range item;
	if( parse_parameter_range(item) < 0 ) {
		return m_error;
	}
	result.push_back(item);
	while (parse_comma(true))
	{
		if( parse_parameter_range(item) < 0 ) {
			return m_error;
		}
		result.push_back(item);
	}

	return m_error;
}

int parser::parse_parameter_range(parameter_range& result, bool allow_no_parameter_range)

{
	// handle case of empty parameter
	if (check_empty_parameter(allow_no_parameter_range)) return m_error;

	if( parse_char('(') < 0 ) {
		m_error = error::parser_unexpected_char;
		throwParseException("Unexcepted character");
	}
	result._parameter = parse_string();
	parse_comma();
	//result._range = parse_range(false, true);
	if( parse_range(result._range, false, true) < 0 ) {
		return m_error;	
	}
	if( parse_char(')') < 0 ) {
		m_error = error::parser_unexpected_char;
		throwParseException("Unexcepted character");
	}

	return m_error;
}

int parser::parse_range(int_range& result, bool allow_no_range, bool allow_non_range)

{
	// handle case of empty parameter
	if (check_empty_parameter(allow_no_range)) return m_error;

	if( parse_char('(') < 0 ) {
		m_error = error::parser_unexpected_char;
		throwParseException("Unexcepted character");
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
		m_error = error::parser_range_error;
		throwParseException("Range error");
		return m_error;
	}
	return m_error;
}

int parser::parse_int(int& result, bool allow_no_int) 
{
	// handle case of empty parameter
	result = NOT_SET;
	if (check_empty_parameter(allow_no_int)) return m_error;

	return do_parse_int(result);
}


int parser::parse_comma(bool allow_no_comma) 
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
	if( m_pos >= m_eob ) {
		m_error = error::parser_buf_overflow;
		return m_error;
	}
	if (*m_pos++ != ',')
	{
		if(allow_no_comma)
		{
			//putBackChar();
			return false;
		}
		else 
		{
			m_error = error::parser_expected_comma;
			throwParseException(_("expected comma"));
			return m_error;
		}
	}
	return true;
}

//TODO: It is inplace operation so internal spaces are not removed
char* parser::parse_eol() 
{
	skip_all_spaces();
	if( m_pos >= m_eob ) {
		m_error = error::parser_buf_overflow;
		return nullptr;
	}
	const auto beg = m_pos;
	seek_eol();
	*m_pos = '\0';

	return beg;
}

int parser::check_eol() 
{
	if( *m_pos == '\0' ) {
		return 0;
	} else {
		m_error = error::parser_eol_excepted;
		throwParseException(_("expected end of line"));
	}
	return m_error;
}

char* parser::get_eol()
{
	skip_all_spaces();
	if( m_pos >= m_eob ) {
		m_error = error::parser_buf_overflow;
		return nullptr;
	}
	return m_pos;
}
