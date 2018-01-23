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
	public:
		//Noncopyable
		gpio& operator=(gpio&) = delete;
		gpio( gpio& ) = delete;
		// Set values
		void operator()(bool bits) noexcept;
		//Get values
		bool operator()() const noexcept;
	private:
		int m_pin;
	};
}}
