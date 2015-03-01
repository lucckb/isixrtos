/*
 * =====================================================================================
 *
 *       Filename:  phbook_id.cpp
 *
 *    Description:  Phone book identifier implementation
 *
 *        Version:  1.0
 *        Created:  01.03.2015 21:49:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <gsm/phbook_id.hpp>
#include <cstddef>
#include <cstring>
/* ------------------------------------------------------------------ */
namespace {
	static constexpr const char* const bookids[] = {
		"SM", "FD", "ON", "EN", "LD", "MC", "ME", "MT", "RC", "SN", nullptr 
	};
}
/* ------------------------------------------------------------------ */ 
namespace gsm_modem {
/* ------------------------------------------------------------------ */ 
//! Set bit by name
void phbook_id::set_bit( const char* name ) {

	for( size_t i=0; bookids[i]; ++i ) 
	{
		if( !std::strcmp(name, bookids[i]) )
			m_book_bits |= 1 << i;
	}
}
/* ------------------------------------------------------------------ */ 
//! Get phone book by name
const char* phbook_id::get_name() const 
{
	const auto idx = __builtin_ffs( m_book_bits );
	const auto popc = __builtin_popcount( m_book_bits );
	return( idx<1 || idx>last_bit_no+1 || popc!=1 )?nullptr:bookids[idx-1];
}
/* ------------------------------------------------------------------ */ 
}

