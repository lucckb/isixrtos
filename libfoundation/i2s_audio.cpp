/*
 * =====================================================================================
 *
 *       Filename:  i2s_audio.cpp
 *
 *    Description:  I2S audio device implementation
 *
 *        Version:  1.0
 *        Created:  31.05.2017 21:40:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <foundation/i2s_audio.hpp>
#include <isix/arch/cache.h>

namespace fnd {
namespace drv {


//! Constructor
i2s_audio::i2s_audio( bus::ii2s& i2s_bus )
	: m_bus(i2s_bus)
	, m_mempool(isix::mempool_create(c_mempool_cnt,c_mempool_siz))
{
	if( !m_mempool ) {
		throw std::bad_alloc();
	}
	m_bus.register_record(
		std::bind(&i2s_audio::record_callback,std::ref(*this),std::placeholders::_1)
	);
	m_bus.register_playback(
		std::bind(&i2s_audio::play_callback,std::ref(*this),std::placeholders::_1)
	);
}

//! Destructor
i2s_audio::~i2s_audio()
{
	discard_streams();
	isix::mempool_destroy( m_mempool );
}

//! Stream configuration
int i2s_audio::stream_conf( unsigned samplerate, format fmt, chn ch ) noexcept
{
	if( m_state!=state::idle || m_state!=state::wait_conf ) {
		return error( error::busy );
	}
	if( ch == chn::mono ) {
		return error( error::chnsupp );
	}
	bus::ii2s::datafmt busfmt;
	switch( fmt ) {
		case format::le16:
			busfmt = bus::ii2s::datafmt::d16_b;
			break;
		case format::le24:
			busfmt = bus::ii2s::datafmt::d24_b;
			break;
		default:
			return error( error::fmtsupp );
	}
	m_state = state::idle;
	return error( m_bus.bus_params(busfmt,samplerate) );
}


//! Start audio processing
int i2s_audio::start( bool play, bool record ) noexcept
{
	if( m_state!=state::idle ) {
		return error(error::busy);
	}
	const auto err = m_bus.start(play,record,c_mempool_siz );
	if( !err ) {
		m_state = state::sampling;
		m_record = record;
	}
	return error(err);
}

//! Stop audio processing
int i2s_audio::stop() noexcept
{
	if( m_state==state::sampling ) {
		m_state = state::stop_wait;
		m_fin_sem.wait(ISIX_TIME_INFINITE);
		return error(error::success);
	} else {
		return error(error::notrunning);
	}
}

//! Release record stream
int i2s_audio::release_record_stream( void* buf ) noexcept
{
	isix_mempool_free( m_mempool, buf );
	return error();
}
//! Release playback stream
int i2s_audio::release_playback_stream(void* buf, int timeout ) noexcept
{
	if( m_state != state::sampling ) {
		isix::mempool_free(m_mempool,buf);
		return error( error::notrunning );
	}
	isix::clean_dcache_by_addr( buf, c_mempool_siz );
	const auto err = m_rec_fifo.push( buf, timeout );
	if( err ) {
		return error(err);
	}
	return error();
}

//! Get record stream
void* i2s_audio::get_record_stream( int timeout ) noexcept
{
	void* ret {};
	if( m_state == state::sampling ) {
		const auto err = m_rec_fifo.pop(ret,timeout);
		if( err ) {
			error(err);
		} else {
			isix::inval_dcache_by_addr( ret, c_mempool_siz );
		}
	}
	return ret;
}

//! Get playback stream
void* i2s_audio::get_playback_stream() noexcept
{
	const auto ret = isix::mempool_alloc(m_mempool);
	if( !ret ) {
		error( error::nomem );
	}
	return ret;
}

//! Record callback called from ISR context
void* i2s_audio::record_callback( void* ptr ) noexcept
{
	if( m_state!=state::sampling ) {
		return nullptr;
	}
	const auto err = m_rec_fifo.push_isr(ptr);
	if( err ) {
		error(err);
		return nullptr;
	} else {
		const auto ret = isix::mempool_alloc(m_mempool);
		if( !ret ) {
			error( error::nomem );
		}
		return ret;
	}
}


//! Play callback called from ISR context
void* i2s_audio::play_callback( void* ptr ) noexcept
{
	void* ret {};
	if( m_state != state::sampling ) {
		return ret;
	}
	if( ptr ) {
		isix::mempool_free(m_mempool,ptr);
	}
	auto err = m_play_fifo.pop_isr(ret);
	if( err ) {
		error(err);
	}
	return ret;
}

//! Error callback called from ISR context
void i2s_audio::error_callback( int err, void* buf1, void* buf2 ) noexcept
{
	if( buf1 ) isix::mempool_free( m_mempool, buf1 );
	if( buf1 ) isix::mempool_free( m_mempool, buf2 );
	if( m_state==state::stop_wait ) {
		if( m_record ) {
			m_state=state::stop_wait2;
		} else {
			m_fin_sem.signal_isr();
		}
	} else if( m_state==state::stop_wait2 ) {
		m_fin_sem.signal_isr();
	} else {
		m_fin_sem.signal_isr();
		error( err );
	}
}


//! Discard all buffers waiting in queue
void i2s_audio::discard_streams() noexcept
{
	void* ret;
	while( m_play_fifo.pop_isr(ret) == ISIX_EOK ) {
		isix_mempool_free( m_mempool, ret );
	}
	while( m_rec_fifo.pop_isr(ret) == ISIX_EOK ) {
		isix_mempool_free( m_mempool, ret );
	}
}

}}


