/*
 * gpio.hpp
 *
 *  Created on: 21 sty 2018
 *      Author: lucck
 */

namespace periph {
namespace gpio {

	//! GPIO class interface for handle objects
	class gpio {
		class input {
			public:
				enum type { floating, pullup, pulldown, alternate, analog };
				input( type value ) : m_value(value) {}
				type value() const { return m_value; }
			private:
				type m_value;
		};
		// Set values
		void operator()(unsigned bits) noexcept;
		//Get values
		unsigned operator()() const noexcept;
		// Configure port
		template <typename Tag>
			void configure(unsigned bits, Tag tag) {
		}

	};
}}
