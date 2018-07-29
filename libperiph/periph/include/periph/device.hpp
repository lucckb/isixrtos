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

namespace isix {
	class event;
}


namespace periph {

	using pointer = void*;
	using cpointer = const void*;
	using size = std::size_t;
	using devname = char[8];

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
		explicit device(type_ type)
			: m_type(type) {
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
		virtual int event_add(isix::event&ev, unsigned bits, poll events);
		//! Monitoring event off
		virtual int event_del(isix::event& ev, unsigned bits, unsigned events=poll::in|poll::out|poll::err);
		virtual int open(int timeout=0) = 0;
		virtual int close() = 0;
	protected:
		virtual int do_set_option(device_option& opt) = 0;
	private:
		const type_ m_type;
	};
}

