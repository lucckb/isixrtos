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
class unit_tests {
	QUnit::UnitTest qunit {QUnit::verbose };
	void heap_test() {
		auto ptr1 = isix::isix_alloc( 1 );
		QUNIT_IS_NOT_EQUAL( ptr1, nullptr );
		isix::isix_free( ptr1 );
		int fragments {};
		const int freem =  isix::isix_heap_free( &fragments );
		QUNIT_IS_TRUE( freem > 0 );
	}
public:
	void run() {
		heap_test();
	}
};

/* ------------------------------------------------------------------ */
int main()
{
#ifdef PDEBUG
    stm32::usartsimple_init( USART2,115200,true, CONFIG_PCLK1_HZ, CONFIG_PCLK2_HZ );
#endif
    dblog_init_putc_locked( stm32::usartsimple_putc, NULL, detail::usart_lock, detail::usart_unlock );
	static unit_tests testobj;
	testobj.run();
	//isix::isix_start_scheduler();
	return 0;
}

/* ------------------------------------------------------------------ */
