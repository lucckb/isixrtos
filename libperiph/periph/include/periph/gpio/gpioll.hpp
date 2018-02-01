/** Low level gpio API */
#pragma once

#include <utility>
#include <initializer_list>
#include <periph/gpio/gpio_numbers.hpp>
#include <periph/gpio/gpio_impl.hpp>

namespace periph {
namespace gpio {
namespace {

	//! Single pin setup
	template <typename MODE>
		void setup(int pin, MODE&& tag) {
			impl::setup(pin,std::forward<MODE>(tag));
	}
	//! Multi pin setup
	template <typename MODE,typename T>
		void setup( std::initializer_list<T> pins, MODE&& tag) {
			for( auto pin: pins ) {
				impl::setup(pin,::std::forward<MODE>(tag));
			}
		}
	//! Set value
	inline void set(int pin, bool val) {
		impl::set(pin,val);
	}
	//! Toggle the port
	inline void toggle(int pin) {
		impl::toggle(pin);
	}
	//! Get value
	inline bool get(int pin) {
		return impl::get(pin);
	}
	//!Set all values
	inline void set_mask(int port, unsigned mask) {
		impl::set_mask(port,mask);
	}
	//! Clear values by mask
	inline void clr_mask(int port, unsigned mask) {
		impl::clr_mask(port,mask);
	}
	//! Set clr mask
	inline void set_clr_mask(int port, unsigned set, unsigned clr) {
		impl::set_clr_mask(port,set,clr);
	}
	//! Get mask
	inline unsigned get_mask(int port, unsigned mask=0xFFFFFFFFU ) {
		return impl::get_mask(port,mask);
	}
}
}}

