/* ------------------------------------------------------------------ */
#include <config.h>
#include <isix.h>
#include <usart_simple.h>
#include <foundation/dbglog.h>
#include <qunit.hpp>
/* ------------------------------------------------------------------ */
namespace {
namespace detail {

   isix::semaphore usem(1, 1);
   void usart_lock() {
       usem.wait(isix::ISIX_TIME_INFINITE);
   }
   void usart_unlock() {
       usem.signal();
   }

}}
/* ------------------------------------------------------------------ */
namespace {
	class base_task_tests : public isix::task_base {
		static constexpr auto STACK_SIZE = 1024;
		volatile unsigned m_exec_count {};
		//Main function
		virtual void main() {
			for(;;) {
				++m_exec_count;
			}
		}
	public:
		static constexpr auto TASK_PRIO = 1;
		base_task_tests()
			: task_base( STACK_SIZE, TASK_PRIO )
		{};
		unsigned exec_count() const {
			return m_exec_count;
		}
		void exec_count( unsigned v ) {
			m_exec_count = v;
		}
	};
}
/* ------------------------------------------------------------------ */
class unit_tests : public isix::task_base
{
	static constexpr auto STACK_SIZE = 2048;
	QUnit::UnitTest qunit {QUnit::verbose };
	//Test heap
	void heap_test() {
		auto ptr1 = isix::isix_alloc( 1 );
		QUNIT_IS_NOT_EQUAL( ptr1, nullptr );
		isix::isix_free( ptr1 );
		int fragments {};
		const int freem =  isix::isix_heap_free( &fragments );
		dbprintf("Free %i frags %i", freem, fragments );
		QUNIT_IS_TRUE( freem > 0 );
	}
	//Test basic tasks
	void basic_tasks_tests()
	{
		auto t1 = new base_task_tests;
		auto t2 = new base_task_tests;
		auto t3 = new base_task_tests;
		auto t4 = new base_task_tests;
		//Active wait tasks shouldnt run
		for(auto tc = isix::isix_get_jiffies(); isix::isix_get_jiffies()<tc+5000; ) {
			asm volatile("nop\n");
		}
		QUNIT_IS_FALSE( t1->exec_count() );
		QUNIT_IS_FALSE( t2->exec_count() );
		QUNIT_IS_FALSE( t3->exec_count() );
		QUNIT_IS_FALSE( t4->exec_count() );
		//Now goto sleep
		isix::isix_wait_ms( 5000 );
		//TASK should run now
		QUNIT_IS_TRUE( t1->exec_count()>0 );
		QUNIT_IS_TRUE( t2->exec_count()>0 );
		QUNIT_IS_TRUE( t3->exec_count()>0 );
		QUNIT_IS_TRUE( t4->exec_count()>0 );
		//Zero task count.. change prio and go active wait
		t1->exec_count(0);
		t2->exec_count(0);
		t3->exec_count(0);
		t4->exec_count(0);
		QUNIT_IS_FALSE( t1->exec_count() );
		QUNIT_IS_FALSE( t2->exec_count() );
		QUNIT_IS_FALSE( t3->exec_count() );
		QUNIT_IS_FALSE( t4->exec_count() );

		QUNIT_IS_EQUAL( isix::isix_task_change_prio(t1->get_taskid(),0), base_task_tests::TASK_PRIO );
		QUNIT_IS_EQUAL( isix::isix_task_change_prio(t2->get_taskid(),0), base_task_tests::TASK_PRIO );
		QUNIT_IS_EQUAL( isix::isix_task_change_prio(t3->get_taskid(),0), base_task_tests::TASK_PRIO );
		QUNIT_IS_EQUAL( isix::isix_task_change_prio(t4->get_taskid(),0), base_task_tests::TASK_PRIO );

		//Active wait tasks shouldnt run
		for(auto tc = isix::isix_get_jiffies(); isix::isix_get_jiffies()<tc+5000; ) {
				asm volatile("nop\n");
			}
		//TASK should run now
		QUNIT_IS_TRUE( t1->exec_count()>0 );
		QUNIT_IS_TRUE( t2->exec_count()>0 );
		QUNIT_IS_TRUE( t3->exec_count()>0 );
		QUNIT_IS_TRUE( t4->exec_count()>0 );

		//Now delete tasks
		delete t1;
		delete t2;
		delete t3;
		delete t4;
	}
	virtual void main() {
			heap_test();
			basic_tasks_tests();
			isix::isix_shutdown_scheduler();
		}
public:
	unit_tests()
		: task_base( STACK_SIZE, 0 )
	{}
};

/* ------------------------------------------------------------------ */
int main()
{
#ifdef PDEBUG
    stm32::usartsimple_init( USART2,115200,true, CONFIG_PCLK1_HZ, CONFIG_PCLK2_HZ );
#endif
    dblog_init_putc_locked( stm32::usartsimple_putc, NULL, detail::usart_lock, detail::usart_unlock );
	static unit_tests testobj;
	isix::isix_start_scheduler();
	return 0;
}

/* ------------------------------------------------------------------ */
