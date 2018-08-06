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

namespace isix {
	class event;
}


namespace periph {

	using pointer = void*;
	using cpointer = const void*;
	using size = std::size_t;

	class device_option;
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
		template<typename settable_device_option>
			void set_option(settable_device_option& option)  {
			return do_set_option( option );
		}
		template<typename ...settable_device_option>
			void set_option(settable_device_option& ... options) {
				return set_option(options...);
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
		/** Open the device driver
		 * @param[in] flags Open flags
		 * @param[in] timeout Global device timeout
		 * @return error code
		 */
		virtual int open(unsigned flags, int timeout) = 0;
		/** Close the device driver
		 * @return error code
		 */
		virtual int close() = 0;
		/** Suspend or resume device with selected state
		 * @param[in] state suspend state
		 * @return error code or success
		 */
		virtual int pm_suspend(int /*state*/) {
			return error::nosys;
		}
	protected:
		//! Do set option implementation specific
		virtual int do_set_option(device_option& opt) = 0;
		//! Get device base addr
		template<typename device_type>
		auto io() { return reinterpret_cast<device_type*>(m_base_addr); };
		template<typename device_type>
		auto io() const { return reinterpret_cast<const device_type*>(m_base_addr); };
		const type_ m_type;		//! Device type
		uintptr_t m_base_addr;	//! Base address
	};
}

