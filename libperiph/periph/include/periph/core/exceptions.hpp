/*
 * =====================================================================================
 *
 *       Filename:  exceptions.hpp
 *
 *    Description:  basic exceptions
 *
 *        Version:  1.0
 *        Created:  06.08.2018 20:58:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#ifndef __EXCEPTIONS
#include <cstdlib>
#endif
namespace periph::error {

	class exception {
	public:
		exception(int code)
		: m_error(code) {
		}
		auto error() const {
			return m_error;
		}
	private:
		int m_error;
	};

	class generic_exception : public exception {
		using exception::exception;
	};
	class bus_exception : public exception {
		using exception::exception;
	};

#ifdef __EXCEPTIONS
	namespace {
		template<typename EXC>
		inline void expose(int code) {
			if(code) throw EXC(code);
		}
	}
#else
	namespace {
		template<typename EXC>
		inline void expose(int code) {
			if(code) std::abort();
		}
	}
#endif
}

