/*
 * gpio.hpp
 *
 *  Created on: 21 sty 2018
 *      Author: lucck
 */

#pragma once
#include <periph/gpio/gpio_impl.hpp>
#include "gpiomodes.hpp"
#include "gpioll.hpp"

namespace periph {
namespace gpio {

	//! GPIO class interface for handle objects
	class gpio {
	public:
		//! Constructor
		template <typename T>
		gpio( int pin, T&& tag )
			: m_pin(pin)
		{
			setup(std::forward<T>(tag));
		}
		~gpio() {
			setup(m_pin,mode::in{pulltype::floating});
		}
		// !Noncopyable
		gpio& operator=(gpio&) = delete;
		gpio( gpio& ) = delete;
		// Set values
		void operator()(bool bit) noexcept {
			impl::set(m_pin,bit);
		}
		//Get values
		bool operator()() const noexcept {
			return impl::get(m_pin);
		}
		//! Toggle port
		void toggle() noexcept {
			impl::toggle(m_pin);
		}
	private:
		int m_pin {};
	};
}}

