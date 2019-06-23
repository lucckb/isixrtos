/*
 * =====================================================================================
 *
 *       Filename:  i2c_interrupt_handlers.cpp
 *
 *    Description:  I2c interrupt handlers
 *
 *        Version:  1.0
 *        Created:  21.06.2019 18:16:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <periph/core/error.hpp>
#include <periph/i2c/i2c_interrupt_handlers.hpp>
#include <cstdlib>


namespace periph::drivers::i2c::_handlers {


namespace {
	inline void _abort(htype) { std::abort(); }
#if defined(I2C4)
std::function<void(htype)> i2c_vector[4] { _abort, _abort, _abort, _abort };
#elif defined(I2C3)
std::function<void(htype)> i2c_vector[3] { _abort, _abort, _abort };
#elif defined(I2C2)
std::function<void(htype)> i2c_vector[2] { _abort, _abort };
#elif defined(I2C1)
std::function<void(htype)> i2c_vector[1] { _abort };
#else
#	error NO i2c present
#endif
}

int register_handler(const I2C_TypeDef * const i2c, std::function<void(htype)> callback) {
#ifdef I2C1
		if(i2c==I2C1) {
			i2c_vector[0] = callback;
			return error::success;
		}
#endif
#ifdef I2C2
		if(i2c==I2C2) {
			i2c_vector[1] = callback;
			return error::success;
		}
#endif
#ifdef I2C3
		if(i2c==I2C3) {
			i2c_vector[2] = callback;
			return error::success;
		}
#endif
#ifdef I2C4
		if(i2c==I2C4) {
			i2c_vector[3] = callback;
			return error::success;
		}
#endif
		return error::inval;
	}


//Interrupts handlers
extern "C" {

#ifdef I2C1
	__attribute__((interrupt)) void i2c1_ev_isr_vector() {
		i2c_vector[0](htype::ev);
	}
	__attribute__((interrupt)) void i2c1_er_isr_vector() {
		i2c_vector[0](htype::err);
	}
#endif
#ifdef I2C2
	__attribute__((interrupt)) void i2c2_ev_isr_vector() {
		i2c_vector[1](htype::ev);
	}
	__attribute__((interrupt)) void i2c2_er_isr_vector() {
		i2c_vector[1](htype::err);
	}
#endif
#ifdef I2C3
	__attribute__((interrupt)) void i2c3_ev_isr_vector() {
		i2c_vector[2](htype::ev);
	}
	__attribute__((interrupt)) void i2c3_er_isr_vector() {
		i2c_vector[2](htype::err);
	}
#endif
#ifdef I2C4
	__attribute__((interrupt)) void i2c4_ev_isr_vector() {
		i2c_vector[3](htype::ev);
	}
	__attribute__((interrupt)) void i2c4_er_isr_vector() {
		i2c_vector[3](htype::err);
	}
#endif
}

}
