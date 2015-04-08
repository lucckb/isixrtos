/*
 * =====================================================================================
 *
 *       Filename:  event_tests.cpp
 *
 *    Description:  EVENT tests base class implementation
 *
 *        Version:  1.0
 *        Created:  08.04.2015 18:38:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include "qunit.hpp"
#include <foundation/dbglog.h>
#include "event_tests.hpp"
#include <isix.h>

namespace tests {

namespace {
		
	static const unsigned EV0 = 1U<<0;
	static const unsigned EV1 = 1U<<1;
	static const unsigned EV2 = 1U<<2;
	static const unsigned EV3 = 1U<<3;
	
	//Task for post events
	class task_post : public isix::task_base {
		static constexpr auto STACK_SIZE = 1024;
	public:
		task_post( osevent_t _ev ) 
			: ev( _ev )
		{}
		virtual ~task_post() {};
		void start( osprio_t prio ) {
			start_thread( STACK_SIZE, prio );
		}
	protected:
		virtual void main()
		{
			isix_wait_ms(20);
			static constexpr auto nposts = 1;
			for( unsigned n=0; n<nposts; ++n ) {
				auto r = isix_event_set( ev,  EV0|EV1 );
				dbprintf("Set bit val %08x", r );
			}
		}
	private:
		osevent_t ev {};
	};

	//Task for listen event
	class task_listen : public isix::task_base {
		static constexpr auto STACK_SIZE = 1024;
	public:
		task_listen( osevent_t _ev, unsigned _id )
			: ev( _ev ), id( _id )
		{}
		virtual ~task_listen() {}
		void start( osprio_t prio ) {
			start_thread( STACK_SIZE, prio );
		}
	protected:
		virtual void main()
		{
			isix_wait_ms(20);
			dbprintf("Hello listen ");
			while(1) {
				auto ret = isix_event_wait( ev, id, true, true, ISIX_TIME_INFINITE );
				dbprintf("Signaled %08x TID %08x", ret, id );
			}
		}
	private:
		osevent_t ev {};
		unsigned id {};
	};

}

// Base tests implementation procedure
void event_test::base_test()
{
	osevent_t ev = isix_event_create();
	QUNIT_IS_NOT_EQUAL( ev, nullptr );
	task_listen t1( ev, EV0 );
	task_listen t2( ev, EV1|EV3 );
	task_listen t3( ev, EV2 );
	task_listen t4( ev, EV3 );
	task_post tp( ev );
	t1.start(3);
	t2.start(3);
	t3.start(3);
	t4.start(3);
	tp.start(3);
	if(1) {
		isix_wait_ms(5000);
	}
	isix_event_destroy(ev);
}

namespace {
namespace sync {

	static constexpr auto TASK_0_BIT  = ( 1U << 0 );
	static constexpr auto TASK_1_BIT  = ( 1U << 1 );
	static constexpr auto TASK_2_BIT  = ( 1U << 2 );
	static constexpr auto ALL_SYNC_BITS  = TASK_0_BIT|TASK_1_BIT|TASK_2_BIT;

	void task0( void* param )
	{
		auto ev = static_cast<osevent_t>(param);
		for(;;) {
			auto ret = isix_event_sync( ev, TASK_0_BIT, ALL_SYNC_BITS, ISIX_TIME_INFINITE ); 
			if( (ret&ALL_SYNC_BITS)==ALL_SYNC_BITS ) {
				dbprintf("Sync from t0 reached %04x", ret );
			} else {
				dbprintf(" Ret_t0 %08x val %08x", ret, isix_event_get_isr(ev));
			}
		}
	}

	void task1( void *param )
	{
		auto ev = static_cast<osevent_t>(param);
		for(;;) {
			auto ret = isix_event_sync( ev, TASK_1_BIT, ALL_SYNC_BITS, ISIX_TIME_INFINITE ); 
			//dbprintf(" Ret_t1 %08x", ret );
			(void)ret;
		}
	}
	void task2( void *param )
	{
		auto ev = static_cast<osevent_t>(param);
		for(;;) {
			auto ret = isix_event_sync( ev, TASK_2_BIT, ALL_SYNC_BITS, ISIX_TIME_INFINITE ); 
			//dbprintf(" Ret_t2 %08x", ret );
			//(void)ret;
		}
	}
}}


//Sync API test
void event_test::sync_test()
{
	osevent_t ev = isix_event_create();
	QUNIT_IS_NOT_EQUAL( ev, nullptr );
	isix_task_create( sync::task0, ev, 1024, 3, 0 );
	isix_task_create( sync::task1, ev, 1024, 3, 0 );
	isix_task_create( sync::task2, ev, 1024, 3, 0 );
	isix_wait_ms(5000);
	dbprintf("FInal val %08x", isix_event_get_isr(ev) );
	isix_event_destroy( ev );
}

} //ns tests
