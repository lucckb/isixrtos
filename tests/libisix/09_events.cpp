/*
 * =====================================================================================
 *
 *       Filename:  event_tests.cpp
 *
 *    Description:  Event test base API
 *
 *        Version:  1.0
 *        Created:  24.06.2017 17:49:53
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



namespace {
	constexpr auto c_stack_size = 1024U;
	constexpr auto c_stack_margin = 100U;

namespace base {
	constexpr unsigned EV0 = 1U<<0;
	constexpr unsigned EV1 = 1U<<1;
	constexpr unsigned EV2 = 1U<<2;
	const unsigned EV3 = 1U<<3;

	//Task for post events
	class task_post {
	public:
		task_post( osevent_t _ev )
			: ev( _ev )
			, m_thr( isix::thread_create( std::bind(&task_post::thread,std::ref(*this))))
		{}
		void start( osprio_t prio ) {
			m_thr.start_thread( c_stack_size, prio );
		}
		task_post& operator=( task_post& ) = delete;
		task_post( task_post& ) = delete;
	private:
		void thread() noexcept
		{
			isix_wait_ms(20);
			static constexpr auto nposts = 1;
			for( unsigned n=0; n<nposts; ++n ) {
				isix_event_set( ev,  EV0|EV1 );
			}
		}
	private:
		osevent_t ev {};
		isix::thread m_thr;
	};

	//Task for listen event
	class task_listen {
		static constexpr auto STACK_SIZE = 1024;
	public:
		task_listen( osevent_t _ev, unsigned _id )
			: ev( _ev ), id( _id )
			, m_thr( isix::thread_create( std::bind(&task_listen::thread,std::ref(*this))))
		{}
		void start( osprio_t prio ) {
			m_thr.start_thread( STACK_SIZE, prio );
		}
		task_listen( task_listen& ) = delete;
		task_listen& operator=( task_listen& ) = delete;
	private:
		void thread() noexcept
		{
			while(1) {
				isix_event_wait( ev, id, true, true, ISIX_TIME_INFINITE );
			}
		}
	private:
		osevent_t ev {};
		unsigned id {};
		isix::thread m_thr;
	};
}
}

//Namespace for fifo conn
namespace {
namespace tfifo {
	struct evfifo
	{
		osfifo_t fifo1 { isix_fifo_create( 16, sizeof(char) ) };
		osfifo_t fifo2 { isix_fifo_create( 16, sizeof(char) ) };
		osevent_t ev { isix_event_create() };
		int err1 {};
		int err2 {};
	};
	void fifo_task1( void *ptr )
	{
		constexpr auto ch = 'A';
		auto& dt = *static_cast<evfifo*>(ptr);
		for(;;) {
			dt.err1 = isix_fifo_write( dt.fifo1, &ch, ISIX_TIME_INFINITE );
			isix_task_suspend(nullptr);
		}
	}
	void fifo_task2( void *ptr )
	{
		constexpr auto ch = 'B';
		auto& dt = *static_cast<evfifo*>(ptr);
		for(;;) {
			dt.err2 = isix_fifo_write( dt.fifo2, &ch, ISIX_TIME_INFINITE );
			isix_task_suspend(nullptr);
		}
	}
}
}

const lest::test module[] =
{
	CASE( "09_events_01 Events api base check" )
	{
		using namespace base;
		osevent_t ev = isix_event_create();
		EXPECT( ev );
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
		isix_wait_ms(5000);
		isix_event_destroy(ev);
	},
	CASE("09_events_02 Events sync test")
	{
		static constexpr auto TASK_0_BIT  = ( 1U << 0 );
		static constexpr auto TASK_1_BIT  = ( 1U << 1 );
		static constexpr auto TASK_2_BIT  = ( 1U << 2 );
		static constexpr auto ALL_SYNC_BITS  = TASK_0_BIT|TASK_1_BIT|TASK_2_BIT;
		osevent_t ev = isix_event_create();
		struct mystat {
			unsigned err {};
			unsigned ok {};
		} stats[3];
		const auto evsync_thr = [&]( mystat& stat, unsigned bit )
		{
			for(;;) {
				auto ret = isix_event_sync( ev, bit, ALL_SYNC_BITS, ISIX_TIME_INFINITE );
				if( (ret&ALL_SYNC_BITS)==ALL_SYNC_BITS ) {
					++stat.ok;
				} else {
					--stat.err;
					return;
				}
			}
		};
		EXPECT( ev );
		constexpr auto test_prio = 3;
		auto t1 = isix::thread_create_and_run(
			c_stack_size ,test_prio, 0, evsync_thr, std::ref(stats[0]), TASK_0_BIT
		);
		auto t2 = isix::thread_create_and_run(
			c_stack_size ,test_prio, 0, evsync_thr, std::ref(stats[1]), TASK_1_BIT
		);
		auto t3 = isix::thread_create_and_run(
			c_stack_size ,test_prio, 0, evsync_thr, std::ref(stats[2]), TASK_2_BIT
		);
		EXPECT( t1 );
		EXPECT( t2 );
		EXPECT( t3 );
		isix_wait_ms(5000);
		t1.kill();
		t2.kill();
		t3.kill();
		for( auto& stat : stats ) {
			EXPECT( stat.ok > 10000U );
			EXPECT( stat.err == 0U );
		}
		auto df1 = std::abs( int(stats[0].ok - stats[1].ok) );
		auto df2 = std::abs( int(stats[2].ok - stats[1].ok) );
		EXPECT( df1 < 3 );
		EXPECT( df2 < 3 );
		isix_wait_ms(50);
		isix_event_destroy( ev );
	},
	CASE( "09_events_03 Events fifo conn api" )
	{
		using namespace tfifo;
		static constexpr auto EV1 = 1U<<0;
		static constexpr auto EV2 = 1U<<1;
		evfifo fstr;
		//Check fifos
		EXPECT( fstr.fifo1 );
		EXPECT( fstr.fifo2 );
		EXPECT( fstr.ev );
		EXPECT( isix_fifo_event_connect(fstr.fifo1,fstr.ev,0)==ISIX_EOK );
		EXPECT( isix_fifo_event_connect(fstr.fifo2,fstr.ev,1)==ISIX_EOK );
		auto t1 =  isix_task_create( fifo_task1, &fstr, c_stack_size, 3, 0 );
		auto t2 =  isix_task_create( fifo_task2, &fstr, c_stack_size, 3, 0 );
		EXPECT( t1 );
		EXPECT( t2 );
		osbitset_t abits=0;
		for(int c=0;c<10;) {
			auto sbits = isix::event_wait( fstr.ev, EV1|EV2, true, false );
			if( sbits & EV1 ) {
				char ch;
				EXPECT( isix::fifo_read(fstr.fifo1,&ch)==ISIX_EOK );
				EXPECT( ch == 'A');
			}
			if( sbits & EV2 ) {
				char ch;
				EXPECT( isix::fifo_read(fstr.fifo2,&ch)==ISIX_EOK );
				EXPECT( ch=='B');
			}
			abits |= sbits;
			if( (abits&(EV1|EV2)) == (EV1|EV2) ) {
				c++;
				isix_wait_ms(5);	//Get chance to task fin
				EXPECT(isix_task_resume(t1)==ISIX_EOK );
				EXPECT(isix_task_resume(t2)==ISIX_EOK );
			}
		}
		EXPECT( isix_fifo_event_disconnect(fstr.fifo1,fstr.ev)==ISIX_EOK);
		EXPECT( isix_fifo_event_disconnect(fstr.fifo2,fstr.ev)==ISIX_EOK);
		EXPECT( isix_fifo_destroy(fstr.fifo1)==ISIX_EOK );
		EXPECT( isix_fifo_destroy(fstr.fifo2)==ISIX_EOK );
		EXPECT( isix_event_destroy(fstr.ev)==ISIX_EOK );
		isix_task_kill( t1 );
		isix_task_kill( t2 );
	}
};


extern lest::tests & specification();
MODULE( specification(), module )

