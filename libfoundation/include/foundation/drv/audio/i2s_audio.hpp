/*
 * =====================================================================================
 *
 *       Filename:  i2s_audio.hpp
 *
 *    Description:  I2S audio device
 *
 *        Version:  1.0
 *        Created:  31.05.2017 18:12:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <foundation/drv/audio/audio_device.hpp>
#include <foundation/drv/bus/ii2s.hpp>
#include <isix.h>


namespace fnd {
namespace drv {

	//! I2S audio class implementation
	class i2s_audio : public audio_device
	{
		static constexpr auto c_mempool_siz = 1024;
		static constexpr auto c_mempool_cnt = 16;
		static constexpr auto c_timeout_ms = 3000;
	public:
		i2s_audio( bus::ii2s& i2s_bus );
		virtual ~i2s_audio();
		i2s_audio(i2s_audio&) = delete;
		i2s_audio& operator=(i2s_audio&) = delete;
		int stream_conf( unsigned samplerate, format fmt=format::def, chn ch=chn::def )
			noexcept override = 0;
		int start( bool play, bool record ) noexcept override;
		int stop() noexcept override;
	protected:
		int release_record_stream( void* buf ) noexcept override;
		int release_playback_stream( void* buf ) noexcept override;
		void* get_record_stream() noexcept override;
		void* get_playback_stream() noexcept override;
		std::size_t pbuf_size() const noexcept override {
			return c_mempool_siz;
		}
	private:
		//! Record callback swap errors called from isr ctx
		void* record_callback( void* ptr ) noexcept;
		void* play_callback( void* ptr ) noexcept;
		void error_callback(int error, void* buf1, void* buf2 ) noexcept;
		//! Discard all buffers waiting in queue
		void discard_streams() noexcept;
	private:
		/* data */
		bus::ii2s& m_bus;
		isix::mempool_t m_mempool;
		isix::fifo<void*> m_play_fifo { c_mempool_cnt*2+1 };
		isix::fifo<void*> m_rec_fifo { c_mempool_cnt*2+1 };
		isix::semaphore   m_fin_sem { 1, 1 };
		isix::semaphore   m_mem_sem { c_mempool_cnt, c_mempool_cnt };
		enum class state : short {
			wait_conf,	//! Wait for configuration
			idle,		//! Ready for processing
			sampling,	//! Sampling
			stop_wait,	//! Wait for terminate transfer
			stop_wait2	//! Wait for terminate transfer second notify
		} m_state {};
		bool m_record {};	//! If recording
	};
}}

