/** Low level gpio API */
#pragma once

#include <utility>
#include <initializer_list>
#include <periph/gpio/gpio_impl.hpp>
#include <periph/gpio/gpio_numbers.hpp>

namespace periph {
namespace gpio {
namespace {

	//! Single pin setup
	template <typename MODE>
		void setup(int pin, MODE&& tag) {
			impl::setup(pin,std::forward<MODE>(tag));
	}
	//! Multi pin setup
	template <typename MODE>
		void setup( std::initializer_list<int> pins, MODE&& tag) {
			for( auto pin: pins ) {
				impl::setup(pin,::std::forward<MODE>(tag));
			}
		}
	//! Set value
	inline void set(int pin, bool val) {
		impl::set(pin,val);
	}
	//! Get value
	inline bool get(int pin) {
		return impl::get(pin);
	}
	//!Set all values
	inline void set_mask(int pin, unsigned mask) {
		impl::set_mask(pin,mask);
	}
	//! Clear values by mask
	inline void clr_mask(int pin, unsigned mask) {
		impl::clr_mask(pin,mask);
	}
	//! Set clr mask
	inline void set_clr_mask(int pin, unsigned set, unsigned clr) {
		impl::set_clr_mask(pin,set,clr);
	}
	//! Get mask
	inline unsigned get_mask( int pin, unsigned mask=0xFFFFFFFFU ) {
		return impl::get_mask(pin,mask);
	}
}
}}

