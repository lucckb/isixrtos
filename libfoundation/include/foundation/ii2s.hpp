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
		//! Async calback definition
		using async_callback_t = std::function<void*(void*)>;

		//! Audio bus data format
		enum class datafmt {
			d16_b,
			d16b_ext,
			d24_b,
			d32_b
		};
		//! Bus mode master or slave
		enum class mode {
			master,
			slave
		};
		//! Transmission standard
		enum standard {
			phillips,
			msb,
			lsb,
			pcm_short,
			pcm_long
		};
		//! Polarity
		enum polarity {
			low,
			high
		};

		virtual ~ii2s() {}
		/** Setup bus parameters
		 * @param[in] fmt Bus data format @see datafmt
		 * @param[in] freq Sampling frequency
		 * @return error code
		 */
		virtual int bus_params( datafmt fmt, int freq ) = 0;
		/** Start bus processing
		 * @return error code
		 */
		virtual int start() = 0;
		/** Stop bus processing
		 * @return error code
		*/
		virtual int stop() = 0;
		/* Register playback callback called from interrupt context
		 * @param[in] fn Playback callback
		 * @return false if success otherwise true
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
	protected:
		void* swap_playback(void* newbuf) const noexcept {
			return m_play_cb?m_play_cb( newbuf ):nullptr;
		}
		void* swap_record(void* newbuf) const noexcept {
			return m_record_cb?m_record_cb( newbuf ):nullptr;
		}
	private:
		async_callback_t m_play_cb;
		async_callback_t m_record_cb;
	};

}}

