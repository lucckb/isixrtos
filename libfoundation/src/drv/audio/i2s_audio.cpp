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

#include <foundation/drv/audio/i2s_audio.hpp>
#include <isix/arch/cache.h>
#include <cstring>
#include <foundation/sys/dbglog.h>

namespace fnd {
namespace drv {


//! Constructor
i2s_audio::i2s_audio( bus::ii2s& i2s_bus )
	: m_bus(i2s_bus)
	, m_mempool(isix::mempool_create(c_mempool_cnt,c_mempool_siz))
{
	if( !m_mempool ) {
#ifdef __EXCEPTIONS
		throw std::bad_alloc();
#else
		std::abort();
#endif
	}
	m_bus.register_record(
		std::bind(&i2s_audio::record_callback,std::ref(*this),std::placeholders::_1)
	);
	m_bus.register_playback(
		std::bind(&i2s_audio::play_callback,std::ref(*this),std::placeholders::_1)
	);
	m_bus.register_error(
		std::bind(&i2s_audio::error_callback,std::ref(*this),
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
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
	if( m_state!=state::idle && m_state!=state::wait_conf ) {
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
		case format::le32:
			busfmt = bus::ii2s::datafmt::d32_b;
			break;
		case format::le24:
			busfmt = bus::ii2s::datafmt::d24_b;
			break;
		default:
			return error( error::fmtsupp );
	}
	m_state = state::idle;
	return error(m_bus.bus_params(busfmt,samplerate) );
}


//! Start audio processing
int i2s_audio::start( bool play, bool record ) noexcept
{
	if( m_state!=state::idle ) {
		return error(error::notconf);
	}
	//! Get first play buffer
	if( play )
	{
		const auto c_dma_pre_bufs = 2;
		for( int i=0; i<c_dma_pre_bufs; ++i ) {
			auto err = m_mem_sem.wait( c_timeout_ms );
			if( err ) {
				return error(err==ISIX_TIME_INFINITE?error::nomem:err);
			}
			auto buf = isix::mempool_alloc( m_mempool );
			if( !buf ){
				return error( error::nomem );
			}
			std::memset( buf, 0, c_mempool_siz );
			isix::clean_dcache_by_addr( buf, c_mempool_siz );
			err = m_play_fifo.push( buf, c_timeout_ms );
			if( err ) {
				return error( err );
			}
		}
	}
	if( record )
	{
		constexpr auto c_dma_pre_bufs = 2;
		for( int i=0; i<c_dma_pre_bufs; ++i ) {
			auto err = m_mem_sem.wait( c_timeout_ms );
			if( err ) {
				return error(err==ISIX_TIME_INFINITE?error::nomem:err);
			}
			auto buf = isix::mempool_alloc( m_mempool );
			if( !buf ){
				return error( error::nomem );
			}
			err = m_rec_fifo.push( buf, c_timeout_ms );
			if( err ) {
				return error( err );
			}
		}
	}
	m_state = state::sampling;
	m_record = record;
	const auto err = m_bus.start(play,record,c_mempool_siz);
	if( err ) {
		m_record = false;
		m_state = state::idle;
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
	isix::mempool_free( m_mempool, buf );
	const auto err = m_mem_sem.signal();
	return error(err);
}
//! Release playback stream
int i2s_audio::release_playback_stream( void* buf ) noexcept
{
	if( m_state != state::sampling ) {
		isix::mempool_free(m_mempool,buf);
		m_mem_sem.signal();
		return error( error::notrunning );
	}
	isix::clean_dcache_by_addr( buf, c_mempool_siz );
	const auto err = m_play_fifo.push( buf, c_timeout_ms );
	if( err ) {
		return error(err);
	}
	return error();
}

//! Get record stream
void* i2s_audio::get_record_stream() noexcept
{
	void* ret {};
	int err;
	if( m_state == state::sampling )
	{
		err = m_rec_fifo.pop( ret ,c_timeout_ms );
		if(!err) {
			isix::inval_dcache_by_addr( ret, c_mempool_siz );
		}
		error(err);
	}
	else
	{
		error(error::notrunning);
	}
	return ret;
}

//! Get playback stream
void* i2s_audio::get_playback_stream() noexcept
{
	void* ret {};
	auto err = m_mem_sem.wait( c_timeout_ms );
	if( err ) {
		error( err==ISIX_ETIMEOUT?error::nomem:err );
		return ret;
	}
	ret = isix::mempool_alloc(m_mempool);
	if( !ret ) {
		error(error::nomem);
	}
	return ret;
}

//! Record callback called from ISR context
void* i2s_audio::record_callback( void* ptr ) noexcept
{
	if( m_state!=state::sampling )
		return nullptr;
	if( ptr )
	{
		auto err = m_rec_fifo.push_isr(ptr);
		if( err ) {
			error(err);
			return nullptr;
		}
	}

	const auto err = m_mem_sem.trywait();
	if( err<0 ) {
		error( error::nomem );
		return nullptr;
	}
	const auto ret = isix::mempool_alloc(m_mempool);
	if( !ret ) {
		error( error::nomem );
		return nullptr;
	}
	return ret;
}


//! Play callback called from ISR context
void* i2s_audio::play_callback( void* ptr ) noexcept
{
	if( m_state != state::sampling )
	{
		return nullptr;
	}
	if( ptr )
	{
		isix::mempool_free(m_mempool,ptr);
		auto err = m_mem_sem.signal_isr();
		if( err ) {
			error(err);
			return nullptr;
		}
	}
	void* ret;
	auto err = m_play_fifo.pop_isr(ret);
	if( err ) {
		ret = nullptr;
		error(err);
	}
	return ret;
}

//! Error callback called from ISR context
void i2s_audio::error_callback( int err, void* buf1, void* buf2 ) noexcept
{
	if( buf1 ) {
		isix::mempool_free( m_mempool, buf1 );
		m_mem_sem.signal_isr();
	}
	if( buf1 ) {
		isix::mempool_free( m_mempool, buf2 );
		m_mem_sem.signal_isr();
	}
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
		if(!error()) error( err );
	}
}


//! Discard all buffers waiting in queue
void i2s_audio::discard_streams() noexcept
{
	void* ret;
	while( m_play_fifo.pop_isr(ret) == ISIX_EOK ) {
		isix::mempool_free( m_mempool, ret );
		m_mem_sem.signal_isr();
	}
	while( m_rec_fifo.pop_isr(ret) == ISIX_EOK ) {
		isix::mempool_free( m_mempool, ret );
		m_mem_sem.signal_isr();
	}
}

}}


