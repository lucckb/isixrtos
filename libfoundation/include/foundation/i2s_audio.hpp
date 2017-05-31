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
#include <foundation/audio_device.hpp>
#include <foundation/ii2s.hpp>

namespace fnd {
namespace drv {

	//! I2S audio clas implementation
	class i2s_audio : public audio_device
	{
	public:
		i2s_audio( bus::ii2s& i2s_bus )
			: m_bus(i2s_bus)
		{}
		virtual ~i2s_audio() {}
		i2s_audio(i2s_audio&) = delete;
		i2s_audio& operator=(i2s_audio&) = delete;
	public:
		int stream_conf( unsigned samplerate, format fmt=format::def, chn ch=chn::def )
			noexcept override;
		int start( bool record, bool playback ) noexcept override;
		int stop() noexcept override;
	protected:
		int release_record_stream( void* buf ) noexcept override;
		int release_playback_stream(void* buf) noexcept override;
		void* get_record_stream( int timeout ) noexcept override;
		void* get_playback_stream( int timeout ) noexcept override;
	private:
		/* data */
		bus::ii2s& m_bus;
	};
}}
