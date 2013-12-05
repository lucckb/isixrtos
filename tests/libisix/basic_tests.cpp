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
int main()
{
#ifdef PDEBUG
    stm32::usartsimple_init( USART2,115200,true, CONFIG_PCLK1_HZ, CONFIG_PCLK2_HZ );
#endif
    dblog_init_putc_locked( stm32::usartsimple_putc, NULL, detail::usart_lock, detail::usart_unlock );
	dbprintf("Basic unit test framework started");
	QUnit::UnitTest qunit(QUnit::verbose);
	QUNIT_IS_TRUE( true );
	QUNIT_IS_EQUAL( 5 , 5 );
	return 0;
}

/* ------------------------------------------------------------------ */
