/*
 * =====================================================================================
 *
 *       Filename:  audio_device.hpp
 *
 *    Description:  Audio device generic interface
 *
 *        Version:  1.0
 *        Created:  27.05.2017 21:29:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

namespace fnd {
namespace drv {

	class audio_device
	{
	public:
		//! Set current audio format
		enum class format {
			def,		//!Device default
			b8,
			le16,
			be16,
			le24,
			be24,
			le32,
			be32,
		};
		//! Number of audio channels
		enum class chn {
			def,	//! Device default
			mono,
			stereo
		};

		//! Mixer path control
		enum class mixpath {
			linein,
			linein_r,
			linein_l,
			lineout,
			lineout_r,
			lineout_l,
			headphone,
			headphone_l,
			headphone_r,
			mic_in
		};

		//! Switch bits
		struct switchs {
		enum _switchs {
			mic_boost	= 0x01,
			mic_mute	= 0x02,
			mic_select	= 0x04,
			mic_bypass	= 0x08,
			line_bypass	= 0x10,
		};};

		//! Error code
		struct err {
		enum _err {
			success	= 0,
		}; };
		virtual ~audio_device() {}
		/** Configure DAC and stream parameter
		 * @param[in] samplerate Setup sample rate
		 * @param[in] fmt Sample format @see format
		 * @param[in] ch Channel configuration @see chn
		 * @return Error code
		 */
		virtual int stream_conf( unsigned samplerate, format fmt=format::def, chn ch=chn::def )
			noexcept = 0;
		/** Get record stream buffer with audio data.
		 *  Wait some time for buffer fill
		 *  @param[in] timeout Wait time for buffer fill
		 *  @return Buffer with audio data or nullptr if fail
		 */
		template<typename T> T* get_record_streambuf( int timeout ) noexcept {
			return static_cast<T*>(get_record_stream(timeout) );
		}
		/** Frees record streambufer with audio data
		 * @param buf Buffer allocated by get_record_streambuf
		 * @return Error code
		 */
		virtual int release_record_streambuf(void* buf) noexcept = 0;
		/** Get record stream buffer with audio data.
		 *  Wait some time for buffer fill
		 *  @param[in] timeout Wait time for buffer fill
		 *  @return Buffer with audio data or nullptr if fail
		 */
		template<typename T> T* get_playback_streambuf( int timeout ) noexcept {
			return static_cast<T*>(get_record_stream(timeout) );
		}
		/** Frees record streambufer with audio data
		 * @param buf Buffer allocated by get_record_streambuf
		 * @return Error code
		 */
		virtual int release_playback_streambuf(void* buf) noexcept = 0;
		/** Start audio processing need playback and need record
		 * @param[in] record Start recording
		 * @param[in] playback Start playback
		 * @return Error code
		 */
		virtual int start( bool record, bool playback ) noexcept = 0;
		/** Stop audio processing
		 * @return Error code
		 */
		virtual int stop() noexcept = 0;
		/** Get last error code */
		int error() const noexcept {
			return m_error;
		}
		/** Gain control settings in db value
		 * @param[in] mixpath Mixer input path
		 * @param[in[ dbgain Gain in DB
		 * @return Error code
		 */
		virtual int gain_ctl( mixpath path, int dbgain ) noexcept = 0;
		/** Setup the switch parameters configuration
		 * @param[in] swbits_on Path bits for set
		 * @param[in[ swbits_off Path bits for clear
		 * @return Error code
		 * */
		virtual int switch_ctl( unsigned swbits_on, unsigned swbits_off ) noexcept = 0;

	protected:
		virtual void* get_record_stream( int timeout ) noexcept = 0;
		virtual void* get_playback_stream( int timeout ) noexcept = 0;
		void error( int err ) noexcept {
			m_error = err;
		}
	private:
		int m_error {};

	};
}}


