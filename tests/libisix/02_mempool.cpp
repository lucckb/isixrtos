#define UNITY_FIXTURE_NO_EXTRAS
#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <cstring>
#include <foundation/sys/dbglog.h>
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
			, m_thr( isix::thread_create(std::bind(&task_test::thread,std::ref(*this))))
		{}
		//Destructor
		~task_test()
		{
			m_pool.free(m_ptr);
		}
		//Start the task
		void start() {
			m_thr.start_thread(STACK_SIZE, TASK_PRIO);
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


TEST_GROUP(mempool);
TEST_SETUP(mempool) {}
TEST_TEAR_DOWN(mempool) {}

TEST(mempool, basic)
{
	static constexpr size_t N_POOL = 14;
	isix::mempool<pool> memp( N_POOL );
	pool* pptr[N_POOL];
	for(size_t n=0; n<N_POOL; ++n) {
		auto p = memp.alloc(n+'A');
		TEST_ASSERT_NOT_NULL(p);
		TEST_ASSERT_EQUAL(0, reinterpret_cast<long>(p)%ISIX_BYTE_ALIGNMENT_SIZE);
		TEST_ASSERT(static_cast<bool>(*p));
		pptr[n] = p;
	}
	//Should return unable alloc
#if defined(__EXCEPTIONS)
	try {
		memp.alloc('X');
		TEST_ASSERT(false);
	} catch (std::bad_alloc) {
		TEST_ASSERT(true);
	} catch (...) {
		TEST_ASSERT(false);
	}
#endif
	//Free memory blocks
	for (auto p : pptr) {
		TEST_ASSERT_EQUAL(ISIX_EOK, memp.free(p));
	}
	//Now allocation should be possible
	{
		auto p = memp.alloc('X');
		TEST_ASSERT_NOT_NULL(p);
		TEST_ASSERT_EQUAL(ISIX_EOK, memp.free(p));
	}
}

TEST(mempool, race_condition)
{
	static constexpr size_t N_POOL = 14;
	isix::mempool<pool> memp(N_POOL);
	task_test t1(memp);
	task_test t2(memp);
	task_test t3(memp);
	task_test t4(memp);
	task_test t5(memp);
	task_test t6(memp);
	t1.start();
	t2.start();
	t3.start();
	t4.start();
	t5.start();
	t6.start();
	isix_wait_ms(500);
	TEST_ASSERT_NOT_NULL(t1.get());
	TEST_ASSERT_NOT_NULL(t2.get());
	TEST_ASSERT_NOT_NULL(t3.get());
	TEST_ASSERT_NOT_NULL(t4.get());
	TEST_ASSERT_NOT_NULL(t5.get());
	TEST_ASSERT_NOT_NULL(t6.get());
}

TEST_GROUP_RUNNER(mempool)
{
	RUN_TEST_CASE(mempool, basic);
	RUN_TEST_CASE(mempool, race_condition);
}