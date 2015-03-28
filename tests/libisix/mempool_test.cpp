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
#include <stm32system.h>

namespace tests {

//Unnamed namespace pool class
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

//Namespace for race condition task
namespace {
	class task_test : public isix::task_base 
	{
	static constexpr auto STACK_SIZE = 1024;
	static constexpr auto TASK_PRIO = 3;
	public:
		//Constructor
		task_test( isix::mempool<pool>& pool )
			: m_pool( pool )
		{}
		//Destructor
		virtual ~task_test() 
		{
			m_pool.free(m_ptr);
		}
		//Start the task
		void start() {
			start_thread(STACK_SIZE, TASK_PRIO );
		}
		//Check valid
		pool* get() const {
			return m_ptr;
		}
	protected:
		//Main thread
		virtual void main( )
		{
			m_ptr = m_pool.alloc('Z');
		}
	private:
		isix::mempool<pool>& m_pool;
		pool* m_ptr {};
	};
}	//Unnamed namespace end

// Mempool basic test
void mempool::mempool_tests() {
	static constexpr size_t N_POOL = 14;
	isix::mempool<pool> memp( N_POOL );
	pool* pptr[N_POOL]; 
	for(size_t n=0; n<N_POOL; ++n) {
		auto p = memp.alloc(n+'A');
		QUNIT_IS_TRUE( p != nullptr );
		QUNIT_IS_EQUAL( reinterpret_cast<long>(p)%ISIX_CONFIG_BYTE_ALIGNMENT_SIZE, 0 );
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
//Mempool task race cond
void mempool::task_racecond() {

	static constexpr size_t N_POOL = 14;
	isix::mempool<pool> memp( N_POOL );
	task_test t1( memp );
	task_test t2( memp );
	task_test t3( memp );
	task_test t4( memp );
	task_test t5( memp );
	task_test t6( memp );
	t1.start();
	t2.start();
	t3.start();
	t4.start();
	t5.start();
	t6.start();
	isix_wait_ms(500);
	QUNIT_IS_TRUE( t1.get() != nullptr);
	QUNIT_IS_TRUE( t2.get() != nullptr);
	QUNIT_IS_TRUE( t3.get() != nullptr);
	QUNIT_IS_TRUE( t4.get() != nullptr);
	QUNIT_IS_TRUE( t5.get() != nullptr);
	QUNIT_IS_TRUE( t6.get() != nullptr);
}

}	// Namespace tests end
