/*
 * =====================================================================================
 *
 *       Filename:  mempool_test.cpp
 *
 *    Description:  Mempool test 
 *
 *        Version:  1.0
 *        Created:  28.12.2013 17:22:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <isix.h>
#include "qunit.hpp"
#include "mempool_test.hpp"
#include <cstring>
#include <foundation/dbglog.h>

namespace tests {

//Unnamed namespace
namespace {
	class pool {
	public:
		pool(char pattern)
			: m_pattern( pattern )
		{
			std::memset(val, pattern, sizeof(val)/sizeof(val[0]));
		}
		operator bool() const 
		{
			for( auto v : val ) {
				if( v != m_pattern ) {
					return false;
				}
			}
			return true;
		}
	private:
		char val[17];
		char m_pattern {};
	};
}


// Mempool basic test
void mempool::mempool_tests() {
	
	static constexpr size_t N_POOL = 14;
	isix::mempool<pool> memp( N_POOL );
	pool* pptr[N_POOL]; 
	for(size_t n=0; n<N_POOL; ++n) {
		auto p = memp.alloc(n+'A');
		QUNIT_IS_TRUE( p != nullptr );
		QUNIT_IS_EQUAL( reinterpret_cast<long>(p)%4, 0 );
		QUNIT_IS_TRUE( *p );
		pptr[n] = p;
	}
	//Should return unable alloc
	QUNIT_IS_EQUAL( memp.alloc('X'), nullptr );
	//Free memory blocks
	for(auto p : pptr ) {
		memp.free(p);
	}
	//Now allocation should be possible
	{
 		auto p = memp.alloc('X');
		QUNIT_IS_TRUE(p!=nullptr);
		memp.free(p);
	}
}

}	// Namespace tests end
