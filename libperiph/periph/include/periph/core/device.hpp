/*
 * =====================================================================================
 *
 *       Filename:  device.hpp
 *
 *    Description:  Base device class
 *
 *        Version:  1.0
 *        Created:  29.07.2018 17:41:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <cstddef>
#include <functional>
#include "error.hpp"
#include "device_option.hpp"

namespace isix {
	class event;
}


namespace periph {

	using pointer = void*;
	using cpointer = const void*;
	using size = std::size_t;

	class device
	{
	public:
		enum type_ : unsigned char {
			block_dev,
			char_dev
		};
		struct poll {
		enum poll_ : unsigned char{
			in, out, err
		};};
		//! Device constructor
		device(type_ type, uintptr_t base_addr)
			: m_type(type), m_base_addr(base_addr) {
		}
		//! Device virtual destructor
		virtual ~device() {}
		//! Noncopyable
		device(device&) = delete;
		device& operator=(device&) = delete;
		// Return object type
		auto type() const {
			return m_type;
		}
		// Cast to the specified device type
		template <typename T> T cast() {
			return static_cast<T>(std::ref(*this));
		}
		//! Set error option
		int set_option(const option::device_option& option)  {
			 return  do_set_option(option);
		}
		//! Monitoring event on
		virtual int event_add(isix::event& /*ev*/, unsigned /*bits*/, poll /*events*/) {
			return error::nosys;
		}
		//! Monitoring event off
		virtual int event_del(isix::event& /*ev*/,
				unsigned /*bits*/,
				unsigned /*events=poll::in|poll::out|poll::err*/) {
			return error::nosys;
		}
		virtual int pm_suspend(int /*state*/) {
			return error::nosys;
		}
		int open(int timeout) {
			if(m_opened) return error::success;
			else {
				m_opened = true;
				return do_open(timeout);
			}
		}
		int close() {
			if(m_opened) {
				m_opened = false;
				return do_close();
			}
			else return error::noinit;
		}
	protected:
		/** Open the device driver
		 * @param[in] timeout Global device timeout
		 * @return error code
		 */
		virtual int do_open(int timeout) = 0;
		/** Close the device driver
		 * @return error code
		 */
		virtual int do_close() = 0;
		/** Suspend or resume device with selected state
		 * @param[in] state suspend state
		 * @return error code or success
		 */
		//! Do set option implementation specific
		virtual int do_set_option(const option::device_option& opt) = 0;
		//! Get device base addr
		template<typename device_type>
		auto io() { return reinterpret_cast<device_type*>(m_base_addr); };
		template<typename device_type>
		auto io() const { return reinterpret_cast<const device_type*>(m_base_addr); };
	private:
		const type_ m_type {};		//! Device type
		bool m_opened {};			//! Device is opened
		uintptr_t m_base_addr {};	//! Base address
	};
}

