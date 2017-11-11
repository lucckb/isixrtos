/*
 * =====================================================================================
 *
 *       Filename:  mempool_test.cpp
 *
 *    Description:  Mempol tests
 *
 *        Version:  1.0
 *        Created:  24.06.2017 16:17:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <lest/lest.hpp>
#include <isix.h>
#include <cstring>

//Unnamed namespace pool class
namespace
{
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
	class task_test
	{
	static constexpr auto STACK_SIZE = 1024;
	static constexpr auto TASK_PRIO = 3;
	public:
		//Constructor
		task_test( isix::mempool<pool>& pool )
			: m_pool( pool )
			, m_thr( isix::thread_create( std::bind(&task_test::thread,std::ref(*this))))
		{}
		//Destructor
		~task_test()
		{
			m_pool.free(m_ptr);
		}
		//Start the task
		void start() {
			m_thr.start_thread(STACK_SIZE, TASK_PRIO );
		}
		//Check valid
		pool* get() const {
			return m_ptr;
		}
		task_test( task_test& ) = delete;
		task_test& operator=( task_test& ) = delete;
	protected:
		//Main thread
		void thread( ) noexcept
		{
			m_ptr = m_pool.alloc('Z');
		}
	private:
		isix::mempool<pool>& m_pool;
		pool* m_ptr {};
		isix::thread m_thr;
	};
}	//Unnamed namespace end

const lest::test module[] =
{
	CASE( "02_mempool_01 Mempool basic check" )
	{
		static constexpr size_t N_POOL = 14;
		isix::mempool<pool> memp( N_POOL );
		pool* pptr[N_POOL]; 
		for(size_t n=0; n<N_POOL; ++n) {
			auto p = memp.alloc(n+'A');
			EXPECT( p != nullptr );
			EXPECT( reinterpret_cast<long>(p)%ISIX_BYTE_ALIGNMENT_SIZE == 0U );
			EXPECT( *p );
			pptr[n] = p;
		}
		//Should return unable alloc
		EXPECT( memp.alloc('X') == nullptr );
		//Free memory blocks
		for(auto p : pptr ) {
			EXPECT( memp.free(p) == ISIX_EOK );
		}
		//Now allocation should be possible
		{
			auto p = memp.alloc('X');
			EXPECT(p!=nullptr);
			EXPECT( memp.free(p) == ISIX_EOK );
		}
	},
	CASE( "02_mempool_02 Mempool race condition check" )
	{
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
		EXPECT( t1.get() != nullptr);
		EXPECT( t2.get() != nullptr);
		EXPECT( t3.get() != nullptr);
		EXPECT( t4.get() != nullptr);
		EXPECT( t5.get() != nullptr);
		EXPECT( t6.get() != nullptr);
	}
};


extern lest::tests & specification();
MODULE( specification(), module )
