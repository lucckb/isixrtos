/*
 * =====================================================================================
 *
 *       Filename:  stringbit_id.hpp
 *
 *    Description:  String bit template class
 *
 *        Version:  1.0
 *        Created:  05.03.2015 12:30:48
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <cstring>

namespace gsm_modem {
namespace detail {

		//! String bit for string names
		class stringbit_id {
		public:
			stringbit_id( unsigned bb = 0 ) 
			: m_bits( bb )
			{}
			void clear() {
				m_bits = 0;
			}
			//Mask bits
			void mask( unsigned bits ) {
				m_bits &= bits;
			}
			//Get all bits
			unsigned bits() const {
				return m_bits;
			}
		protected:
			//Get name internal
			const char* get_name(const char* const nmtbl[], 
					int last_bit, unsigned limit=0) const {
				unsigned tbit;
				if( limit>0 && m_bits>limit ) tbit = limit;
				else tbit = m_bits;
				const auto idx = __builtin_ffs( tbit );
				const auto popc = __builtin_popcount( tbit );
				return( idx<1 || idx>last_bit+1 || popc!=1 )?nullptr:nmtbl[idx-1];
			}
			//Set bit by name
			void set_bit( const char* name, const char* const nmtbl[] ) {
				for( size_t i=0; nmtbl[i]; ++i ) {
					if( !std::strcmp(name, nmtbl[i]) )
						m_bits |= 1 << i;
				}
			}
			unsigned m_bits;
		};
}
}
