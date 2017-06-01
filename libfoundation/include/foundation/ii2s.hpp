/*
 * =====================================================================================
 *
 *       Filename:  ii2s.hpp
 *
 *    Description:  Low Level I2S interface for audio drivers
 *
 *        Version:  1.0
 *        Created:  27.05.2017 21:01:12
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <functional>

namespace fnd {
namespace bus {

	class ii2s
	{
	public:
		struct error {
		enum _error {
			none		= 0,
			busy		= -32767,	//! Busy
			smprate		= -32768,	//! Invalid sample rate
			smpmode		= -32769,	//! Invalid sample mode
			nosupp		= -32770,	//! Unsupported mode
			nullbuf		= -32771,	//! Null buffer
			dma_play	= -32772,	//! Dma transfer fail
			dma_rec		= -32773,	//! Dma transfer fail
			underrun_play    = -32774,	//! Underrun error
			underrun_rec    = -32775,	//! Underrun error
		};};
		//! Async calback definition
		using async_callback_t = std::function<void*(void*)>;
		//! Error callback int code, void* buf2free1, void* buf2free2
		using error_callback_t = std::function<void(int,void*,void*)>;

		//! Audio bus data format
		enum class datafmt {
			d16_b,
			d16b_ext,
			d24_b,
			d32_b
		};
		//! Bus mode master or slave
		enum class mode : bool {
			master,
			slave
		};
		//! Transmission standard
		enum standard : unsigned char {
			phillips,
			msb,
			lsb,
			pcm_short,
			pcm_long
		};
		//! Polarity
		enum polarity : bool {
			low,
			high
		};

		virtual ~ii2s() {}
		/** Setup bus parameters
		 * @param[in] fmt Bus data format @see datafmt
		 * @param[in] freq Sampling frequency
		 * @return error code
		 */
		virtual int bus_params( datafmt fmt, unsigned freq ) noexcept = 0;
		/** Start bus processing
		 * @param[in] play Start playback
		 * @param[in] record Start record
		 * @param[in] Buflen input buffer len
		 * @return error code
		 */
		virtual int start( bool play, bool record, std::size_t buflen ) noexcept = 0;
		/** Stop bus processing
		 * @return error code
		*/
		bool register_playback( async_callback_t fn ) noexcept {
			if(m_play_cb) return true;
			m_play_cb = fn;
			return false;
		}
		/* Register record callback called from interrupt context
		 * @param[in] fn Playback callback
		 * @return false if success otherwise true
		 */
		bool register_record( async_callback_t fn ) noexcept {
			if(m_record_cb) return true;
			m_record_cb = fn;
			return false;
		}
		/* Register error callback called from interrupt context
		 * @param[in] fn Playback callback
		 * @return false if success otherwise true
		 */
		bool register_error( error_callback_t fn ) noexcept {
			if(m_err_cb) return true;
			m_err_cb = fn;
			return false;
		}
	protected:
		void* swap_playback(void* freebuf) const noexcept {
			return m_play_cb?m_play_cb( freebuf ):nullptr;
		}
		void* swap_record(void* freebuf) const noexcept {
			return m_record_cb?m_record_cb( freebuf ):nullptr;
		}
		void report_error( int err, void* b1, void* b2 ) noexcept {
			if(m_err_cb) m_err_cb( err, b1, b2 );
		}
	private:
		async_callback_t m_play_cb;
		async_callback_t m_record_cb;
		error_callback_t m_err_cb;
	};

}}

